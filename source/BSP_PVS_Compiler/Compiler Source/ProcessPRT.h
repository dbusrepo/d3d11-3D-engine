//-----------------------------------------------------------------------------
// File: ProcessPRT.h
//
// Desc: This source file houses the primary portal compiler, of which an
//       object can spawn to build a set of portals for any arbitrary BSP Tree.
//       This compile process is bi-directionally linked to the BSP Tree in 
//       that it both gets and sets data within the BSP Tree and it's
//       associated classes.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _PROCESSPRT_H_
#define _PROCESSPRT_H_

//-----------------------------------------------------------------------------
// CProcessPRT Specific Includes
//-----------------------------------------------------------------------------
#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CBSPTree;
class CBSPPortal;
class CCompiler;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define PRT_ARRAY_THRESHOLD     100

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CProcessPRT (Class)
// Desc : This is the primary portal compilation process class which takes the
//        information stored in a BSP Tree and compiles a set of portals for
//        that geometry.
//-----------------------------------------------------------------------------
class CProcessPRT
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CProcessPRT();
    virtual ~CProcessPRT();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         Process( CBSPTree * pTree );
    void            SetOptions( const PRTOPTIONS& Options ) { m_OptionSet = Options; }
    void            SetLogger ( ILogger * pLogger )         { m_pLogger = pLogger; }
    void            SetParent ( CCompiler * pParent )       { m_pParent = pParent; }

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    CBSPPortal     *ClipPortal          ( unsigned long Node, CBSPPortal * pPortal );
    bool            FindLeaf            ( unsigned long Leaf, unsigned long Node );
    unsigned long   ClassifyLeaf        ( unsigned long Leaf, unsigned long Node );
    HRESULT         AddPortals          ( CBSPPortal * PortalList );

    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    PRTOPTIONS      m_OptionSet;        // The option set for portal Compilation.
    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    CCompiler      *m_pParent;          // Parent Compiler Pointer
    CBSPTree       *m_pTree;            // The tree used to compile the portal set.

};

#endif // _PROCESSPRT_H_