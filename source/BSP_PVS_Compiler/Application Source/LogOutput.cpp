
//-----------------------------------------------------------------------------
// Main Module Includes
//-----------------------------------------------------------------------------
#include "LogOutput.h"
#include <iostream>
#include <conio.h>

//-----------------------------------------------------------------------------
// Namespace Modifiers
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// CLogOutput member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CLogOutput () (Constructor)
// Desc : Creates everything we need to
//-----------------------------------------------------------------------------
CLogOutput::CLogOutput ()
{
    // Reset vars to sensible values
    m_ChannelCount        = 0;
    m_CurrentChannel      = 0;
    m_ChannelText         = NULL;
    m_ChannelTextMarker   = NULL;
    m_ChannelScreenMarker = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CLogOutput () (Destructor)
// Desc : Cleans up everything we need to
//-----------------------------------------------------------------------------
CLogOutput::~CLogOutput ()
{
    ULONG i;

    // Free up text buffers
    if (m_ChannelText)
    {
        // Loop through the channels
        for ( i = 0; i < m_ChannelCount; ++i )
        {
            if ( m_ChannelText[i] ) delete [](m_ChannelText[i]);
        
        } // Next Channel Buffer

        // Delete buffer array
        delete []m_ChannelText;
        m_ChannelText = NULL;

    } // End if any text buffers

    // Free up any resources
    if (m_ChannelTextMarker  ) { delete []m_ChannelTextMarker;   m_ChannelTextMarker = NULL; }
    if (m_ChannelScreenMarker) { delete []m_ChannelScreenMarker; m_ChannelScreenMarker = NULL; }

}

//-----------------------------------------------------------------------------
// Name : Create ()
// Desc : Create the logging window and set up required details.
//-----------------------------------------------------------------------------
bool CLogOutput::Create( unsigned short ChannelCount )
{
    // Validate Requirements
    if (!ChannelCount) return false;

    // Allocate the various per-channel information set
    m_ChannelCount = ChannelCount;
    if (!(m_ChannelText = new LPSTR[ChannelCount])) return false;
    if (!(m_ChannelTextMarker = new unsigned long[ChannelCount])) return false;
    if (!(m_ChannelScreenMarker = new COORD[ChannelCount])) return false;
    
    // Clear arrays
    ZeroMemory( m_ChannelText, ChannelCount * sizeof(LPSTR));
    ZeroMemory( m_ChannelTextMarker, ChannelCount * sizeof(unsigned long));
    ZeroMemory( m_ChannelScreenMarker, ChannelCount * sizeof(COORD));

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : Clear()
// Desc : Clears the text for the channel specified.
//-----------------------------------------------------------------------------
void CLogOutput::Clear( unsigned long Channel )
{
    // Release the text buffer
    if ( m_ChannelText[ Channel ] ) delete []m_ChannelText[ Channel ];
    m_ChannelText[ Channel ] = NULL;

    // If this is the current channel
    if ( Channel == m_CurrentChannel )  ClearConsole( );
}

//-----------------------------------------------------------------------------
// Name : SetRewindMarker()
// Desc : Sets up the rewind marker for the specified channel
//-----------------------------------------------------------------------------
void CLogOutput::SetRewindMarker( unsigned long Channel )
{
    CONSOLE_SCREEN_BUFFER_INFO Info;

    // Store current progress channel
    m_ProgressChannel = Channel;

    // Calculate console position based on currently stored text
    if ( m_ChannelText[ Channel ] )
        m_ChannelTextMarker[ Channel ] = strlen( m_ChannelText[ Channel ] );
    else
        m_ChannelTextMarker[ Channel ] = 0;

    // Retrieve the current screen buffer info
    GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &Info );

    // Store cursor position
    m_ChannelScreenMarker[ Channel ] = Info.dwCursorPosition;
}

//-----------------------------------------------------------------------------
// Name : Rewind()
// Desc : Rewinds the channel so that it now sits at the rewind marker set.
//-----------------------------------------------------------------------------
void CLogOutput::Rewind( unsigned long Channel )
{
    // Reposition terminating character
    if ( m_ChannelText[Channel] ) (m_ChannelText[ Channel ])[ m_ChannelTextMarker[Channel] ] = '\0';
    
    // Clear for current channel?
    if ( Channel == m_CurrentChannel )
    {
        // Clear the console from the current rewind marker
        ClearConsole( m_ChannelScreenMarker[Channel].X, m_ChannelScreenMarker[Channel].Y );

    } // End if current channel
}

//-----------------------------------------------------------------------------
// Name : SetProgressRange()
// Desc : Sets the maximum value used by the progress percentage functions
//-----------------------------------------------------------------------------
void CLogOutput::SetProgressRange( long Maximum )
{
    m_lProgressMax = Maximum;
    m_lOldPercent  = 0;
}

//-----------------------------------------------------------------------------
// Name : SetProgressValue()
// Desc : Sets the current value used by the progress percentage functions
//-----------------------------------------------------------------------------
void CLogOutput::SetProgressValue( long Value )
{
    m_lProgressValue = Value;
}

//-----------------------------------------------------------------------------
// Name : UpdateProgress()
// Desc : Calculates the current progress and prints the information
//-----------------------------------------------------------------------------
void CLogOutput::UpdateProgress( long Amount )
{
    short Percent;

    // Switch to our progress channel just in case.
    SwitchChannel( m_ProgressChannel );

    m_lProgressValue += Amount;
    Percent = (short)((m_lProgressValue * 100) / m_lProgressMax);

    // Is different ?
    if ( Percent > m_lOldPercent )
    {
        // Write percentage info (uses LOGF_ERROR purely to make it red ;)
        Rewind( m_CurrentChannel );
        LogWrite( m_CurrentChannel, LOGF_ERROR, false, _T("%i%%\t\t<==" ), Percent );
        m_lOldPercent = Percent;

    } // End if different
}

//-----------------------------------------------------------------------------
// Name : ProgressSuccess()
// Desc : Denotes that the progress of 'N' was successful
//-----------------------------------------------------------------------------
void CLogOutput::ProgressSuccess( unsigned long Channel )
{
    Rewind( Channel );
    LogWrite( Channel, LOGF_ITALIC, false, _T("Success" ) );   

}

//-----------------------------------------------------------------------------
// Name : ProgressFailure()
// Desc : Denotes that the progress of 'N' was a failure
//-----------------------------------------------------------------------------
void CLogOutput::ProgressFailure( unsigned long Channel )
{
    Rewind( Channel );
    LogWrite( Channel, LOGF_BOLD | LOGF_ERROR, false, _T("Failure!" ) );   

}

//-----------------------------------------------------------------------------
// Name : LogWrite()
// Desc : Adds the specified text (post-formatting) to the log window
// Note : ErrorStatus param allows you to specify the 'severity' of this
//        message as 0 = Normal, 1 = Warning, 2 = Error. The NewMessage param
//        notifies this function that this is not a continuation of an old line
//-----------------------------------------------------------------------------
void CLogOutput::LogWrite( unsigned long Channel, unsigned long Flags, bool NewMessage, LPCTSTR Format, ... )
{
    char InputBuffer[1024], Buffer[1024];
    LPSTR ChannelText;

    // Switch to the new channel if it's not the current
    if ( Channel != m_CurrentChannel) SwitchChannel( Channel );

    // Build the text string
    va_list	ap;
    va_start(ap, Format);
    vsprintf( InputBuffer,Format, ap );
    va_end(ap);

    // Insert bullets and line feeds
    Buffer[0] = '\0';
    if (NewMessage) strcpy( Buffer, _T("\n* ") );
    strcat( Buffer, InputBuffer );

    // Allocate enough room for existing 
    ULONG Size = strlen( Buffer );
    if ( m_ChannelText[ Channel ] ) Size += strlen( m_ChannelText[Channel] );
    if ( Size == 0 ) return;
    
    // Allocate temporary channel text buffer (+ terminating char)
    ChannelText = new char[ Size + 1 ];
    ChannelText[0] = '\0';

    // Copy any old data in and release it if any
    if ( m_ChannelText[Channel] )
    {
        // Copy string over
        strcpy( ChannelText, m_ChannelText[Channel] );

        // Release old data
        delete []m_ChannelText[ Channel ];
    
    } // End if existing data

    // Append new data
    strcat( ChannelText, Buffer );

    // Store new buffer point
    m_ChannelText[Channel] = ChannelText;

    // Output straight to the console
    cout << Buffer;

}

//-----------------------------------------------------------------------------
// Name : GetCurrentChannel ()
// Desc : Return the currently active channel index.
//-----------------------------------------------------------------------------
ULONG CLogOutput::GetCurrentChannel() const
{
    return m_CurrentChannel;
}

//-----------------------------------------------------------------------------
// Name : GetChannelText ()
// Desc : Retrieve the text buffer for the specified channel (if any)
//-----------------------------------------------------------------------------
LPSTR CLogOutput::GetChannelText( unsigned long ChannelIndex ) const
{
    // Validate Requirements
    if ( ChannelIndex >= m_ChannelCount ) return NULL;
    return m_ChannelText[ ChannelIndex ];

}

//-----------------------------------------------------------------------------
// Name : SwitchChannel ()
// Desc : Switches the currently active channel to the one specified
//-----------------------------------------------------------------------------
void CLogOutput::SwitchChannel( unsigned long ChannelIndex, bool Clear /* = true */ )
{
    // Validate Requirements
    if ( ChannelIndex >= m_ChannelCount ) return;
    if ( Clear && ChannelIndex == m_CurrentChannel ) return;

    // Clear the console in its entirety
    if ( Clear ) ClearConsole();

    // Restore the old contents from the new channel
    if ( m_ChannelText[ ChannelIndex ] ) cout << m_ChannelText[ ChannelIndex ];

    // Store new current channel
    m_CurrentChannel = ChannelIndex;
}

//-----------------------------------------------------------------------------
// Name : ClearConsole ()
// Desc : Clears the console window starting from the origin specified
//-----------------------------------------------------------------------------
void CLogOutput::ClearConsole( USHORT OriginX /* = 0 */, USHORT OriginY /* = 0 */ )
{
    COORD ConsoleOrigin = { (SHORT)OriginX, (SHORT)OriginY };
    CONSOLE_SCREEN_BUFFER_INFO Info;
    ULONG Output;

    // Retrieve console information
    GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &Info );

    // Clear the console
    FillConsoleOutputCharacter( GetStdHandle( STD_OUTPUT_HANDLE ), ' ', Info.dwMaximumWindowSize.X * Info.dwMaximumWindowSize.Y, ConsoleOrigin, &Output );

    // Reset the console cursor position
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), ConsoleOrigin );
}