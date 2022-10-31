#pragma once

#include <Analyzer.h>

#include "SpaceWireAnalyzerResults.h"
#include "SpaceWireSimulationDataGenerator.h"

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
    BufferedBitsStruct();
    // push a new bit into the buffer
    // (can only be called when count < 12)
    void Push( BitState state, U64 firstSampleOfBit );
    // return the Xth bit in the buffer
    U8 Get( U8 index ) const;
    // return true if second parity bit matches
    bool ParityMatch( unsigned int charLength ) const;
    // pop an even number of characters
    // (can only be called when count == 6 or count == 12)
    void Pop( U8 skip );
};

class SpaceWireAnalyzerSettings;

class ANALYZER_EXPORT SpaceWireAnalyzer : public Analyzer2
{
public:
	SpaceWireAnalyzer();
	virtual ~SpaceWireAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

	// control character enums
    enum ControlCharacterEnum : uint8_t
    {
        kControlFct = 0b00,
        kControlEop = 0b01,
        kControlEep = 0b10,
        kControlEsc = 0b11,
    };

    // frame types (in mType parameter)
    enum FrameTypeEnum : uint8_t
    {
        kTypeControlCharacter,
        kTypeDataCharacter,
        kTypeNull,
        kTypeTimecode,
        kTypePacket,
        kTypeEmptyPacket,
        kTypeErrorPacket,
        kTypeEscapeError,
        kTypeParityError,
        kTypeLinkSpeedChange,
    };

    // frame flags
    enum FrameFlagEnum : uint8_t
    {
        kFlagNone = 0,
        kFlagWarning = 1 << 6,
        kFlagError = 1 << 7,
    };

  protected: // vars
	std::auto_ptr< SpaceWireAnalyzerSettings > mSettings;
	std::auto_ptr< SpaceWireAnalyzerResults > mResults;
	AnalyzerChannelData* mData;
    AnalyzerChannelData* mStrobe;

	SpaceWireSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;
    U32 mSampleRateHz;

	// desync the stream
    void Desync();

	// add a new frame
    void AddFrame( U64 mData1, U64 mData2, U8 mType, U8 mFlags, U64 mStartingSampleInclusive, U64 mEndingSampleInclusive );

	// true if stream is synchronized
    bool mSynchronized;
	// average bitrate (mbps) of last character
    double mLastCharacterBitrateMbps;

    // saved bits
    BufferedBitsStruct mBits;

	// current packet data buffer
    std::vector<uint8_t> mPacketData;
	// first sample of first bit of data buffer
    U64 mPacketDataStartingSample;

	// true if ESC code was immediately previous
    bool mEscPrefix;
	// first sample of previous ESC code
    U64 mEscPrefixStartingSample;

	// last timecode received, or 255 if none
    U8 mLastTimecode;
	// first bit of last timecode received
    U64 mLastTimecodeStartingSample;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
