//-----------------------------------------------------------------------------
// File: ProcessPVS.cpp
//
// Desc: This source file houses the potential visibility set compiler, of 
//       which an object can spawn to build visible set for any arbitrary 
//       BSP Tree assuming portal information is available.This compile process 
//       is bi-directionally linked to the BSP Tree in that it both gets and 
//       sets data within the BSP Tree and it's associated classes.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CProcessPVS Specific Includes
//-----------------------------------------------------------------------------
#include "ProcessPVS.h"
#include "CCompiler.h"
#include "CBSPTree.h"
#include "..\\Support Source\\CPlane.h"

//-----------------------------------------------------------------------------
// Desc : CPVSPortal member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPVSPortal () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPVSPortal::CPVSPortal()
{
	// Initialise any class specific items
    Status              = PS_NOTPROCESSED;
    Plane               = -1;
    NeighbourLeaf       = -1;
    PossibleVisCount    = 0;
    PossibleVis         = NULL;
    ActualVis           = NULL;
    Points              = NULL;
    OwnsPoints          = false;
}

//-----------------------------------------------------------------------------
// Name : ~CPortalPoints () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
CPVSPortal::~CPVSPortal()
{
    // Clean up after ourselves
    if (ActualVis && PossibleVis != ActualVis) delete []ActualVis;
    if (PossibleVis) delete []PossibleVis;
    if (Points && OwnsPoints ) delete Points;

    // Empty pointers
    PossibleVis = NULL;
    ActualVis   = NULL;
    Points      = NULL;
}

//-----------------------------------------------------------------------------
// Desc : CPortalPoints member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPortalPoints () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPortalPoints::CPortalPoints()
{
    // Initialise any class specific items
    OwnsVertices = false;
    OwnerPortal  = NULL;
}

//-----------------------------------------------------------------------------
// Name : CPortalPoints () (Alternate Constructor)
// Desc : Constructor for this class. Duplicates / Stores verts passed in.
//-----------------------------------------------------------------------------
CPortalPoints::CPortalPoints( const CPolygon * pPolygon, bool Duplicate )
{
    // Initialise any class specific items
    OwnsVertices = false;
    OwnerPortal  = NULL;
    if (!pPolygon) return;

    // Store or duplicate verts
    if ( Duplicate )
    {
        // Duplicate the vertices
        if (AddVertices( pPolygon->VertexCount ) < 0) throw BCERR_OUTOFMEMORY;
        memcpy( Vertices, pPolygon->Vertices, VertexCount * sizeof(CVertex) );
        OwnsVertices = true;

    } // End if Duplicate
    else
    {
        // Simply store a copy of the pointer info
        Vertices     = pPolygon->Vertices;
        VertexCount  = pPolygon->VertexCount;
        OwnsVertices = false;

    } // End if !Duplicate
}

//-----------------------------------------------------------------------------
// Name : ~CPortalPoints () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
CPortalPoints::~CPortalPoints()
{
    // Clean up after ourselves only if required
    if (OwnsVertices) ReleaseVertices();
    
    // Simply NULL our vertex values
    Vertices    = NULL;
    VertexCount = 0;
}

//-----------------------------------------------------------------------------
// Name : Clip ()
// Desc : Similar to Split except it only keeps whats in front and / or on the
//        plane specified.
// Note : May return a pointer to itself, this should be tested.
//-----------------------------------------------------------------------------
CPortalPoints * CPortalPoints::Clip( const CPlane3& Plane, bool KeepOnPlane )
{
    CPortalPoints * NewPoints = NULL;

    try
    {
        // Classify the points
        CLASSIFYTYPE Location = Plane.ClassifyPoly( Vertices, VertexCount, sizeof(CVertex) );

        // What location ?
        switch ( Location )
        {
            case CLASSIFY_INFRONT:
                // All were in front, simply return this
                NewPoints = this;
                break;

            case CLASSIFY_BEHIND:
                // Nothing was in front
                NewPoints = NULL;
                break;

            case CLASSIFY_ONPLANE:
                // Should we keep the onplane case ?
                if ( KeepOnPlane ) NewPoints = this;
                break;

            case CLASSIFY_SPANNING:

                // Allocate a new set of points
                NewPoints = new CPortalPoints;
                if (!NewPoints) throw std::bad_alloc();

                // Clip the current portal points
                if ( FAILED(Split( Plane, NewPoints, NULL ))) throw BCERR_OUTOFMEMORY;
                break;
        
        } // End Switch

    } // End try block

    catch (...)
    {
        return NULL;

    } // End catch block

    // Success!!
    return NewPoints;
}

