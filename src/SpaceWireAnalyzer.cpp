#include <vector>

#include "SpaceWireAnalyzer.h"
#include "SpaceWireAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

/*

The Frame object mType parameter is as follows:
0: bare control character
1: bare data character
2: NULL
3: timecode
4: packet (mData1 = packet length (1 byte) + 63 bytes of data, mData2 = next 64 bytes)
5: error packet
6: escape error
7: parity error
8: link speed change (value is new rate, duration is first bit of new character)

The Frame object mFlag parameter is as follows:

#define DISPLAY_AS_ERROR_FLAG ( 1 << 7 )
#define DISPLAY_AS_WARNING_FLAG ( 1 << 6 )

*/

// constructor
BufferedBitsStruct::BufferedBitsStruct() : count( 0 ), value( 0 )
{
}

// push a new bit into the buffer
// (can only be called when count < 12)
void BufferedBitsStruct::Push( BitState state, U64 firstSampleOfBit )
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
U8 BufferedBitsStruct::Get( U8 index ) const
{
    return ( value & ( ( U16 )1 << ( count - index - 1 ) ) ) ? 1 : 0;
}

// return true if second parity bit matches
bool BufferedBitsStruct::ParityMatch( unsigned int charLength ) const
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
void BufferedBitsStruct::Pop( U8 skip )
{
    count -= skip;
    unsigned int delta = skip / 2;
}


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

void SpaceWireAnalyzer::Desync()
{
    mSynchronized = false;
    mLastCharacterBitrateMbps = 0.0;
    mPacketData.clear();
    mEscPrefix = false;
    mLastTimecode = 255;
    mBits.count = 0;
}

void SpaceWireAnalyzer::AddFrame( U64 mData1, U64 mData2, U8 mType, U8 mFlags, U64 mStartingSampleInclusive, U64 mEndingSampleInclusive )
{
    Frame frame;
    frame.mData1 = mData1;
    frame.mData2 = mData2;
    frame.mType = mType;
    frame.mFlags = mFlags;
    frame.mStartingSampleInclusive = mStartingSampleInclusive;
    frame.mEndingSampleInclusive = mEndingSampleInclusive;
    mResults->AddFrame( frame );
    mResults->CommitResults();
}

void SpaceWireAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();

    mData = GetAnalyzerChannelData( mSettings->mDataChannel );
    mStrobe = GetAnalyzerChannelData( mSettings->mStrobeChannel );

    // reset state variables to desynchronized state
    Desync();

    // if not synced:
    // 1) if not synced and S = D, skip a bit and desync
    // 2) wait for 2 bits to come in
    // 2a) if two transitions ever happen at the exact same time, desync and goto 1
    // 3) if second bit is 1, wait for 6 bits, else wait for 10 bits
    // 4) if second bit is 1, check parity on 5th bit (2-4,6), else check parity on 9th bit (2-8, 10)
    // 5) if parity matches, save frame and skip frame bits, else desync, skip 2 bits
    // 6) in either case, goto 2

    while( true )
    {
        // buffer up to 12 bits
        while( 
            mData->DoMoreTransitionsExistInCurrentData()
            && mStrobe->DoMoreTransitionsExistInCurrentData()
            && (mBits.count < 6 || mBits.Get(1) == 0 && mBits.count < 12))
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
                Desync();
                continue;
            }

            // if this is the first bit, skip it if D == S (since parity bit has D != S)
            if( !mSynchronized && mBits.count == 0 && dataState == strobeState )
            {
                continue;
            }

            // add this bit
            mBits.Push( dataState, firstBitSample );
        }

        // need at least 2 bits to interpret frame type
        if( mBits.count < 2 )
        {
            break;
        }

        // character length
        unsigned int charLength = mBits.Get( 1 ) ? 4 : 10;
        // wait for more bits
        if( mBits.count < charLength + 2 )
        {
            break;
        }

        // get start/end samples of character
        U64 startingSample = mBits.firstSample[ mBits.count / 2 - 1 ];
        U64 endingSample = mBits.firstSample[ ( mBits.count - charLength ) / 2 - 1 ] - 1;

        // if parity matches, save a frame
        if( mBits.ParityMatch( charLength ) || true )
        {
            // get type of character
            bool controlChar = charLength == 4;
            // get character value
            U8 value = mBits.value >> ( mBits.count - charLength );
            // trim or reverse data bits
            if( controlChar )
            {
                value &= 0b11;
            }
            else
            {
                U8 x = value;
                value = 0;
                for( unsigned int i = 0; i < 8; ++i )
                {
                    value <<= 1;
                    value |= x & 1;
                    x >>= 1;
                }
            }
            // pop bits from buffer
            mBits.Pop( charLength );

            // calculate bitrate
            if( mSettings->mShowLinkSpeedChanges )
            {
                double thisCharBitrate = ( endingSample + 1 - startingSample ) / ( mSampleRateHz * 1e6 );
                if( mLastCharacterBitrateMbps != 0.0 &&
                    ( thisCharBitrate > mLastCharacterBitrateMbps * 1.5 || thisCharBitrate < mLastCharacterBitrateMbps / 1.5 ) )
                {
                    // TODO: report bit rate change
                }
                mLastCharacterBitrateMbps = thisCharBitrate;
            }

            // if we're not combining chars, just send it out
            if( !mSettings->mCombineChars )
            {
                AddFrame( value, 0, ( controlChar ) ? kTypeControlCharacter : kTypeDataCharacter, 0, startingSample, endingSample );
            }
            else
            {
                // handle continuing ESC sequences
                if( mEscPrefix )
                {
                    mEscPrefix = false;
                    if( controlChar && value == kControlFct )
                    {
                        // ESC + FCT = NULL
                        if( mSettings->mShowNulls )
                        {
                            AddFrame( value, 0, kTypeNull, 0, mEscPrefixStartingSample, endingSample );
                        }
                    }
                    else if( !controlChar )
                    {
                        // ESC + DATA = TIMECODE
                        // see if this matches the expected value
                        U8 expectedValue = ( mLastTimecode + 1 ) % 64;
                        bool matchesExpected = (mLastTimecode == 255 || value == expectedValue);
                        // save results
                        if( mSettings->mShowTimecodes || !matchesExpected && mSettings->mShowErrors )
                        {
                            // TIMECODE
                            if( mSettings->mShowTimecodes )
                            {
                                U64 delta = 0;
                                if( mLastTimecode != 255 )
                                {
                                    delta = mEscPrefixStartingSample - mLastTimecodeStartingSample;
                                }
                                AddFrame( value, delta, kTypeTimecode, (matchesExpected) ? 0 : kFlagWarning, mEscPrefixStartingSample, endingSample );
                            }
                        }
                        // save timecode for later comparison
                        mLastTimecode = value;
                        mLastTimecodeStartingSample = mEscPrefixStartingSample;
                    }
                    else
                    {
                        // anything else is invalid
                        if( mSettings->mShowErrors )
                        {
                            AddFrame( value, 0, kTypeEscapeError, kFlagError, mEscPrefixStartingSample, endingSample );
                        }
                    }
                }
                else
                {
                    if( controlChar )
                    {
                        if( value == kControlEsc )
                        {
                            mEscPrefix = true;
                            mEscPrefixStartingSample = startingSample;
                        }
                        else if( value == kControlEop )
                        {
                            // end of frame
                            if( mPacketData.empty() )
                            {
                                // empty packet error
                                if( mSettings->mShowErrors )
                                {
                                    AddFrame( 0, 0, kTypeEmptyPacket, 0, startingSample, endingSample );
                                }
                            }
                            else
                            {
                                // packet
                                if( mSettings->mShowRegularPackets )
                                {
                                    U64 length = mPacketData.size();
                                    U64 data = 0;
                                    for( unsigned i = 0; i < 8 && i < length; ++i )
                                    {
                                        data <<= 8;
                                        data |= mPacketData[ i ];
                                    }
                                    AddFrame( data, length, kTypePacket, 0, mPacketDataStartingSample, endingSample );
                                }
                            }
                            mPacketData.clear();
                        }
                        else if( value == kControlEep )
                        {
                            // error packet
                            if( mPacketData.empty() )
                            {
                                mPacketDataStartingSample = startingSample;
                            }
                            if( mSettings->mShowErrorPackets )
                            {
                                U64 length = mPacketData.size();
                                U64 data = 0;
                                for( unsigned i = 0; i < 8 && i < length; ++i )
                                {
                                    data <<= 8;
                                    data |= mPacketData[ i ];
                                }
                                AddFrame( data, length, kTypeErrorPacket, 0, mPacketDataStartingSample, endingSample );
                            }
                            mPacketData.clear();
                        }
                        else if( value == kControlFct )
                        {
                            // FCT (credit)
                            if( mSettings->mShowFcts )
                            {
                                AddFrame( value, 0, kTypeControlCharacter, 0, startingSample, endingSample );
                            }
                        }
                    }
                    else
                    {
                        // save data
                        if( mPacketData.empty() )
                        {
                            mPacketDataStartingSample = startingSample;
                        }
                        mPacketData.push_back( value );
                    }
                }
            }
        }
        else
        {
            // report parity error
            if( mSynchronized && mSettings->mShowErrors )
            {
                AddFrame( 0, 0, kTypeParityError, kFlagError, startingSample, endingSample );
            }
            if( mSettings->mDesyncAfterError )
            {
                mSynchronized = false;
            }
            if( mSynchronized )
            {
                mBits.Pop( charLength );
            }
            else
            {
                mBits.Pop( 2 );
            }
        }
        ReportProgress( endingSample );
    }
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
