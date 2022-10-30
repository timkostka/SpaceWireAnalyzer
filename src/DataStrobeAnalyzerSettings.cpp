#include "DataStrobeAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DataStrobeAnalyzerSettings::DataStrobeAnalyzerSettings() : mDataChannel( UNDEFINED_CHANNEL ), mStrobeChannel( UNDEFINED_CHANNEL )
{
    mDataChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mDataChannelInterface->SetTitleAndTooltip( "Data", "Data channel" );
    mDataChannelInterface->SetChannel( mDataChannel );

    mStrobeChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mStrobeChannelInterface->SetTitleAndTooltip( "Strobe", "Strobe channel" );
    mStrobeChannelInterface->SetChannel( mStrobeChannel );

    AddInterface( mDataChannelInterface.get() );
    AddInterface( mStrobeChannelInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mDataChannel, "Data", false );
    AddChannel( mStrobeChannel, "Strobe", false );
}

DataStrobeAnalyzerSettings::~DataStrobeAnalyzerSettings()
{
}

bool DataStrobeAnalyzerSettings::SetSettingsFromInterfaces()
{
    mDataChannel = mDataChannelInterface->GetChannel();
    mStrobeChannel = mStrobeChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mDataChannel, "Data", true );
    AddChannel( mStrobeChannel, "Strobe", true );

    return true;
}

void DataStrobeAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mDataChannelInterface->SetChannel( mDataChannel );
    mStrobeChannelInterface->SetChannel( mStrobeChannel );
}

void DataStrobeAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mDataChannel;
    text_archive >> mStrobeChannel;

    ClearChannels();
    AddChannel( mDataChannel, "Data", true );
    AddChannel( mStrobeChannel, "Strobe", true );

    UpdateInterfacesFromSettings();
}

const char* DataStrobeAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mDataChannel;
    text_archive << mStrobeChannel;

    return SetReturnString( text_archive.GetString() );
}