//-----------------------------------------------------------------------------
// Name : Split ()
// Desc : This function splits the current portal, against the plane. The two
//        split fragments are returned via the FrontSplit and BackSplit
//        pointers. These must be valid pointers to already allocated portals.
//        However you CAN pass NULL to either parameter, in this case no
//        vertices will be calculated for that fragment.
//-----------------------------------------------------------------------------
HRESULT CPortalPoints::Split( const CPlane3& Plane, CPortalPoints * FrontSplit, CPortalPoints * BackSplit)
{
    // Call base class implementation
    HRESULT ErrCode = CPolygon::Split( Plane, FrontSplit, BackSplit );
    if (FAILED(ErrCode)) return ErrCode;

    // Copy remaining values
    if (FrontSplit) 
    {
        FrontSplit->OwnsVertices = true;
    } // End If

    if (BackSplit) 
    {
        BackSplit->OwnsVertices  = true;
    } // End If

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Desc : CProcessPVS member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CProcessPVS () (Constructor)
// Desc : CProcessPVS Class Constructor
//-----------------------------------------------------------------------------
CProcessPVS::CProcessPVS()
{
    // Reset / Clear all required values
    m_PVSBytesPerSet    = 0;
    m_pLogger           = NULL;
    m_pTree             = NULL;
    m_pParent           = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CProcessPVS () (Destructor)
// Desc : CProcessPVS Class Destructor
//-----------------------------------------------------------------------------
CProcessPVS::~CProcessPVS()
{
    ULONG i;

    // Clean up after ourselves
    for ( i = 0; i < GetPVSPortalCount(); i++ ) if ( GetPVSPortal(i) ) delete GetPVSPortal(i);

    // Clear Vectors
    m_vpPVSPortals.clear();

}

//-----------------------------------------------------------------------------
// Name : Process ()
// Desc : Compiles a visibility set to determine which leaves, of the BSP tree
//        passed in, are visible from which other leaves. This makes use of the
//        portal information previously compiled.
//-----------------------------------------------------------------------------
HRESULT CProcessPVS::Process( CBSPTree * pTree )
{
    HRESULT hRet;

    // Validate values
    if (!pTree) return BCERR_INVALIDPARAMS;

    // Validate Input Data
    if ( pTree->GetPortalCount() == 0 ) return BCERR_BSP_INVALIDTREEDATA;

    // Store tree for compilation
    m_pTree = pTree;

    // Calculate Number Of Bytes needed to store each leafs
	// vis array in BIT form (i.e 8 leafs vis per byte uncompressed)
    m_PVSBytesPerSet = (pTree->GetLeafCount() + 7) / 8;

	// 32 bit align the bytes per set to allow for our early out long conversion
	m_PVSBytesPerSet = (m_PVSBytesPerSet * 3 + 3) & 0xFFFFFFFC;

    // Retrieve all of our one way portals
	hRet = GeneratePVSPortals();
    if ( FAILED( hRet ) ) return hRet;
    if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) return BC_CANCELLED;
    
    // Calculate initial portal visibility
    hRet = InitialPortalVis();
    if ( FAILED( hRet ) ) return hRet;
    if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) return BC_CANCELLED;

    // Perform actual full PVS calculation
    hRet = CalcPortalVis();
    if ( FAILED( hRet ) ) return hRet;
    if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) return BC_CANCELLED;

    // Export the visibility set to the final BSP Tree master array
    hRet = ExportPVS( pTree );
    if ( FAILED( hRet ) ) return hRet;
    if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) return BC_CANCELLED;

    // Success
    return BC_OK;   
}

