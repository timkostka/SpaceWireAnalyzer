#pragma once

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DataStrobeAnalyzerSettings : public AnalyzerSettings
{
  public:
    DataStrobeAnalyzerSettings();
    virtual ~DataStrobeAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    Channel mDataChannel;
    Channel mStrobeChannel;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mStrobeChannelInterface;
};
