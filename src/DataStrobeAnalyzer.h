#pragma once

#include <Analyzer.h>

#include "DataStrobeAnalyzerResults.h"
#include "DataStrobeSimulationDataGenerator.h"

class DataStrobeAnalyzerSettings;
class ANALYZER_EXPORT DataStrobeAnalyzer : public Analyzer2
{
public:
	DataStrobeAnalyzer();
	virtual ~DataStrobeAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< DataStrobeAnalyzerSettings > mSettings;
	std::auto_ptr< DataStrobeAnalyzerResults > mResults;
	AnalyzerChannelData* mData;
    AnalyzerChannelData* mStrobe;

	DataStrobeSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;
    U32 mSampleRateHz;

};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