//-----------------------------------------------------------------------------
// Name : GeneratePVSPortals () (Private)
// Desc : Given a set of two-way portals, create a set of one-way portals as
//        required by the PVS compilation process.
//-----------------------------------------------------------------------------
HRESULT CProcessPVS::GeneratePVSPortals( )
{
    ULONG i, p, PortalCount = m_pTree->GetPortalCount();

    // Allocate enough PVS portals to store one-way copies.
    try 
    { 
        m_vpPVSPortals.resize( PortalCount * 2 );
        for ( i = 0; i < PortalCount * 2; i++ )
        {
            // Allocate a new portal
            m_vpPVSPortals[i] = new CPVSPortal;
            if (!m_vpPVSPortals[i]) throw std::bad_alloc();

        } // Next Portal
    
    } // Try vector ops
    
    // Catch Failures
    catch (...)
    {
        return BCERR_OUTOFMEMORY;
    
    } // End Catch
    
    // Loop through each portal, creating the duplicate points
    for ( i = 0, p = 0; i < PortalCount; i++, p+=2 ) 
    {
        // Retrieve BSP Portal for easy access
        CBSPPortal * pBSPPortal = m_pTree->GetPortal(i);

        // Link this PVS portal the base portal (note, we do not duplicate
        // because the vertices can remain under the ownership of the original 
        // BSP portal to save some memory). (Note only one portal will ultimately
        // own this outer pointer so only one should have 'OwnsPoints' set to true)
        CPortalPoints *pp = AllocPortalPoints( pBSPPortal, false );
        if ( !pp ) return BCERR_OUTOFMEMORY;

        // Create link information for front facing portal
        m_vpPVSPortals[p    ]->Points        = pp;
        m_vpPVSPortals[p    ]->Side          = FRONT_OWNER;
        m_vpPVSPortals[p    ]->Status        = PS_NOTPROCESSED;
        m_vpPVSPortals[p    ]->Plane         = m_pTree->GetNode( pBSPPortal->OwnerNode )->Plane;
        m_vpPVSPortals[p    ]->NeighbourLeaf = pBSPPortal->LeafOwner[ FRONT_OWNER ];
        m_vpPVSPortals[p    ]->OwnsPoints    = true;

        // Store owner portal information (used later)
        pp->OwnerPortal = m_vpPVSPortals[p];

        // Create link information for back facing portal
        m_vpPVSPortals[p + 1]->Points        = pp;
        m_vpPVSPortals[p + 1]->Side          = BACK_OWNER;
        m_vpPVSPortals[p + 1]->Status        = PS_NOTPROCESSED;
        m_vpPVSPortals[p + 1]->Plane         = m_pTree->GetNode( pBSPPortal->OwnerNode )->Plane;
        m_vpPVSPortals[p + 1]->NeighbourLeaf = pBSPPortal->LeafOwner[ BACK_OWNER ];
        m_vpPVSPortals[p + 1]->OwnsPoints    = false;
        
    } // Next Portal

    // Success!!
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : InitialPortalVis ()
// Desc : Performs the first set of visibility calculations between portals.
//        This is essentially a pre-process to speed up the main PVS processing
//-----------------------------------------------------------------------------
HRESULT CProcessPVS::InitialPortalVis()
{
    CPortalPoints  *pp;
    ULONG           p1, p2, i;
    CPlane3         Plane1, Plane2;
    UCHAR          *PortalVis = NULL;
    CPVSPortal     *pPortal1, *pPortal2;

    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger )
    {
        m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Calculating initial PVS portal flow \t\t- " ) );
        m_pLogger->SetRewindMarker( LOG_PVS );
        m_pLogger->LogWrite( LOG_PVS, 0, false, _T("0%%" ) );
        m_pLogger->SetProgressRange( GetPVSPortalCount() );
        m_pLogger->SetProgressValue( 0 );
    } 
    // *************************
    // *    End of Logging     *
    // *************************


    try
    {
        // Allocate temporary visibility buffer
        if ( !(PortalVis = new UCHAR[ GetPVSPortalCount() ])) throw std::bad_alloc();

        // Loop through the portal array allocating and checking 
        // portal visibility against every other portal
        for ( p1 = 0; p1 < GetPVSPortalCount(); p1++) 
        {
            // Update progress
            if (!m_pParent->TestCompilerState()) break;
            if ( m_pLogger ) m_pLogger->UpdateProgress( );

            // Retrieve first portal for easy access
            pPortal1 = GetPVSPortal( p1 );

            // Retrieve portal's plane
            GetPortalPlane( pPortal1, Plane1 );

            // Allocate memory for portal visibility info
            if (!(pPortal1->PossibleVis = new UCHAR[m_PVSBytesPerSet])) throw std::bad_alloc();
            ZeroMemory( pPortal1->PossibleVis, m_PVSBytesPerSet );

            // Clear temporary buffer
            ZeroMemory( PortalVis, GetPVSPortalCount() );

            // For this portal, loop through all other portals
            for ( p2 = 0; p2 < GetPVSPortalCount(); p2++) 
            {
                // Don't test against self
                if (p2 == p1) continue;

                // Retrieve second portal for easy access
                pPortal2 = GetPVSPortal( p2 );

                // Retrieve portal's plane
                GetPortalPlane( pPortal2, Plane2 );

                // Test to see if any of p2's points are in front of p1's plane
                pp = pPortal2->Points;
                for ( i = 0; i < pp->VertexCount; i++) 
                {
	                if ( Plane1.ClassifyPoint( pp->Vertices[i] ) == CLASSIFY_INFRONT ) break;
                
                } // Next Portal Vertex

                // If the loop reached the end, there were no points in front so continue
                if ( i == pp->VertexCount ) continue;
                
                // Test to see if any of p1's portal points are Behind p2's plane.
                pp = pPortal1->Points;
                for ( i = 0; i < pp->VertexCount; i++) 
                {
	                if ( Plane2.ClassifyPoint( pp->Vertices[i] ) == CLASSIFY_BEHIND ) break;
                
                } // Next Portal Vertex

                // If the loop reached the end, there were no points in front so continue
                if ( i == pp->VertexCount ) continue;

                // Fill out the temporary portal visibility array
                PortalVis[p2] = 1;		

            } // Next Portal 2

            // Now flood through all the portals which are visible
            // from the source portal through into the neighbour leaf
            // and flag any leaves which are visible (the leaves which
            // remain set to 0 can never possibly be seen from this portal)
            pPortal1->PossibleVisCount = 0;
            PortalFlood( pPortal1, PortalVis, pPortal1->NeighbourLeaf );

        } // Next Portal

        // If we're cancelled, clean up and return
        if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) 
        {
            if ( PortalVis) delete[]PortalVis;
            return BC_CANCELLED;
        
        } // End if Cancelled.

    } // End Try Block

    // Catch Bad Allocations
    catch ( std::bad_alloc )
    {
        // Clean up and return (Failure)
        if (PortalVis) delete []PortalVis;
        if ( m_pLogger ) m_pLogger->ProgressFailure( LOG_PVS );
        return BCERR_OUTOFMEMORY;
    
    } // End Catch Block

    // Clean up
    if (PortalVis) delete []PortalVis;

    // Success!!
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_PVS );
    return BC_OK;
}


