
#ifndef _PROCESSTJR_H_
#define _PROCESSTJR_H_

//-----------------------------------------------------------------------------
// CProcessTJR Specific Includes
//-----------------------------------------------------------------------------
#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CBSPTree;
class CCompiler;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define TJR_ARRAY_THRESHOLD     100

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CProcessTJR (Class)
// Desc : This is the primary T-Junction repair process class.
//-----------------------------------------------------------------------------
class CProcessTJR
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CProcessTJR();
    virtual ~CProcessTJR();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    void            SetOptions( const TJROPTIONS& Options ) { m_OptionSet = Options; }
    void            SetLogger ( ILogger * pLogger )         { m_pLogger = pLogger; }
    void            SetParent ( CCompiler * pParent )       { m_pParent = pParent; }

    HRESULT         Process( CPolygon ** ppPolys, ULONG PolyCount );

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    bool            RepairTJunctions( CPolygon *pPoly1, CPolygon *pPoly2 ) const;

    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    TJROPTIONS      m_OptionSet;        // The option set for TJR Processing
    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    CCompiler      *m_pParent;          // Parent Compiler Pointer

};

#endif // _PROCESSTJR_H_