//-----------------------------------------------------------------------------
// File: ProcessPRT.cpp
//
// Desc: This source file houses the primary portal compiler, of which an
//       object can spawn to build a set of portals for any arbitrary BSP Tree.
//       This compile process is bi-directionally linked to the BSP Tree in 
//       that it both gets and sets data within the BSP Tree and it's
//       associated classes.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CProcessPRT Specific Includes
//-----------------------------------------------------------------------------
#include "ProcessPRT.h"
#include "CCompiler.h"
#include "CBSPTree.h"
#include "..\\Support Source\\CBounds.h"
#include "..\\Support Source\\CPlane.h"

//-----------------------------------------------------------------------------
// Name : CProcessPRT () (Constructor)
// Desc : CProcessPRT Class Constructor
//-----------------------------------------------------------------------------
CProcessPRT::CProcessPRT()
{
    // Reset / Clear all required values
    m_pParent     = NULL;
    m_pLogger     = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CProcessPRT () (Destructor)
// Desc : CProcessPRT Class Destructor
//-----------------------------------------------------------------------------
CProcessPRT::~CProcessPRT()
{
    // Clean up after ourselves
}

//-----------------------------------------------------------------------------
// Name : Process ()
// Desc : Compiles a set of portals by essentially sending a large polygon
//        per node through the tree clipping them down until they reach empty
//        space. If they are valid portals, they are added to the master array.
//-----------------------------------------------------------------------------
HRESULT CProcessPRT::Process( CBSPTree * pTree )
{
    HRESULT      ErrCode;
    CBounds3     PortalBounds;
    CBSPPortal * InitialPortal = NULL;
    CBSPNode   * CurrentNode   = NULL;
    CBSPNode   * RootNode      = NULL;
    CPlane3    * NodePlane     = NULL;
    CBSPPortal * PortalList    = NULL;

    // Validate values
    if (!pTree) return BCERR_INVALIDPARAMS;

    // Store tree for compilation
    m_pTree = pTree;

    try 
    {
        // *************************
        // * Write Log Information *
        // *************************
        if ( m_pLogger )
        {
            m_pLogger->LogWrite( LOG_PRT, 0, true, _T("Compiling scene portal information \t\t- " ) );
            m_pLogger->SetRewindMarker( LOG_PRT );
            m_pLogger->LogWrite( LOG_PRT, 0, false, _T("0%%" ) );
            m_pLogger->SetProgressRange( pTree->GetNodeCount() );
            m_pLogger->SetProgressValue( 0 );
        } 
        // *************************
        // *    End of Logging     *
        // *************************

        // Store required values ready for use.
        if (!(RootNode = m_pTree->GetNode(0))) throw BCERR_BSP_INVALIDTREEDATA;

        // Create a portal for each node
	    for (unsigned long i = 0; i < pTree->GetNodeCount(); i++) 
        {
            // Update progress
            if ( m_pParent && !m_pParent->TestCompilerState()) return BC_CANCELLED;
            if ( m_pLogger ) m_pLogger->UpdateProgress( );

            // Store required values ready for use.
            if (!(CurrentNode = m_pTree->GetNode(i))) throw BCERR_BSP_INVALIDTREEDATA;
            if (!(NodePlane   = m_pTree->GetPlane(CurrentNode->Plane))) throw BCERR_BSP_INVALIDTREEDATA;

            // Skip any that have solid space behind them
            if ( CurrentNode->Back == BSP_SOLID_LEAF ) continue;
            
            // Allocate a new initial portal for clipping
            if (!(InitialPortal = CBSPTree::AllocBSPPortal())) throw BCERR_OUTOFMEMORY;
            
            // Use root node for portal bounding box
            PortalBounds = RootNode->Bounds;
            
            // Generate the portal polygon for the current node
            InitialPortal->GenerateFromPlane( *NodePlane, PortalBounds );
            InitialPortal->OwnerNode = i;

            // Clip the portal and obtain a list of all fragments
		    PortalList = ClipPortal(0, InitialPortal );

            // Clear the initial portal value, we no longer own this
            InitialPortal = NULL;
            
            // Add any valid fragments to the final portal list
            if (PortalList) 
            {
                if (FAILED(ErrCode = AddPortals( PortalList ))) throw ErrCode;
            } // End If PortalList

	    } // Next Node

    } // End Try

    catch ( HRESULT& Error ) 
    {
        // If we dropped here, something failed
        if ( InitialPortal ) delete InitialPortal;
        if ( m_pLogger ) m_pLogger->ProgressFailure( LOG_PRT );
        return Error;

    } // End Catch

    // Success
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_PRT );
    return BC_OK;
    
}