//-------------------------------------------------------------------------------------
// Name : PortalFlood() (Recursive)
// Desc : This function does a basic flood fill THROUGH all of the portals from this
//        leaf recursively and flags all leaves which the flood fill reached
//-------------------------------------------------------------------------------------
void CProcessPVS::PortalFlood( CPVSPortal * SourcePortal, unsigned char PortalVis[], unsigned long Leaf )
{
    CBSPLeaf * pLeaf = m_pTree->GetLeaf( Leaf );

    // Test the source portals 'Possible Visibility' list
    // to see if this leaf has already been set.
    if ( GetPVSBit( SourcePortal->PossibleVis, Leaf ) ) return;

    // Set the possible visibility bit for this leaf
    SetPVSBit( SourcePortal->PossibleVis, Leaf );

    // Increase portals 'Complexity' level
    SourcePortal->PossibleVisCount++;

    // Loop through all portals in this leaf (remember the portal numbering
    // in the leaves match up with the originals, not our PVS portals )
    for ( ULONG i = 0; i < pLeaf->PortalIndices.size(); i++)	
    {
        // Find correct portal index (the one IN this leaf (not Neighbouring))
        ULONG PortalIndex = pLeaf->PortalIndices[ i ] * 2;
        if ( GetPVSPortal( PortalIndex )->NeighbourLeaf == Leaf ) PortalIndex++;

        // If this portal was not flagged as allowed to pass through, then continue to next portal
        if ( !PortalVis[ PortalIndex ] ) continue;

        // Flood fill out through this portal
        PortalFlood( SourcePortal, PortalVis, GetPVSPortal( PortalIndex )->NeighbourLeaf );
    
    } // Next Leaf Portal
}

//-------------------------------------------------------------------------------------
// Name : CalcPortalVis() 
// Desc : Top level PVS calculation function which starts the recursion for each portal
//-------------------------------------------------------------------------------------
HRESULT CProcessPVS::CalcPortalVis()
{
    ULONG   i;
    HRESULT hRet;
    PVSDATA PVSData;
    
    // If we want to perform a quick vis (not at all accurate) we can
	// simply use the possible vis bits array as our pvs bytes.
	if ( !m_OptionSet.FullCompile ) 
    {
		for ( i = 0; i < GetPVSPortalCount(); i++ ) 
        {
            CPVSPortal * pPortal = GetPVSPortal( i );
			pPortal->ActualVis = pPortal->PossibleVis;
		
        } // Next Portal
		
        // We are finished here
        return BC_OK;

	} // End if !FullCompile

    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger )
    {
        m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Full PVS compile progress \t\t\t- " ) );
        m_pLogger->SetRewindMarker( LOG_PVS );
        m_pLogger->LogWrite( LOG_PVS, 0, false, _T("0%%" ) );
        m_pLogger->SetProgressRange( GetPVSPortalCount() );
        m_pLogger->SetProgressValue( 0 );
    } 
    // *************************
    // *    End of Logging     *
    // *************************

    try
    {
        // Clear out our PVSData struct
        ZeroMemory( &PVSData, sizeof(PVSDATA) );

        // Lets process those portal bad boys!! ;)
        for ( i = 0; i != -1; i = GetNextPortal() )
        {
            CPVSPortal * pPortal = GetPVSPortal( i );
        
            // Update Progress
            if (!m_pParent->TestCompilerState()) throw BC_CANCELLED;
            if (m_pLogger) m_pLogger->UpdateProgress();

            // Fill our our initial data structure
            PVSData.SourcePoints    = pPortal->Points;
            PVSData.VisBits         = pPortal->PossibleVis;
            GetPortalPlane( pPortal, PVSData.TargetPlane );
		    
            // Allocate the portals actual visibility array
            pPortal->ActualVis = new UCHAR[ m_PVSBytesPerSet ];
            if (!pPortal->ActualVis) throw std::bad_alloc(); // VC++ Compat

            // Set initial visibility to off for all leaves
            ZeroMemory( pPortal->ActualVis, m_PVSBytesPerSet );

            // Step in and begin processing this portal
            hRet = RecursePVS( pPortal->NeighbourLeaf, pPortal, PVSData );
            if ( FAILED( hRet ) ) throw hRet;

            // We've finished processing this portal
            pPortal->Status = PS_PROCESSED;

        } // Next Portal

    } // End Try Block

    // Catch all failures
    catch (std::bad_alloc)
    {
        // Failed to allocate
        if ( m_pLogger ) m_pLogger->ProgressFailure( LOG_PVS );
        return BCERR_OUTOFMEMORY;

    } // End if

    catch ( HRESULT& e )
    {
        // Arbitrary Error
        if ( m_pLogger && FAILED(e) ) m_pLogger->ProgressFailure( LOG_PVS );
        return e;

    } // End if

    // Success!!
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_PVS );
    return BC_OK;
}

