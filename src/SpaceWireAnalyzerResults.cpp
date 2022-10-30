#include <iostream>
#include <fstream>

#include <AnalyzerHelpers.h>

#include "SpaceWireAnalyzerResults.h"
#include "SpaceWireAnalyzer.h"
#include "SpaceWireAnalyzerSettings.h"

SpaceWireAnalyzerResults::SpaceWireAnalyzerResults( SpaceWireAnalyzer* analyzer, SpaceWireAnalyzerSettings* settings )
    : AnalyzerResults(), mSettings( settings ), mAnalyzer( analyzer )
{
}

SpaceWireAnalyzerResults::~SpaceWireAnalyzerResults()
{
}

void SpaceWireAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    static const char * controlType[ 4 ] = { "FCT", "EOP", "EEP", "ESC" };

    if( frame.mType == 0 )
    {
        AddResultString(controlType[frame.mData1 & 0b11]);
    }
    else
    {
        char buffer[ 5 ] = "0x00";
        static const char hexDigit[ ] = "0123456789ABCDEF";
        buffer[ 2 ] = hexDigit[ ( frame.mData1 & 0xF0 ) >> 4 ];
        buffer[ 3 ] = hexDigit[frame.mData1 & 0x0F];
        AddResultString( buffer );
    }

    //char number_str[ 128 ];
    //AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
    //AddResultString( number_str );
}

void SpaceWireAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    //std::ofstream file_stream( file, std::ios::out );

    //U64 trigger_sample = mAnalyzer->GetTriggerSample();
    //U32 sample_rate = mAnalyzer->GetSampleRate();

    //file_stream << "Time [s],Value" << std::endl;

    //U64 num_frames = GetNumFrames();
    //for( U32 i = 0; i < num_frames; i++ )
    //{
    //    Frame frame = GetFrame( i );

    //    char time_str[ 128 ];
    //    AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

    //    char number_str[ 128 ];
    //    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

    //    file_stream << time_str << "," << number_str << std::endl;

    //    if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
    //    {
    //        file_stream.close();
    //        return;
    //    }
    //}

    //file_stream.close();
}

void SpaceWireAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{

    ClearTabularText();
    Frame frame = GetFrame( frame_index );

    static const char* controlType[ 4 ] = { "FCT", "EOP", "EEP", "ESC" };

    if( frame.mType == 0 )
    {
        AddTabularText( controlType[ frame.mData1 & 0b11 ] );
    }
    else
    {
        char buffer[ 5 ] = "0x00";
        static const char hexDigit[] = "0123456789ABCDEF";
        buffer[ 2 ] = hexDigit[ ( frame.mData1 & 0xF0 ) >> 4 ];
        buffer[ 3 ] = hexDigit[ frame.mData1 & 0x0F ];
        AddTabularText( buffer );
    }

//#ifdef SUPPORTS_PROTOCOL_SEARCH
//    Frame frame = GetFrame( frame_index );
//    ClearTabularText();
//
//    char number_str[ 128 ];
//    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
//    AddTabularText( number_str );
//#endif
}

void SpaceWireAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    // not supported
}

void SpaceWireAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    // not supported
}
