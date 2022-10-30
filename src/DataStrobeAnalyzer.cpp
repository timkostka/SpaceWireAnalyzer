#include <vector>

#include "DataStrobeAnalyzer.h"
#include "DataStrobeAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

DataStrobeAnalyzer::DataStrobeAnalyzer() : Analyzer2(), mSettings( new DataStrobeAnalyzerSettings() ), mSimulationInitialized( false )
{
    SetAnalyzerSettings( mSettings.get() );
}

DataStrobeAnalyzer::~DataStrobeAnalyzer()
{
    KillThread();
}

void DataStrobeAnalyzer::SetupResults()
{
    mResults.reset( new DataStrobeAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mDataChannel );
}

void DataStrobeAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();

    mData = GetAnalyzerChannelData( mSettings->mDataChannel );
    mStrobe = GetAnalyzerChannelData( mSettings->mStrobeChannel );

    // data from previous packet (either 4 or 8 bits), or 0 if none received
    U8 lastData = 0;
    // parity from last packet, or 0 if none received
    U8 lastDataParity = 0;

    // true if sync is established
    // (else we search for good data)
    bool synced = false;

    // total number of bits received
    U64 bitsReceived = 0;

    // value of received bit, with MSB received first
    // (new bits are shifted into LSB)
    U64 lastBits = 0;

    // holds up to 64 buffered bits
    struct BufferedBitsStruct
    {
        // number of bits buffered
        U64 count;
        // value of those bits (LSB first)
        U64 value;
        // first sample of the given bit
        // TODO: implement this
        U64 firstSample[ 64 ];
        // return the Xth bit in the buffer
        U8 Get( U8 index ) const
        {
            return ( value & ( ( U64 )1 << ( count - index - 1 ) ) ) ? 1 : 0;
        }
        // pop some characters
        void Pop( U8 skip )
        {
            count -= skip;
        }
    };

    BufferedBitsStruct bits;

    struct CharacterStruct
    {
        // true if this is a control character, else it's a data character
        bool is_control;
        // value of code (right aligned)
        uint8_t value;
        U64 firstSample;
        U64 lastSample;
    };

    // number of valid characters required before we output data and assume sync
    const U32 validCharacterToSync = 4;

    // buffered characters (prior to sync established)
    std::vector<CharacterStruct> bufferedCharacter;

    // if not synced:
    // 1) if D == S, advance one bit to get to potential parity bit
    // 2) wait for 6 bits to come in
    // 3) assume this is a control character
    // 4) wait for 10 bits to be buffered
    // 5) if next parity doesn't match, assume it's a data character
    // 6) if it doesn't match, skip 2 bits and goto 2
    // 7) if it does match, process code and goto 2

    // TODO: at startup, skip a bit if D = S

    U64 firstSample = 0;

    while( true )
    {
        // buffer as many bits as possible, up to 64
        while( mData->DoMoreTransitionsExistInCurrentData() && mStrobe->DoMoreTransitionsExistInCurrentData() && bits.count < 64 )
        {
            // find next transition on each line
            U64 dataTransition = mData->GetSampleOfNextEdge();
            U64 strobeTransition = mStrobe->GetSampleOfNextEdge();

            // last sample of this frame is just before transition
            U64 nextFirstSample = ( dataTransition < strobeTransition ) ? dataTransition - 1 : strobeTransition - 1;
            U64 lastSample = nextFirstSample - 1;

            // move channels past transition
            mData->AdvanceToAbsPosition( nextFirstSample );
            mStrobe->AdvanceToAbsPosition( nextFirstSample );

            // if they transition on the same sample, de-sync
            if( dataTransition == strobeTransition )
            {
                synced = false;
                bits.count = 0;
                firstSample = nextFirstSample;
                lastDataParity = 0;
                continue;
            }

            // if this is the first bit, skip it if D == S (since parity bit has D != S)
            if( !synced && bits.count == 0 )
            {
                continue;
            }

            // add this bit
            ++bits.count;
            bits.value <<= 1;
            if( mData->GetBitState() == BIT_HIGH )
            {
                bits.value |= 1;
            }
        }

        // need at least 6 bits for a control character check
        if( bits.count < 6 || !bits.Get( 1 ) && bits.count < 10 )
        {
            continue;
        }
        bits.Pop( 2 );
        // check for parity mismatch
        if( bits.Get( 0 ) == lastDataParity ^ bits.Get( 1 ) )
        {
            bits.Pop( 2 );
            synced = false;
            lastDataParity = 0;
            continue;
        }
        if( bits.Get( 1 ) )
        {
            // control character
            Frame frame;
            frame.mData1 = ( mData->GetBitState() == BIT_HIGH ) ? 1 : 0;
            frame.mStartingSampleInclusive = firstSample;
            frame.mEndingSampleInclusive = lastSample;
            frame.mFlags = 0;
            frame.mType = 0;
        }
        else
        {
            // data character
        }

        //// if first sample is different than last sample, create a frame
        //      if( firstSample != lastSample )
        //      {
        //          Frame frame;
        //          frame.mData1 = ( mData->GetBitState() == BIT_HIGH ) ? 1 : 0;
        //          frame.mStartingSampleInclusive = firstSample;
        //          frame.mEndingSampleInclusive = lastSample;
        //          frame.mFlags = 0;
        //          frame.mType = 0;
        //      }
        //      firstSample = lastSample + 1;
        //      mData->AdvanceToAbsPosition( firstSample );
        //      mStrobe->AdvanceToAbsPosition( firstSample );

        // mResults->CommitResults();
        //      ReportProgress( firstSample);
    }
}

bool DataStrobeAnalyzer::NeedsRerun()
{
    return false;
}

U32 DataStrobeAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                                SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitialized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitialized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DataStrobeAnalyzer::GetMinimumSampleRateHz()
{
    return 25000000;
    // mSettings->mBitRate * 4;
}

const char* DataStrobeAnalyzer::GetAnalyzerName() const
{
    return "Data/Strobe";
}

const char* GetAnalyzerName()
{
    return "Data/Strobe";
}

Analyzer* CreateAnalyzer()
{
    return new DataStrobeAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