//-------------------------------------------------------------------------------------
// Name : GetNextPortal() 
// Desc : Function that returns the next portal in order of complexity.
// Note : This means that all the least complex portals are processed first so that
//        these portal's vis info can be used in the early out system in RecursePVS
//        to help speed things up.
//-------------------------------------------------------------------------------------
ULONG CProcessPVS::GetNextPortal( )
{
    CPVSPortal * pPortal;
	long PortalIndex = -1, Min = 999999, i;

    // Loop through all portals
	for ( i = 0; i < (signed)GetPVSPortalCount(); i++ ) 
    {
        pPortal = GetPVSPortal(i);

		// If this portal's complexity is the lowest and it has 
        // not already been processed then we could use it.
		if ( pPortal->PossibleVisCount < Min && pPortal->Status == PS_NOTPROCESSED) 
        {
			Min = pPortal->PossibleVisCount;
			PortalIndex = i;

		} // End if Least Complex

	} // Next Portal

	// Set our status flag to currently being worked on =)
	if ( PortalIndex > -1) GetPVSPortal( PortalIndex )->Status = PS_PROCESSING;

    // Return the next portal
	return PortalIndex;
}

//-------------------------------------------------------------------------------------
// Name : RecursePVS() 
// Desc : PVS recursion function, steps through the portals and calcs true visibility
//-------------------------------------------------------------------------------------
HRESULT CProcessPVS::RecursePVS( ULONG Leaf, CPVSPortal * SourcePortal, PVSDATA & PrevData )
{
    ULONG           i,j;
    bool            More;
    ULONG          *Test, *Possible, *Vis;

    PVSDATA         Data;
    CPVSPortal     *GeneratorPortal;
    CPlane3         ReverseGenPlane, SourcePlane;
    CPortalPoints  *SourcePoints, *GeneratorPoints, *NewPoints;

    // Store the leaf for easy access
    CBSPLeaf * pLeaf = m_pTree->GetLeaf( Leaf );

    // Mark this leaf as visible
    SetPVSBit( SourcePortal->ActualVis, Leaf );

    // Allocate our current visibility buffer
    Data.VisBits = new UCHAR[ m_PVSBytesPerSet ];
    if (!Data.VisBits) throw std::bad_alloc(); // VC++ Compat

    // Store data we will be using inside the loop
    Possible    = (ULONG*)Data.VisBits;
    Vis         = (ULONG*)SourcePortal->ActualVis;
    GetPortalPlane( SourcePortal, SourcePlane );

    // Check all portals for flow into other leaves
    for ( i = 0; i < pLeaf->PortalIndices.size(); i++ )
    {
        // Find correct portal index (the one IN this leaf (not Neighbouring))
        ULONG PortalIndex = pLeaf->PortalIndices[ i ] * 2;
        if ( GetPVSPortal( PortalIndex )->NeighbourLeaf == Leaf ) PortalIndex++;

        // Store the portal for easy access
        GeneratorPortal = GetPVSPortal( PortalIndex );

        // We can't possibly recurse through this portal if it's neighbour
        // leaf is set to invisible in the target portals PVS
        if ( !GetPVSBit( PrevData.VisBits, GeneratorPortal->NeighbourLeaf ) ) continue;

        // If the portal can't see anything we haven't already seen, skip it
        if ( GeneratorPortal->Status == PS_PROCESSED ) 
            Test = (ULONG*)GeneratorPortal->ActualVis;
        else
            Test = (ULONG*)GeneratorPortal->PossibleVis;

        More = false;
        // Check to see if we have processed as much as we need to
        // this is an early out system. We check in 32 bit chunks to
        // help speed the process up a little.
        for ( j = 0; j < m_PVSBytesPerSet / sizeof(ULONG); j++ )
        {
            Possible[j] = ((ULONG*)PrevData.VisBits)[j] & Test[j];
            if ( Possible[j] & ~Vis[j] ) More = true;

        } // Next 32 bit Chunk

        // Can we see anything new ??
        if ( !More ) continue;

        // The current generator plane will become the next recursions target plane
        GetPortalPlane( GeneratorPortal, Data.TargetPlane );

        // We can't recurse out of a coplanar face, so check it
        ReverseGenPlane.Normal   = -Data.TargetPlane.Normal;
        ReverseGenPlane.Distance = -Data.TargetPlane.Distance;
        if ( ReverseGenPlane.Normal.FuzzyCompare( PrevData.TargetPlane.Normal, 0.001f ) ) continue;

        // Clip the generator portal to the source. If none remains, continue.
        GeneratorPoints = GeneratorPortal->Points->Clip( SourcePlane, false );
        if ( GeneratorPoints != GeneratorPortal->Points ) FreePortalPoints( GeneratorPortal->Points );
        if (!GeneratorPoints) continue;

        // The second leaf can only be blocked if coplanar
        if ( !PrevData.TargetPoints )
        {
            Data.SourcePoints = PrevData.SourcePoints;
            Data.TargetPoints = GeneratorPoints;
            RecursePVS( GeneratorPortal->NeighbourLeaf, SourcePortal, Data );
            FreePortalPoints( GeneratorPoints );
            continue;

        } // End if Previous Points

        // Clip the generator portal to the previous target. If none remains, continue.
        NewPoints = GeneratorPoints->Clip( PrevData.TargetPlane, false );
        if ( NewPoints != GeneratorPoints ) FreePortalPoints( GeneratorPoints );
        GeneratorPoints = NewPoints;
        if (!GeneratorPoints) continue;

        // Make a copy of the source portals points
        SourcePoints = new CPortalPoints( PrevData.SourcePoints, true );

        // Clip the source portal
        NewPoints = SourcePoints->Clip( ReverseGenPlane, false );
        if ( NewPoints != SourcePoints ) FreePortalPoints( SourcePoints );
        SourcePoints = NewPoints;

        // If none remains, continue to the next portal
        if ( !SourcePoints ) { FreePortalPoints(  GeneratorPoints ); continue; }

        // Lets go Clipping :)
        if ( m_OptionSet.ClipTestCount > 0 )
        {
            GeneratorPoints = ClipToAntiPenumbra( SourcePoints, PrevData.TargetPoints, GeneratorPoints, false ); 
            if (!GeneratorPoints) { FreePortalPoints( SourcePoints ); continue; }
        
        } // End if 1 Clip Test

        if ( m_OptionSet.ClipTestCount > 1 )
        {
            GeneratorPoints = ClipToAntiPenumbra( PrevData.TargetPoints, SourcePoints, GeneratorPoints, true ); 
            if (!GeneratorPoints) { FreePortalPoints( SourcePoints ); continue; }
        
        } // End if 2 Clip Tests

        if ( m_OptionSet.ClipTestCount > 2 )
        {
            SourcePoints = ClipToAntiPenumbra( GeneratorPoints, PrevData.TargetPoints, SourcePoints, false ); 
            if (!SourcePoints) { FreePortalPoints( GeneratorPoints ); continue; }
        
        } // End if 3 Clip Test

        if ( m_OptionSet.ClipTestCount > 3 )
        {
            SourcePoints = ClipToAntiPenumbra( PrevData.TargetPoints, GeneratorPoints, SourcePoints, true ); 
            if (!SourcePoints) { FreePortalPoints( GeneratorPoints ); continue; }
        
        } // End if 4 Clip Test

        // Store data for next recursion
        Data.SourcePoints = SourcePoints;
        Data.TargetPoints = GeneratorPoints;

        // Flow through it for real
        RecursePVS( GeneratorPortal->NeighbourLeaf, SourcePortal, Data );

        // Clean up
        FreePortalPoints( SourcePoints );
        FreePortalPoints( GeneratorPoints );

    } // Next Portal

    // Clean up
    if (Data.VisBits) delete []Data.VisBits;
        
    // Success
    return BC_OK;
}

