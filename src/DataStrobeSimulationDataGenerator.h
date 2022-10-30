#pragma once

#include <string>

#include <SimulationChannelDescriptor.h>

class DataStrobeAnalyzerSettings;

class DataStrobeSimulationDataGenerator
{
public:
	DataStrobeSimulationDataGenerator();
	~DataStrobeSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, DataStrobeAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

protected:
	DataStrobeAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
    U8 mNextData;
	//void CreateSerialByte();
	//std::string mSerialText;
	//U32 mStringIndex;

	SimulationChannelDescriptorGroup mDataStrobeSimulationChannels; 
	SimulationChannelDescriptor * mData;
    SimulationChannelDescriptor * mStrobe;

};