//-----------------------------------------------------------------------------
// Name : ClipPortal () (Recursive) (Private)
// Desc : This recursive function repeatedly clips the current portal to the 
//        tree until it ends up in a leaf at which point it is returned.
//-----------------------------------------------------------------------------
CBSPPortal * CProcessPRT::ClipPortal( unsigned long Node, CBSPPortal * pPortal )
{
    // 52 Bytes including Parameter list (based on __thiscall declaration)
	CBSPPortal   * PortalList     = NULL, *FrontPortalList = NULL;
	CBSPPortal   * BackPortalList = NULL, *Iterator        = NULL;
	CBSPPortal   * FrontSplit     = NULL, *BackSplit       = NULL;
    CBSPNode     * CurrentNode    = NULL;
    CPlane3      * CurrentPlane   = NULL;
    unsigned long  OwnerPos, LeafIndex;

    // Validate Requirements
    if (!pPortal || !m_pTree) throw BCERR_INVALIDPARAMS;

    // Store node for quick access
    if (!(CurrentNode  = m_pTree->GetNode( Node ))) throw BCERR_BSP_INVALIDTREEDATA;
    if (!(CurrentPlane = m_pTree->GetPlane(CurrentNode->Plane))) throw BCERR_BSP_INVALIDTREEDATA;

    // Classify the portal against this nodes plane
	switch (CurrentPlane->ClassifyPoly(pPortal->Vertices, pPortal->VertexCount, sizeof(CVertex))) 
    {
		
        case CLASSIFY_ONPLANE:

            // The Portal has to be sent down Both sides of the tree and tracked. Send it down 
			// front first but DO NOT delete any bits that end up in solid space, just ignore them.	
            if (CurrentNode->Front < 0 ) 
            {
				// The Front is a Leaf, determine which side of the node it fell
                LeafIndex   = abs(CurrentNode->Front + 1);
                OwnerPos    = ClassifyLeaf( LeafIndex, pPortal->OwnerNode );

                // Found the leaf below?
                if ( OwnerPos != NO_OWNER) 
                {
                    // This portal is added straight to the front list
                    pPortal->LeafOwner[OwnerPos] = LeafIndex;
				    pPortal->NextPortal          = NULL;
				    FrontPortalList	             = pPortal;
                    pPortal->LeafCount++;
                
                } // End if leaf found
                else 
                {
                    delete pPortal;
                    return NULL;

                } // End If no leaf found

			} // End if child leaf
            else 
            {
				// Send the Portal Down the Front List and get returned a list of PortalFragments 
				// that survived the Front Tree
				FrontPortalList = ClipPortal(CurrentNode->Front, pPortal);

			} // End If child node

			// If nothing survived return here.
            if (FrontPortalList == NULL) return NULL;

            //// If the back is solid, just return the front list
            if ( CurrentNode->Back == BSP_SOLID_LEAF ) return FrontPortalList;

			// Loop through each front list fragment and send it down the back branch
            pPortal = FrontPortalList;
			while ( pPortal != NULL ) 
            {	
				CBSPPortal * NextPortal = pPortal->NextPortal;
				BackPortalList	        = NULL;

                // Empty leaf behind?
                if ( CurrentNode->Back < 0 )
                {
                    // The back is a Leaf, determine which side of the node it fell
                    LeafIndex   = abs(CurrentNode->Back + 1);
                    OwnerPos    = ClassifyLeaf( LeafIndex, pPortal->OwnerNode );

                    // Found the leaf below?
                    if ( OwnerPos != NO_OWNER ) 
                    {
                        // Attach it to the back list
                        pPortal->LeafOwner[OwnerPos] = LeafIndex;
                        pPortal->NextPortal          = BackPortalList;
                        BackPortalList               = pPortal;
                        pPortal->LeafCount++;
                    
                    } // End if leaf found
                    else 
                    {
                        // Delete the portal, but continue to the next fragment
                        delete pPortal;
                        continue;
                    
                    } // End If no leaf found

                } // End if child leaf
                else 
                {
                    // Send the Portal Down the back and get returned a list of PortalFragments 
                    // that survived the Front Tree
                    BackPortalList	= ClipPortal(CurrentNode->Back, pPortal);

                } // End If child node

				// Anything in the back list?
				if (BackPortalList != NULL) 
                {
                    // Iterate to the end to get the last item in the back list
					Iterator = BackPortalList;
                    while ( Iterator->NextPortal != NULL) Iterator = Iterator->NextPortal;
					
                    // Attach the last fragment to the first fragment from the previous iteration.
					Iterator->NextPortal = PortalList;

					// Portal List now points at the current complete list of fragment collected so far
					PortalList = BackPortalList;
                    
				} // End if BackPortalList is not empty

                // Move on to next portal
				pPortal = NextPortal;

			} // End While
			
            // Return the full list
            return PortalList;

		case CLASSIFY_INFRONT:
			
            // Either send it down the front tree or add it to the portal 
			// list because it has come out in Empty Space
			if (CurrentNode->Front < 0 ) 
            {
                // The front is a Leaf, determine which side of the node it fell
                LeafIndex   = abs(CurrentNode->Front + 1);
                OwnerPos    = ClassifyLeaf( LeafIndex, pPortal->OwnerNode );

                // Found the leaf below?
                if ( OwnerPos != NO_OWNER ) 
                {
                    // This is just returned straight away, it's in an empty leaf
                    pPortal->LeafOwner[OwnerPos] = LeafIndex;
                    pPortal->NextPortal	         = NULL;
				    pPortal->LeafCount++;
                    return pPortal;
                
                } // End if leaf found
                else 
                {
                    delete pPortal;
                    return NULL;
                
                } // End if leaf not found

			} // End if child leaf
            else 
            {
                // Pass down the front
				PortalList = ClipPortal(CurrentNode->Front, pPortal);
				return PortalList;

            } // End If child node
				
			break;

		case CLASSIFY_BEHIND:
			
            // Test the contents of the back child            
            if (CurrentNode->Back == BSP_SOLID_LEAF ) 
            {
                // Destroy the portal
                delete pPortal;
                return NULL;
            
            } // End if solid leaf
            else if (CurrentNode->Back < 0 ) 
            {
                // The back is a Leaf, determine which side of the node it fell
                LeafIndex   = abs(CurrentNode->Back + 1);
                OwnerPos    = ClassifyLeaf( LeafIndex, pPortal->OwnerNode );

                // Found the leaf below?
                if ( OwnerPos != NO_OWNER ) 
                {
                    // This is just returned straight away, it's in an empty leaf
                    pPortal->LeafOwner[OwnerPos] = LeafIndex;
                    pPortal->NextPortal	         = NULL;
                    pPortal->LeafCount++;
                    return pPortal;

                } // End if leaf found
                else 
                {
                    delete pPortal;
                    return NULL;

                } // End if leaf not found

            } // End if child leaf
            else 
            {
                // Pass down the back
                PortalList = ClipPortal(CurrentNode->Back, pPortal);
                return PortalList;

            } // End If child node

            break;

		case CLASSIFY_SPANNING:
			
            // Allocate new front fragment
            if (!(FrontSplit = CBSPTree::AllocBSPPortal())) throw BCERR_OUTOFMEMORY;
            
            // Allocate new back fragment
            if (!(BackSplit	 = CBSPTree::AllocBSPPortal())) { delete FrontSplit; throw BCERR_OUTOFMEMORY; }
			
            // Portal fragment is spanning the plane, so it must be split
            if (FAILED( pPortal->Split(*CurrentPlane, FrontSplit, BackSplit))) 
            {  
                delete FrontSplit; delete BackSplit; 
                throw BCERR_OUTOFMEMORY;
            } // End If
			
            // Delete the ORIGINAL portal fragment
			delete pPortal;
			pPortal = NULL;
	
			// There is another Front NODE ?
			if (CurrentNode->Front < 0 ) 
            {
                // The front is a Leaf, determine which side of the node it fell
                LeafIndex   = abs(CurrentNode->Front + 1);
                OwnerPos    = ClassifyLeaf(LeafIndex, FrontSplit->OwnerNode );

                // Found the leaf?
                if ( OwnerPos != NO_OWNER) 
                {
                    FrontSplit->LeafOwner[OwnerPos] = LeafIndex;
                    FrontSplit->NextPortal          = NULL;
				    FrontPortalList                 = FrontSplit;
                    FrontSplit->LeafCount++;
                
                } // End if leaf found
                else 
                {
                    delete FrontSplit;
                
                } // End If no leaf found
			
            } // End if child leaf
            else 
            {
				FrontPortalList = ClipPortal(CurrentNode->Front, FrontSplit);
			
            } // End If child node

            // There is another back NODE ?
            if ( CurrentNode->Back == BSP_SOLID_LEAF ) 
            {
                // We ended up in solid space
                delete BackSplit;
            
            } // End if solid leaf
            else if (CurrentNode->Back < 0 ) 
            {
                // The back is a Leaf, determine which side of the node it fell
                LeafIndex   = abs(CurrentNode->Back + 1);
                OwnerPos    = ClassifyLeaf(LeafIndex, BackSplit->OwnerNode );

                // Found the leaf?
                if ( OwnerPos != NO_OWNER) 
                {
                    BackSplit->LeafOwner[OwnerPos] = LeafIndex;
                    BackSplit->NextPortal          = NULL;
                    BackPortalList                 = BackSplit;
                    BackSplit->LeafCount++;

                } // End if leaf found
                else 
                {
                    delete BackSplit;

                } // End If no leaf found

            } // End if child leaf
            else 
            {
                BackPortalList = ClipPortal(CurrentNode->Back, BackSplit);

            } // End If child node
			
            
            // Find the End of the front list and attach it to Back List
			if (FrontPortalList != NULL) 
            {
				// There is something in the front list
				Iterator = FrontPortalList;
				while (Iterator->NextPortal != NULL) Iterator = Iterator->NextPortal;
				if (BackPortalList != NULL) Iterator->NextPortal = BackPortalList;
                return FrontPortalList;
			
            } // End if front list
            else 
            {
				// There is nothing in the front list simply return the back list
                if (BackPortalList != NULL) return BackPortalList;
                return NULL;

			} // End if no front list
			
			// If we got here, we are fresh out of portal fragments so simply return NULL.
			return NULL;

    } // End switch

    return NULL;
}

