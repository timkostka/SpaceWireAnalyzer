#include <vector>

#include "SpaceWireAnalyzer.h"
#include "SpaceWireAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SpaceWireAnalyzer::SpaceWireAnalyzer() : Analyzer2(), mSettings( new SpaceWireAnalyzerSettings() ), mSimulationInitialized( false )
{
    SetAnalyzerSettings( mSettings.get() );
}

SpaceWireAnalyzer::~SpaceWireAnalyzer()
{
    KillThread();
}

void SpaceWireAnalyzer::SetupResults()
{
    mResults.reset( new SpaceWireAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mDataChannel );
}

void SpaceWireAnalyzer::WorkerThread()
{
    printf( "Worker started!\n" );

    mSampleRateHz = GetSampleRate();

    mData = GetAnalyzerChannelData( mSettings->mDataChannel );
    mStrobe = GetAnalyzerChannelData( mSettings->mStrobeChannel );

    // true if sync is established
    // (else we search for good data)
    bool synced = false;

    // total number of bits received
    // U64 bitsReceived = 0;

    // holds up to 12 buffered bits
    struct BufferedBitsStruct
    {
        // number of bits buffered
        U16 count;
        // value of those bits (LSB first)
        U16 value;
        // first sample of the given bit
        U64 firstSample[ 6 ];
        // constructor
        BufferedBitsStruct() : count( 0 ), value( 0 )
        {
        }
        // push a new bit into the buffer
        // (can only be called when count < 12)
        void Push( BitState state, U64 firstSampleOfBit )
        {
            value <<= 1;
            if( state == BIT_HIGH )
            {
                value |= 1;
            }
            count += 1;
            // save first sample of every other bit
            if( count % 2 )
            {
                firstSample[ 5 ] = firstSample[ 4 ];
                firstSample[ 4 ] = firstSample[ 3 ];
                firstSample[ 3 ] = firstSample[ 2 ];
                firstSample[ 2 ] = firstSample[ 1 ];
                firstSample[ 1 ] = firstSample[ 0 ];
                firstSample[ 0 ] = firstSampleOfBit;
            }
        }
        // return the Xth bit in the buffer
        U8 Get( U8 index ) const
        {
            return ( value & ( ( U16 )1 << ( count - index - 1 ) ) ) ? 1 : 0;
        }
        // return true if second parity bit matches
        bool ParityMatch( unsigned int charLength ) const
        {
            // calculate expected parity
            U8 parity = 0;
            for( unsigned int i = 2; i < charLength + 2; ++i )
            {
                parity ^= Get( i );
            }
            return parity == 0;
        }
        // pop an even number of characters
        // (can only be called when count == 6 or count == 12)
        void Pop( U8 skip )
        {
            count -= skip;
            unsigned int delta = skip / 2;
        }
    };

    // hold bits and starting sample of each bit
    BufferedBitsStruct bits;

    // struct CharacterStruct
    //{
    //    // true if this is a control character, else it's a data character
    //    bool is_control;
    //    // value of code (right aligned)
    //    uint8_t value;
    //    U64 firstSample;
    //    U64 lastSample;
    //};

    // number of valid characters required before we output data and assume sync
    // const U32 validCharacterToSync = 4;

    // buffered characters (prior to sync established)
    // std::vector<CharacterStruct> bufferedCharacter;

    // if not synced:
    // 1) if S = D, skip a bit and desync
    // 2) wait for 2 bits to come in
    // 2a) if two transitions ever happen at the exact same time, desync and goto 1
    // 3) if second bit is 1, wait for 6 bits, else wait for 10 bits
    // 4) if second bit is 1, check parity on 5th bit (2-4,6), else check parity on 9th bit (2-8, 10)
    // 5) if parity matches, save frame and skip frame bits, else desync, skip 2 bits
    // 6) in either case, goto 2

    while( true )
    {
        // buffer up to 12 bits
        while( mData->DoMoreTransitionsExistInCurrentData() && mStrobe->DoMoreTransitionsExistInCurrentData() && bits.count < 12 )
        {
            // find next transition on each line
            U64 dataTransition = mData->GetSampleOfNextEdge();
            U64 strobeTransition = mStrobe->GetSampleOfNextEdge();

            // last sample of this frame is just before transition
            U64 nextFirstSample = ( dataTransition < strobeTransition ) ? dataTransition : strobeTransition;

            // move channels past transition
            U64 firstBitSample = mData->GetSampleNumber();
            BitState dataState = mData->GetBitState();
            BitState strobeState = mStrobe->GetBitState();
            mData->AdvanceToAbsPosition( nextFirstSample );
            mStrobe->AdvanceToAbsPosition( nextFirstSample );
            ReportProgress( nextFirstSample - 1 );

            // if they transition on the same sample, de-sync
            if( dataTransition == strobeTransition )
            {
                synced = false;
                bits.count = 0;
                continue;
            }

            // if this is the first bit, skip it if D == S (since parity bit has D != S)
            if( !synced && bits.count == 0 && dataState == strobeState )
            {
                continue;
            }

            // add this bit
            bits.Push( dataState, firstBitSample );
        }

        // need at least 2 bits to interpret frame type
        if( bits.count < 2 )
        {
            break;
            // continue;
        }

        // character length
        unsigned int charLength = bits.Get( 1 ) ? 4 : 10;
        // wait for more bits
        if( bits.count < charLength + 2 )
        {
            break;
            // continue;
        }

        // if parity matches, save a frame
        if( bits.ParityMatch( charLength ) || true )
        {
            synced = true;
            Frame frame;
            frame.mData1 = bits.value >> (bits.count - charLength);
            if (charLength == 4) {
                frame.mData1 &= 0b11;
            }
            else
            {
                frame.mData1 &= 0b11111111;
                // TODO: reverse data bits
            }
            frame.mStartingSampleInclusive = bits.firstSample[ bits.count / 2 - 1 ];
            frame.mEndingSampleInclusive = bits.firstSample[ (bits.count - charLength) / 2 - 1 ] - 1;
            frame.mFlags = 0;
            frame.mType = ( charLength == 4 ) ? 0 : 1;
            mResults->AddFrame( frame );
            mResults->CommitResults();
            ReportProgress( frame.mEndingSampleInclusive );

            bits.Pop( charLength );
        }
        else
        {
            // TODO: if we're synced, mark a failed parity bit
            bits.Pop( 2 );
            synced = false;
        }
    }
    printf( "Done!\n" );
}

bool SpaceWireAnalyzer::NeedsRerun()
{
    return false;
}

U32 SpaceWireAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                                SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitialized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitialized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SpaceWireAnalyzer::GetMinimumSampleRateHz()
{
    return 25000000;
    // mSettings->mBitRate * 4;
}

const char* SpaceWireAnalyzer::GetAnalyzerName() const
{
    return "SpaceWire";
}

const char* GetAnalyzerName()
{
    return "SpaceWire";
}

Analyzer* CreateAnalyzer()
{
    return new SpaceWireAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
