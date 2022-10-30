#include "SpaceWireSimulationDataGenerator.h"
#include "SpaceWireAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

SpaceWireSimulationDataGenerator::SpaceWireSimulationDataGenerator() : mNextData( 0 )
{
}

SpaceWireSimulationDataGenerator::~SpaceWireSimulationDataGenerator()
{
}

void SpaceWireSimulationDataGenerator::Initialize( U32 simulation_sample_rate, SpaceWireAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mData = mSpaceWireSimulationChannels.Add( settings->mDataChannel, mSimulationSampleRateHz, BIT_LOW );
    mStrobe = mSpaceWireSimulationChannels.Add( settings->mDataChannel, mSimulationSampleRateHz, BIT_LOW );

    mSpaceWireSimulationChannels.AdvanceAll( mSimulationSampleRateHz / 10000000 );

}

U32 SpaceWireSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	// 10 Mbit
    U32 samples_per_bit = (mSimulationSampleRateHz + 5000000) / 10000000;

	while( mData->GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
		// create this data bit, MSB first
        uint8_t x = mNextData++;
        for( int i = 0; i < 8; ++i )
        {
            BitState target_data_state = ( x & 0x80 ) ? BIT_HIGH : BIT_LOW;
            x <<= 1;

            if( mData->GetCurrentBitState() == target_data_state)
            {
                mData->Transition();
            }
            else
            {
                mStrobe->Transition();
            }
            mSpaceWireSimulationChannels.AdvanceAll( samples_per_bit );
        }
	}

	*simulation_channels = mSpaceWireSimulationChannels.GetArray();
    return mSpaceWireSimulationChannels.GetCount();
}

//void SpaceWireSimulationDataGenerator::CreateSerialByte()
//{
//	U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;
//
//	U8 byte = mSerialText[ mStringIndex ];
//	mStringIndex++;
//	if( mStringIndex == mSerialText.size() )
//		mStringIndex = 0;
//
//	//we're currenty high
//	//let's move forward a little
//	mSerialSimulationData.Advance( samples_per_bit * 10 );
//
//	mSerialSimulationData.Transition();  //low-going edge for start bit
//	mSerialSimulationData.Advance( samples_per_bit );  //add start bit time
//
//	U8 mask = 0x1 << 7;
//	for( U32 i=0; i<8; i++ )
//	{
//		if( ( byte & mask ) != 0 )
//			mSerialSimulationData.TransitionIfNeeded( BIT_HIGH );
//		else
//			mSerialSimulationData.TransitionIfNeeded( BIT_LOW );
//
//		mSerialSimulationData.Advance( samples_per_bit );
//		mask = mask >> 1;
//	}
//
//	mSerialSimulationData.TransitionIfNeeded( BIT_HIGH ); //we need to end high
//
//	//lets pad the end a bit for the stop bit:
//	mSerialSimulationData.Advance( samples_per_bit );
//}
