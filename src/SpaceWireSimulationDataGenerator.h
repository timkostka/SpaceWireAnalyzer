#pragma once

#include <string>

#include <SimulationChannelDescriptor.h>

class SpaceWireAnalyzerSettings;

class SpaceWireSimulationDataGenerator
{
public:
	SpaceWireSimulationDataGenerator();
	~SpaceWireSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, SpaceWireAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

protected:
	SpaceWireAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
    U8 mNextData;
	//void CreateSerialByte();
	//std::string mSerialText;
	//U32 mStringIndex;

	SimulationChannelDescriptorGroup mSpaceWireSimulationChannels; 
	SimulationChannelDescriptor * mData;
    SimulationChannelDescriptor * mStrobe;

};