//-------------------------------------------------------------------------------------
// Name : ClipToAntiPenumbra() 
// Desc : Clips the portals to one another using the generated Anti-Penumbra.
//-------------------------------------------------------------------------------------
CPortalPoints * CProcessPVS::ClipToAntiPenumbra( CPortalPoints * Source, CPortalPoints * Target, CPortalPoints * Generator, bool ReverseClip )
{
    CPlane3         Plane;
    CVector3        v1, v2;
    float           Length;
    ULONG           Counts[3];
    ULONG           i, j, k, l;
    bool            ReverseTest;
    CPortalPoints  *NewPoints;

    // Check all combinations
    for ( i = 0; i < Source->VertexCount; i++ )
    {
        // Build first edge
        l = ( i + 1 ) % Source->VertexCount;
        v1 = Source->Vertices[l] - Source->Vertices[i];

        // Find a vertex belonging to the generator that makes a plane
        // which puts all of the vertices of the target on the front side
        // and all of the vertices of the source on the back side
        for ( j = 0; j < Target->VertexCount; j++ )
        {
            // Build second edge
            v2 = Target->Vertices[ j ] - Source->Vertices[ i ];
            Plane.Normal = v1.Cross( v2 );

            // If points don't make a valid plane, skip it
            Length = Plane.Normal.x * Plane.Normal.x +
                     Plane.Normal.y * Plane.Normal.y +
                     Plane.Normal.z * Plane.Normal.z;
            if ( Length < 0.1f ) continue;

            // Normalize the plane normal
            Length = 1 / sqrtf( Length );
            Plane.Normal *= Length;

            // Calculate the plane distance
            Plane.Distance = -Target->Vertices[ j ].Dot( Plane.Normal );

            // Find out which side of the generated separating plane has the source portal
            ReverseTest = false;
            for ( k = 0; k < Source->VertexCount; k++ )
            {
                // Skip if it matches other verts
                if ( k == i || k == l ) continue;

                // Classify the point
                CLASSIFYTYPE Location = Plane.ClassifyPoint( Source->Vertices[ k ] );
                if ( Location == CLASSIFY_BEHIND )
                {
                    // Source is on the negative side, so we want all pass
                    // and target on the positive side.
                    ReverseTest = false;
                    break;
                
                } // End If Behind
                else if ( Location == CLASSIFY_INFRONT )
                {
                    // Source is on the positive sode, so we want all pass
                    // and target on the negative side.
                    ReverseTest = true;
                    break;
                
                } // End if In Front

            } // Next Source Vertex

            // Planar with the source portal ?
            if ( k == Source->VertexCount ) continue;

            // Flip the normal if the source portal is backwards
            if ( ReverseTest ) { Plane.Normal = -Plane.Normal; Plane.Distance = -Plane.Distance; }

            // If all of the pass portal points are now on the positive 
            // side then this is the separating plane.
            ZeroMemory( Counts, 3 * sizeof(ULONG) );
            for ( k = 0; k < Target->VertexCount; k++ )
            {
                // Skip if the two match
                if ( k == j ) continue;

                // Classify the point
                CLASSIFYTYPE Location = Plane.ClassifyPoint( Target->Vertices[ k ] );
                if ( Location == CLASSIFY_BEHIND )
                    break;
                else if ( Location == CLASSIFY_INFRONT )
                    Counts[0]++;
                else
                    Counts[2]++;

            } // Next Target Vertex

            // Points on the negative side ?
            if ( k != Target->VertexCount ) continue;

            // Planar with separating plane ?
            if ( Counts[0] == 0 ) continue;

            // Flip the normal if we want the back side
            if ( ReverseClip ) { Plane.Normal = -Plane.Normal; Plane.Distance = -Plane.Distance; }

            // Clip the target by the separating plane
            NewPoints = Generator->Clip( Plane, false );
            if ( NewPoints != Generator ) FreePortalPoints( Generator );
            Generator = NewPoints;

            // Target is not visible ?
            if (!Generator) return NULL;

        } // Next Target Vertex

    } // Next Source Vertex

    // Success!!
    return Generator;
}

