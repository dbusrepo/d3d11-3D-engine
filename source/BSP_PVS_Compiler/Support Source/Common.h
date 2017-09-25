
#ifndef _COMMON_H_
#define _COMMON_H_

//-----------------------------------------------------------------------------
// Miscellaneous definitions
//-----------------------------------------------------------------------------
#define CURRENT_FILE		__FILE__
#define CURRENT_LINE		__LINE__
#define VC_EXTRALEAN

//-----------------------------------------------------------------------------
// App Specific Includes
//-----------------------------------------------------------------------------
#include <tchar.h>
#include <wtypes.h> // Warning may include windows.h.. Beware when using in MFC
#include <math.h>
#include "AppError.h"

//-----------------------------------------------------------------------------
// Useful Math Defines, Macros and Constants
//-----------------------------------------------------------------------------
#define EPSILON         0.01f
#define DEGTORAD(x)     (x * 0.01745329251994329547f)
#define RADTODEG(x)     (x * 57.2957795130823228646f)
#define PI              3.14159265358979323846f

//--------------------------------------------------------------
// Log Channel Definitions
//--------------------------------------------------------------
#define LOG_GENERAL     0       // General Log Channel
#define LOG_HSR         1       // Hidden Surface Removal Log Channel
#define LOG_BSP         3       // Binary Space Partition Compiler Log Channel
#define LOG_PRT         4       // Portal Compiler Log Channel
#define LOG_PVS         5       // Potential Visibility Set Compiler Log Channel
#define LOG_TJR         6       // T-Junction Repair Log Channel
#define LOG_LMP         7       // Light Mapping Channel

#define LOGF_WARNING     1      // Log Flags : Display with warning format
#define LOGF_ERROR       2      // Log Flags : Display with error format
#define LOGF_UNDERLINE   4      // Log Flags : Display with underline property
#define LOGF_BOLD        8      // Log Flags : Display with bold property
#define LOGF_ITALIC      16     // Log Flags : Display with italic property

//--------------------------------------------------------------
// Process Number Definitions
//--------------------------------------------------------------
#define PROCESS_HSR         0   // Hidden Surface Removal
#define PROCESS_BSP         2   // Binary Space Partition
#define PROCESS_PRT         3   // Portals
#define PROCESS_PVS         4   // Potential Visibility Set
#define PROCESS_TJR         5   // T-Junction Repair

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : ILogger (Interface Class)
// Desc : This is essentially an abstract base interface class to provide
//        the compiler and application with logging support irrespective of
//        one another. An abstract class is provided so that providing a
//        different output implementation should be seamless.
//-----------------------------------------------------------------------------
class ILogger
{
public:
    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    virtual void    LogWrite( unsigned long Channel, unsigned long Flags, bool NewMessage, LPCTSTR Format, ... ) = 0;
    virtual void    SetRewindMarker( unsigned long Channel ) = 0;
    virtual void    Rewind( unsigned long Channel ) = 0;
    virtual void    Clear( unsigned long Channel ) = 0;
    virtual ULONG   GetCurrentChannel( ) const = 0;
    virtual void    SetProgressRange( long Maximum ) = 0;
    virtual void    SetProgressValue( long Value ) = 0;
    virtual void    UpdateProgress( long Amount = 1 ) = 0;
    virtual void    ProgressSuccess( unsigned long Channel ) {};
    virtual void    ProgressFailure( unsigned long Channel ) {};
};

enum CLASSIFYTYPE           // Plane / Poly classification
{
	CLASSIFY_ONPLANE = 0,  // On the plane
	CLASSIFY_BEHIND = 1,  // Is behind
	CLASSIFY_INFRONT = 2,  // Is in front
	CLASSIFY_SPANNING = 3   // Spans the plane
};

#endif //_COMMON_H_