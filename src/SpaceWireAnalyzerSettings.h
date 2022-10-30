#pragma once

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class SpaceWireAnalyzerSettings : public AnalyzerSettings
{
  public:
    SpaceWireAnalyzerSettings();
    virtual ~SpaceWireAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    Channel mDataChannel;
    Channel mStrobeChannel;

    bool mCombineChars;
    bool mShowNulls;
    bool mShowFcts;
    bool mShowTimecodes;
    bool mShowRegularPackets;
    bool mShowErrorPackets;
    bool mShowErrors;
    bool mShowLinkSpeedChanges;
    bool mDesyncAfterError;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mStrobeChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mCombineCharsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowNullsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowFctsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowTimecodesInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowRegularPacketsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowErrorPacketsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowErrorsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mShowLinkSpeedChangesInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mDesyncAfterErrorInterface;
};