//-----------------------------------------------------------------------------
// Name : AddPortals () (Private)
// Desc : Iterators through all the fragments passed. If the fragments are
//        valid portals, then they are added to the final portal list.
//-----------------------------------------------------------------------------
HRESULT CProcessPRT::AddPortals( CBSPPortal * PortalList )
{
    unsigned long PortalIndex = 0;
    CBSPPortal  * Iterator;
    CBSPLeaf * Leaf = NULL;

    // Validate 
    if (!PortalList) return BCERR_INVALIDPARAMS;

    // Iterate through the list, obtaining valid portals
    Iterator = PortalList;
    while ( Iterator != NULL ) 
    {
        // Store new portal index
        PortalIndex = m_pTree->GetPortalCount();

        // Add this portal to each leaf
        for ( int i = 0; i < 2; i++ ) 
        {
            Leaf = m_pTree->GetLeaf( Iterator->LeafOwner[i] );
            if (!Leaf) return BCERR_BSP_INVALIDTREEDATA;
            Leaf->AddPortal( PortalIndex );

        } // Next Leaf

        // We are adding a new portal
        m_pTree->IncreasePortalCount();

        // Set the portal
        m_pTree->SetPortal( PortalIndex, Iterator );

        // Move onto next portal
        Iterator = Iterator->NextPortal;

    } // End While

    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : FindLeaf () (Recursive) (Private)
// Desc : Determines if the leaf passed is somewhere below the specified node.
//-----------------------------------------------------------------------------
bool CProcessPRT::FindLeaf( unsigned long Leaf, unsigned long Node )
{
    CBSPNode * CurrentNode = NULL;

    // Validate Requirements
    if (!m_pTree) throw BCERR_INVALIDPARAMS;
    if (!(CurrentNode = m_pTree->GetNode( Node ))) throw BCERR_BSP_INVALIDTREEDATA;

    // Check to see if the front is this leaf
    if ( CurrentNode->Front < 0 ) 
    {
        if ( abs(CurrentNode->Front + 1) == Leaf ) return true;
    } 
    else 
    { 
        if (FindLeaf( Leaf, CurrentNode->Front )) return true;
    } // End If

    // Iterate down the back if it's a node
    if ( CurrentNode->Back >= 0 ) 
    {
        if (FindLeaf( Leaf, CurrentNode->Back )) return true;
    } // End If

    return false;
}

//-----------------------------------------------------------------------------
// Name : ClassifyLeaf () (Private)
// Desc : Classifies a leaf against this node (for ClipPortal). Leaf must be
//        somewhere relative to the node passed.
//-----------------------------------------------------------------------------
unsigned long CProcessPRT::ClassifyLeaf( unsigned long Leaf, unsigned long Node )
{
    CBSPNode * CurrentNode = NULL;

    // Validate Requirements
    if (!m_pTree) throw BCERR_INVALIDPARAMS;
    if (!(CurrentNode = m_pTree->GetNode( Node ))) throw BCERR_BSP_INVALIDTREEDATA;

    // Check to see if the front is this leaf
    if ( CurrentNode->Front < 0 ) 
    {
        if ( abs(CurrentNode->Front + 1) == Leaf ) return FRONT_OWNER;
    } 
    else 
    { 
        if (FindLeaf( Leaf, CurrentNode->Front )) return FRONT_OWNER;
    } // End If

    if ( CurrentNode->Back >= 0 ) 
    {
        if (FindLeaf(Leaf, CurrentNode->Back )) return BACK_OWNER;
    } // End If

    return NO_OWNER;
}