//-------------------------------------------------------------------------------------
// Name : ExportPVS() 
// Desc : Exports all calculated PVS information and stores it within the specified
//        BSP Tree object.
//-------------------------------------------------------------------------------------
HRESULT CProcessPVS::ExportPVS( CBSPTree * pTree )
{
    UCHAR * PVSData = NULL;
    UCHAR * LeafPVS = NULL;
    ULONG   PVSWritePtr = 0, i, p, j;
    
    try
    {
        // *************************
        // * Write Log Information *
        // *************************
        if ( m_pLogger )
        {

            #if ( PVS_COMPRESSDATA )
                m_pLogger->LogWrite( LOG_PVS, 0, true, _T("ZRLE compressing PVS data for export \t\t- " ) );
            #else
                m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Building final PVS data for export \t\t- " ) );
            #endif

            m_pLogger->SetRewindMarker( LOG_PVS );
            m_pLogger->LogWrite( LOG_PVS, 0, false, _T("0%%" ) );
            m_pLogger->SetProgressRange( pTree->GetLeafCount() );
            m_pLogger->SetProgressValue( 0 );
        } 
        // *************************
        // *    End of Logging     *
        // *************************

        // Reserve Enough Space to hold every leafs PVS set
        PVSData = new UCHAR[pTree->GetLeafCount() * (m_PVSBytesPerSet * 2)];
        if (!PVSData) throw std::bad_alloc();

        // Set all visibility initially to off
        ZeroMemory( PVSData, pTree->GetLeafCount() * (m_PVSBytesPerSet * 2));

        // Allocate enough memory for a single leaf set
        LeafPVS = new UCHAR[ m_PVSBytesPerSet ];
        if (!LeafPVS) throw std::bad_alloc();

        // Loop round each leaf and collect the vis info
        // this is all OR'd together and ZRLE compressed
        // Then finally stored in the master array
        for ( i = 0; i < pTree->GetLeafCount(); i++ ) 
        {
            CBSPLeaf * pLeaf = pTree->GetLeaf(i);
            
            // Update progress
            if (!m_pParent->TestCompilerState()) break;
            if ( m_pLogger ) m_pLogger->UpdateProgress( );

            // Clear Temp PVS Array Buffer
            ZeroMemory( LeafPVS, m_PVSBytesPerSet );
            pLeaf->PVSIndex = PVSWritePtr;
        
            // Current leaf is always visible
            SetPVSBit( LeafPVS, i );
            
            // Loop through all portals in this leaf
            for ( p = 0; p < pLeaf->PortalIndices.size(); p++ ) 
            {
                // Find correct portal index (the one IN this leaf)
                ULONG PortalIndex = pLeaf->PortalIndices[ p ] * 2;
                if ( GetPVSPortal( PortalIndex )->NeighbourLeaf == i ) PortalIndex++;

                // Or the vis bits together
                for ( j = 0; j < m_PVSBytesPerSet; j++ ) 
                {
	                LeafPVS[j] |= GetPVSPortal( PortalIndex )->ActualVis[j];
                
                } // Next PVS Byte

            } // Next Portal

            #if ( PVS_COMPRESSDATA )
     
                // Compress the leaf set here and update our master write pointer
                PVSWritePtr += CompressLeafSet( PVSData, LeafPVS, PVSWritePtr );

            #else

                // Copy the data into the Master PVS Set
                memcpy( &PVSData[ PVSWritePtr ], LeafPVS, m_PVSBytesPerSet );
                PVSWritePtr += m_PVSBytesPerSet;

            #endif

        } // Next Leaf

        // Clean up after ourselves
        delete []LeafPVS;
        LeafPVS = NULL;

        // Pass this data off to the BSP Tree (data, size, compressed)
        if (FAILED(pTree->SetPVSData( PVSData, PVSWritePtr, PVS_COMPRESSDATA ))) throw std::bad_alloc();

        // Free our PVS buffer
        delete []PVSData;

        // If we're cancelled, bail
        if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) return BC_CANCELLED;

    } // End Try Block

    catch (...)
    {
        // Clean up and return (Failure)
        if ( LeafPVS ) delete []LeafPVS;
        if ( PVSData ) delete []PVSData;
        if ( m_pLogger ) m_pLogger->ProgressFailure( LOG_PVS );
        return BCERR_OUTOFMEMORY;
    
    } // End Catch Block

    // Success!!
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_PVS );
    return BC_OK;
}

