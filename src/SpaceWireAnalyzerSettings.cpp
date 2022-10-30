#include "SpaceWireAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


SpaceWireAnalyzerSettings::SpaceWireAnalyzerSettings()
    : mDataChannel( UNDEFINED_CHANNEL ),
      mStrobeChannel( UNDEFINED_CHANNEL ),
      mCombineChars( true ),
      mShowNulls( false ),
      mShowFcts( false ),
      mShowTimecodes( true ),
      mShowRegularPackets( true ),
      mShowErrorPackets( true ),
      mShowErrors( true ),
      mShowLinkSpeedChanges( false ),
      mDesyncAfterError( true )
{
    mDataChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mDataChannelInterface->SetTitleAndTooltip( "Data", "Data channel" );
    mDataChannelInterface->SetChannel( mDataChannel );

    mStrobeChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mStrobeChannelInterface->SetTitleAndTooltip( "Strobe", "Strobe channel" );
    mStrobeChannelInterface->SetChannel( mStrobeChannel );

    mCombineCharsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mCombineCharsInterface->SetTitleAndTooltip( "", "Show packets and codes rather than bare characters" );
    mCombineCharsInterface->SetCheckBoxText( "Combine characters" );
    mCombineCharsInterface->SetValue( mCombineChars );

    mShowNullsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowNullsInterface->SetTitleAndTooltip( "", "" );
    mShowNullsInterface->SetCheckBoxText( "Show NULLs" );
    mShowNullsInterface->SetValue( mShowNulls );

    mShowFctsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowFctsInterface->SetTitleAndTooltip( "", "" );
    mShowFctsInterface->SetCheckBoxText( "Show FCTs" );
    mShowFctsInterface->SetValue( mShowFcts );

    mShowTimecodesInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowTimecodesInterface->SetTitleAndTooltip( "", "" );
    mShowTimecodesInterface->SetCheckBoxText( "Show time-codes" );
    mShowTimecodesInterface->SetValue( mShowTimecodes );

    mShowRegularPacketsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowRegularPacketsInterface->SetTitleAndTooltip( "", "" );
    mShowRegularPacketsInterface->SetCheckBoxText( "Show regular packets" );
    mShowRegularPacketsInterface->SetValue( mShowRegularPackets );

    mShowErrorPacketsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowErrorPacketsInterface->SetTitleAndTooltip( "", "" );
    mShowErrorPacketsInterface->SetCheckBoxText( "Show error packets" );
    mShowErrorPacketsInterface->SetValue( mShowErrorPackets );

    mShowErrorsInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowErrorsInterface->SetTitleAndTooltip( "", "" );
    mShowErrorsInterface->SetCheckBoxText( "Show other errors" );
    mShowErrorsInterface->SetValue( mShowErrors );

    mShowLinkSpeedChangesInterface.reset( new AnalyzerSettingInterfaceBool() );
    mShowLinkSpeedChangesInterface->SetTitleAndTooltip( "", "" );
    mShowLinkSpeedChangesInterface->SetCheckBoxText( "Show link speed changes" );
    mShowLinkSpeedChangesInterface->SetValue( mShowLinkSpeedChanges );

    mDesyncAfterErrorInterface.reset( new AnalyzerSettingInterfaceBool() );
    mDesyncAfterErrorInterface->SetTitleAndTooltip( "", "" );
    mDesyncAfterErrorInterface->SetCheckBoxText( "Desync after protocol error" );
    mDesyncAfterErrorInterface->SetValue( mDesyncAfterError );

    AddInterface( mDataChannelInterface.get() );
    AddInterface( mStrobeChannelInterface.get() );
    AddInterface( mCombineCharsInterface.get() );
    AddInterface( mShowNullsInterface.get() );
    AddInterface( mShowFctsInterface.get() );
    AddInterface( mShowTimecodesInterface.get() );
    AddInterface( mShowRegularPacketsInterface.get() );
    AddInterface( mShowErrorPacketsInterface.get() );
    AddInterface( mShowErrorsInterface.get() );
    AddInterface( mShowLinkSpeedChangesInterface.get() );
    AddInterface( mDesyncAfterErrorInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mDataChannel, "Data", false );
    AddChannel( mStrobeChannel, "Strobe", false );
}

SpaceWireAnalyzerSettings::~SpaceWireAnalyzerSettings()
{
}

bool SpaceWireAnalyzerSettings::SetSettingsFromInterfaces()
{
    mDataChannel = mDataChannelInterface->GetChannel();
    mStrobeChannel = mStrobeChannelInterface->GetChannel();
    mCombineChars = mCombineCharsInterface->GetValue();
    mShowNulls = mShowNullsInterface->GetValue();
    mShowFcts = mShowFctsInterface->GetValue();
    mShowTimecodes = mShowTimecodesInterface->GetValue();
    mShowRegularPackets = mShowRegularPacketsInterface->GetValue();
    mShowErrorPackets = mShowErrorPacketsInterface->GetValue();
    mShowErrors = mShowErrorsInterface->GetValue();
    mShowLinkSpeedChanges = mShowLinkSpeedChangesInterface->GetValue();
    mDesyncAfterError = mDesyncAfterErrorInterface->GetValue();

    ClearChannels();
    AddChannel( mDataChannel, "Data", true );
    AddChannel( mStrobeChannel, "Strobe", true );

    return true;
}

void SpaceWireAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mDataChannelInterface->SetChannel( mDataChannel );
    mStrobeChannelInterface->SetChannel( mStrobeChannel );
    mCombineCharsInterface->SetValue( mCombineChars );
    mShowNullsInterface->SetValue( mShowNulls );
    mShowFctsInterface->SetValue( mShowFcts );
    mShowTimecodesInterface->SetValue( mShowTimecodes );
    mShowRegularPacketsInterface->SetValue( mShowRegularPackets );
    mShowErrorPacketsInterface->SetValue( mShowErrorPackets );
    mShowErrorsInterface->SetValue( mShowErrors );
    mShowLinkSpeedChangesInterface->SetValue( mShowLinkSpeedChanges );
    mDesyncAfterErrorInterface->SetValue( mDesyncAfterError );
}

void SpaceWireAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mDataChannel;
    text_archive >> mStrobeChannel;
    text_archive >> mCombineChars;
    text_archive >> mShowNulls;
    text_archive >> mShowFcts;
    text_archive >> mShowTimecodes;
    text_archive >> mShowRegularPackets;
    text_archive >> mShowErrorPackets;
    text_archive >> mShowErrors;
    text_archive >> mShowLinkSpeedChanges;
    text_archive >> mDesyncAfterError;

    ClearChannels();
    AddChannel( mDataChannel, "Data", true );
    AddChannel( mStrobeChannel, "Strobe", true );

    UpdateInterfacesFromSettings();
}

const char* SpaceWireAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mDataChannel;
    text_archive << mStrobeChannel;
    text_archive << mCombineChars;
    text_archive << mShowNulls;
    text_archive << mShowFcts;
    text_archive << mShowTimecodes;
    text_archive << mShowRegularPackets;
    text_archive << mShowErrorPackets;
    text_archive << mShowErrors;
    text_archive << mShowLinkSpeedChanges;
    text_archive << mDesyncAfterError;

    return SetReturnString( text_archive.GetString() );
}
