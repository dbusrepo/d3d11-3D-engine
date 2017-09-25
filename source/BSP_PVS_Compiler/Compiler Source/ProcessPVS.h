//-----------------------------------------------------------------------------
// File: ProcessPVS.h
//
// Desc: This source file houses the potential visibility set compiler, of 
//       which an object can spawn to build visible set for any arbitrary 
//       BSP Tree assuming portal information is available.This compile process 
//       is bi-directionally linked to the BSP Tree in that it both gets and 
//       sets data within the BSP Tree and it's associated classes.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _PROCESSPVS_H_
#define _PROCESSPVS_H_

//-----------------------------------------------------------------------------
// CProcessPVS Specific Includes
//-----------------------------------------------------------------------------
#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"
#include "..\\Support Source\\CPlane.h"
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CBSPTree;
class CBSPPortal;
class CPortalPoints;
class CPVSPortal;
class CCompiler;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define PVS_ARRAY_THRESHOLD     100

#define PS_NOTPROCESSED			0       // Portal has not yet been processed
#define PS_PROCESSING			1       // Portal is currently being processed
#define PS_PROCESSED			2       // Portal has been processed

//-----------------------------------------------------------------------------
// Compilation Pre-Processor Flags
//-----------------------------------------------------------------------------
#define PVS_COMPRESSDATA        1       // 1 = ZRLE Compress, 0 = Don't Compress

//-----------------------------------------------------------------------------
// Typedefs, structures & enumerators
//-----------------------------------------------------------------------------
typedef struct _PVSDATA                 // Structure to hold pvs processing data
{
	CPortalPoints   *SourcePoints;   // Current source portals points
    CPortalPoints   *TargetPoints;   // Current target portals points
	CPlane3             TargetPlane;    // The target's plane
	UCHAR              *VisBits;        // Visible Bits being calculated
} PVSDATA;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPortalPoints (Class)
// Desc : The actual points, used by the PVS portal and also passed through
//        the recursion independantly. This is derived from CPolygon to provide
//        all base polygon support, such as splitting etc.
//-----------------------------------------------------------------------------
class CPortalPoints : public CPolygon
{
public:
    //-------------------------------------------------------------------------
	// Constructors / Destructors for this Class
	//-------------------------------------------------------------------------
             CPortalPoints( );
             CPortalPoints( const CPolygon * pPolygon, bool Duplicate = false );
    virtual ~CPortalPoints( );

    //-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    CPortalPoints *     Clip( const CPlane3& Plane, bool KeepOnPlane );

    //-------------------------------------------------------------------------
	// Public Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual HRESULT     Split( const CPlane3& Plane, CPortalPoints * FrontSplit, CPortalPoints * BackSplit);

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    bool                OwnsVertices;           // Do we own the vertices stored here ?
    CPVSPortal         *OwnerPortal;            // Pointer to this points parent portal ;)

};

//-----------------------------------------------------------------------------
// Name : CPVSPortal (Class)
// Desc : Simple top level portal class used for PVS compilation
//-----------------------------------------------------------------------------
class CPVSPortal
{
public:
    //-------------------------------------------------------------------------
	// Constructors / Destructors for this Class
	//-------------------------------------------------------------------------
             CPVSPortal( );
    virtual ~CPVSPortal( );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    UCHAR             Status;               // The compilation status of this portal
    UCHAR             Side;                 // Which direction does this portal point
    long              Plane;                // The plane on which this portal lies
    long              NeighbourLeaf;        // The leaf into which this portal points
    long              PossibleVisCount;     // The size of the PossibleVis array
    UCHAR            *PossibleVis;          // "Possible" visibility information
    UCHAR            *ActualVis;            // "Actual" visibility information
    bool              OwnsPoints;           // Does this own the points ??
    CPortalPoints    *Points;               // The vertices making up this portal

};

//-----------------------------------------------------------------------------
// Typedefs utilised for shorthand versions of our STL types.
//-----------------------------------------------------------------------------
typedef std::vector<CPVSPortal*> vectorPVSPortal;

//-----------------------------------------------------------------------------
// Name : CProcessPVS (Class)
// Desc : This is the primary potential visibility set compiler.
//-----------------------------------------------------------------------------
class CProcessPVS
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CProcessPVS();
    virtual ~CProcessPVS();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         Process( CBSPTree * pTree );
    void            SetOptions( const PVSOPTIONS& Options ) { m_OptionSet = Options; }
    void            SetLogger ( ILogger * pLogger )         { m_pLogger = pLogger; }
    void            SetParent ( CCompiler * pParent )       { m_pParent = pParent; }

    unsigned long   GetPVSPortalCount( ) const { return (unsigned long)m_vpPVSPortals.size(); }
    CPVSPortal     *GetPVSPortal( unsigned long Index ) const { return (Index < m_vpPVSPortals.size()) ? m_vpPVSPortals[Index] : NULL; }

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         GeneratePVSPortals( );
    HRESULT         InitialPortalVis( );
    HRESULT         CalcPortalVis( );
    void            PortalFlood( CPVSPortal * SourcePortal, unsigned char PortalVis[], unsigned long Leaf );
    HRESULT         ExportPVS( CBSPTree * pTree );
    void            GetPortalPlane( const CPVSPortal * pPortal, CPlane3& Plane );
    ULONG           CompressLeafSet ( UCHAR MasterPVS[], const UCHAR VisArray[], ULONG WritePos);
    ULONG           GetNextPortal();
    HRESULT         RecursePVS( ULONG Leaf, CPVSPortal * SourcePortal, PVSDATA & PrevData );
    CPortalPoints * ClipToAntiPenumbra( CPortalPoints * Source, CPortalPoints * Target, CPortalPoints * Generator, bool ReverseClip );

    //-------------------------------------------------------------------------
    // Private Static Functions for This Class.
    //-------------------------------------------------------------------------
    static CPortalPoints *  AllocPortalPoints( const CPolygon * pPolygon, bool Duplicate );
    static bool             GetPVSBit( UCHAR VisArray[], ULONG DestLeaf );
    static void             SetPVSBit( UCHAR VisArray[], ULONG DestLeaf, bool Value = true );
    static void             FreePortalPoints( CPortalPoints * pPoints );

    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    PVSOPTIONS      m_OptionSet;        // The option set for PVS Compilation.
    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    CCompiler      *m_pParent;          // Parent Compiler Pointer
    
    CBSPTree       *m_pTree;            // The tree used to compile the PVS.
    ULONG           m_PVSBytesPerSet;   // Number of Bytes required to describe a single leaf's visibility
    vectorPVSPortal m_vpPVSPortals;     // Vector storage of pointers to CPVSPortal objects

};

#endif // _PROCESSPVS_H_