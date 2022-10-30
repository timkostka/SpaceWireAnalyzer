#pragma once

#include <Analyzer.h>

#include "SpaceWireAnalyzerResults.h"
#include "SpaceWireSimulationDataGenerator.h"

#define CONTROL_FLAG ( 1 << 0 )
#define DATA_FLAG ( 1 << 1 )

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

protected: //vars
	std::auto_ptr< SpaceWireAnalyzerSettings > mSettings;
	std::auto_ptr< SpaceWireAnalyzerResults > mResults;
	AnalyzerChannelData* mData;
    AnalyzerChannelData* mStrobe;

	SpaceWireSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;
    U32 mSampleRateHz;

};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
