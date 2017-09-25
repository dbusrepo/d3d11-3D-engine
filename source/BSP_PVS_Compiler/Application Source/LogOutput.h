
#ifndef _CLOGOUTPUT_H_
#define _CLOGOUTPUT_H_

//-----------------------------------------------------------------------------
// CLogOutput Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CLogOutput (Class)
// Desc : The main logging window that displays compiler output.
//-----------------------------------------------------------------------------
class CLogOutput : public ILogger
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             CLogOutput();
    virtual ~CLogOutput();

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    bool                Create              ( unsigned short ChannelCount );
    void                SwitchChannel       ( unsigned long ChannelIndex, bool Clear = true );
    void                ClearConsole        ( USHORT OriginX = 0, USHORT OriginY = 0 );
    LPSTR               GetChannelText      ( unsigned long ChannelIndex ) const;

    //-------------------------------------------------------------------------
    // Public Functions for This Class (ILogger)
    //-------------------------------------------------------------------------
    virtual void        LogWrite            ( unsigned long Channel, unsigned long Flags, bool NewMessage, LPCTSTR Format, ... );
    virtual void        SetRewindMarker     ( unsigned long Channel );
    virtual void        Rewind              ( unsigned long Channel );
    virtual void        Clear               ( unsigned long Channel );
    virtual ULONG       GetCurrentChannel   ( ) const;
    virtual void        SetProgressRange    ( long Maximum );
    virtual void        SetProgressValue    ( long Value );
    virtual void        UpdateProgress      ( long Amount = 1 );
    virtual void        ProgressSuccess     ( unsigned long Channel );
    virtual void        ProgressFailure     ( unsigned long Channel );

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    LPSTR              *m_ChannelText;          // Storage for the text in the number of different channels
    unsigned long      *m_ChannelTextMarker;    // Storage for the current rewind markers for the text buffer
    COORD              *m_ChannelScreenMarker;  // Storage for the current rewind markers for the screen
    unsigned long       m_ChannelCount;         // Number of channels currently available for writing
    unsigned long       m_CurrentChannel;       // Currently selected channel
    unsigned long       m_ProgressChannel;      // The channel current progress is being tracked for
    long                m_lProgressMax;         // Maximum progress value
    long                m_lProgressValue;       // Current progress value
    long                m_lOldPercent;          // Last percent value recorded
};

#endif // _CLOGOUTPUT_H_