//-------------------------------------------------------------------------------------
// Name : CompressLeafSet () (Private)
// Desc : ZRLE Compresses the uncompressed vis bit array which was passed in, and
//        compresses and adds it to the master PVS Array, this function returns
//        the size of the compressed set so we can update our write pointer.
//-------------------------------------------------------------------------------------
ULONG CProcessPVS::CompressLeafSet ( UCHAR MasterPVS[], const UCHAR VisArray[], ULONG WritePos)
{
	ULONG   RepeatCount;
	UCHAR  *pDest = &MasterPVS[ WritePos ];
	UCHAR  *pDest_p;
	
    // Set dynamic pointer to start position
    pDest_p = pDest;
	
    // Loop through and compress the set
	for ( ULONG j = 0; j < m_PVSBytesPerSet; j++ ) 
    {
        // Store the current 8 leaves
		*pDest_p++ = VisArray[j];

        // Don't compress if all bits are not zero
		if ( VisArray[j] ) continue;

        // Count the number of 0 bytes
		RepeatCount = 1;
		for ( j++; j < m_PVSBytesPerSet; j++ ) 
        {
            // Keep counting until byte != 0 or we reach our max repeat count
			if ( VisArray[j] || RepeatCount == 255) break; else RepeatCount++;
		
        } // Next Byte
		
        // Store our repeat count
        *pDest_p++ = (UCHAR)RepeatCount;

        // Step back one byte because the outer loop
        // will increment. We are already at the correct pos.
		j--;
	
    } // Next Byte
	
    // Return written size
	return pDest_p - pDest;
}

//-------------------------------------------------------------------------------------
// Name : GetPortalPlane() 
// Desc : Calculates the correct plane orientation for the specified portal
//-------------------------------------------------------------------------------------
void CProcessPVS::GetPortalPlane( const CPVSPortal * pPortal, CPlane3& Plane )
{

    // Store plane information
    Plane = *m_pTree->GetPlane( pPortal->Plane );

    // Swap sides if necessary
    if ( pPortal->Side == BACK_OWNER )
    {
        Plane.Normal   = -Plane.Normal;
        Plane.Distance = -Plane.Distance;
    
    } // End if Swap Sides
}

//-------------------------------------------------------------------------------------
// Name : GetPVSBit() (Static, Private)
// Desc : Pass in a destination leaf and it will return true if it's corresponding 
//        uncompressed bit position is set to 1 in the array passed
//-------------------------------------------------------------------------------------
bool CProcessPVS::GetPVSBit( UCHAR VisArray[], ULONG DestLeaf )
{
    return (VisArray[ DestLeaf >> 3 ] & (1 << ( DestLeaf & 7))) != 0;
}

//-------------------------------------------------------------------------------------
// Name : SetPVSBit()
// Desc : Pass in a destination leaf and it will set it's corresponding uncompressed
//        bit position to the value specified, in the array passed
//-------------------------------------------------------------------------------------
void CProcessPVS::SetPVSBit( UCHAR VisArray[], ULONG DestLeaf, bool Value /* = true */ )
{
    // Set / remove bit depending on the value
    if ( Value == true )
    {
        VisArray[ DestLeaf >> 3 ] |=  (1 << ( DestLeaf & 7 ));
    }
    else
    {
        VisArray[ DestLeaf >> 3 ] &= ~(1 << ( DestLeaf & 7 ));
    
    } // End if Value
}

//-----------------------------------------------------------------------------
// Name : AllocPortalPoints () (Static, Private)
// Desc : Simply allocate a brand new CPortalPoints object and return it.
// Note : Provided for ease of use, and to provide some level of portability
//        without polluting the main code base.
//-----------------------------------------------------------------------------
CPortalPoints * CProcessPVS::AllocPortalPoints( const CPolygon * pPolygon, bool Duplicate )
{
    CPortalPoints * NewPoints = NULL;

    try
    {
        // Attempt to allocate a new set of points
        NewPoints = new CPortalPoints( pPolygon, Duplicate );
        if (!NewPoints) throw std::bad_alloc();

    } // End try block

    catch (HRESULT)
    {
        // Constructor throws HRESULT
        if (NewPoints) delete NewPoints;
        return NULL;
    
    } // End Catch

    catch ( std::bad_alloc )
    {
        // Failed to allocate
        return NULL;
    
    } // End Catch

    // Success!!
    return NewPoints;
}

//-----------------------------------------------------------------------------
// Name : FreePortalPoints() (Static, Private)
// Desc : Releases a set of portal points.
// Note : Only releases if it is not owned by a physical portal.
//-----------------------------------------------------------------------------
void CProcessPVS::FreePortalPoints( CPortalPoints * pPoints )
{
    // Validate Parameters
    if (!pPoints) return;

    // We are only allowed to delete NON-Owned point sets
    if ( pPoints->OwnerPortal == NULL ) delete pPoints;

}