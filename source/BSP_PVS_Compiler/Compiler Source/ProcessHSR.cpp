//-----------------------------------------------------------------------------
// File: ProcessHSR.cpp
//
// Desc: This source file houses the primary hidden surface removal processor.
//       This object can be spawned to remove all hidden surfaces from a set
//       of brushes, at which point a single mesh, or multiple meshes, can be
//       returned ready for further processing.
//

//-----------------------------------------------------------------------------
// CProcessHSR Specific Includes
//-----------------------------------------------------------------------------
#include "ProcessHSR.h"
#include "CCompiler.h"
#include "CBSPTree.h"

//-----------------------------------------------------------------------------
// Name : CProcessHSR () (Constructor)
// Desc : CProcessHSR Class Constructor
//-----------------------------------------------------------------------------
CProcessHSR::CProcessHSR()
{
    // Reset / Clear all required values
    m_pResultMesh = NULL;
    m_pLogger     = NULL;
    m_pParent     = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CProcessHSR () (Destructor)
// Desc : CProcessHSR Class Destructor
//-----------------------------------------------------------------------------
CProcessHSR::~CProcessHSR()
{
    // Clean up after ourselves
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Cleans up the HSR processor ready for another batch of meshes.
//-----------------------------------------------------------------------------
void CProcessHSR::Release()
{
    // Just clear our mesh list (we do not own these pointers)
    m_vpMeshList.clear();

    // Reset our result mesh (we also do not own this pointer)
    m_pResultMesh = NULL;
}

//-----------------------------------------------------------------------------
// Name : AddMesh ()
// Desc : Adds a mesh ready for processing.
// Note : The meshes being passed will not be modified, they are only
//        used to retrieve the data needed for the HSR pass itself.
//-----------------------------------------------------------------------------
bool CProcessHSR::AddMesh( CMesh * pMesh )
{
    try 
    {
        // Resize the vector if we need to
        if (m_vpMeshList.size() >= (m_vpMeshList.capacity() - 1)) 
        {
            m_vpMeshList.reserve( m_vpMeshList.size() + HSR_ARRAY_THRESHOLD );

        } // End If

        // Finally add this mesh pointer to the list
        m_vpMeshList.push_back( pMesh );
    
    } // End Try Block

    catch (...) 
    { 
        // Clean up and bail
        m_vpMeshList.clear(); 
        return false; 

    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : Process ()
// Desc : Begins the processing operation which will remove all hidden surfaces
//-----------------------------------------------------------------------------
HRESULT CProcessHSR::Process()
{
    HRESULT     hRet;
    BSPOPTIONS  Options;
    int         i, a, b;
    CBSPTree ** BSPTrees  = NULL;
    long        MeshCount = m_vpMeshList.size();
    CBounds3    BoundsA, BoundsB;

    // Build the option set which will be used for all Mini-BSP Trees
    ZeroMemory( &Options, sizeof(BSPOPTIONS));
    Options.TreeType          = BSP_TYPE_NONSPLIT;
    Options.Enabled           = true;
    Options.RemoveBackLeaves  = true;
    Options.SplitHeuristic    = 3.0f;
    Options.SplitterSample    = 60;
    
    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger )
    {
        m_pLogger->LogWrite( LOG_HSR, 0, true, _T("Building mesh solid area information \t\t- " ) );
        m_pLogger->SetRewindMarker( LOG_HSR );
        m_pLogger->LogWrite( LOG_HSR, 0, false, _T("0%%" ) );
        m_pLogger->SetProgressRange( MeshCount );
        m_pLogger->SetProgressValue( 0 );
    } 
    // *************************
    // *    End of Logging     *
    // *************************

    try
    {
        // Allocate memory for an array pf BSP Trees
        BSPTrees = new CBSPTree*[ MeshCount ];
        if (!BSPTrees) throw BCERR_OUTOFMEMORY;

        // Set all pointers to NULL
        ZeroMemory( BSPTrees, MeshCount * sizeof(CBSPTree*));

        // Allocate our result mesh
        m_pResultMesh = new CMesh;
        if (!m_pResultMesh) throw BCERR_OUTOFMEMORY;

        // Build a BSP Tree for each mesh.
        for ( i = 0; i < MeshCount; i++ )
        {
            // Allocate a new tree
            BSPTrees[i] = new CBSPTree;
            if (!BSPTrees[i]) throw BCERR_OUTOFMEMORY;
            BSPTrees[i]->SetOptions( Options );

            // Add the faces from this mesh ready for compile
            hRet = BSPTrees[i]->AddFaces( m_vpMeshList[i]->Faces, m_vpMeshList[i]->FaceCount );
            if (FAILED(hRet)) throw hRet;

            // Compile the BSP Tree
            hRet = BSPTrees[i]->CompileTree();
            if (FAILED(hRet)) throw hRet;

            // Update Progress
            if ( !m_pParent->TestCompilerState() ) break;
            if ( m_pLogger ) m_pLogger->UpdateProgress( );

        } // Next Mesh

        // If we're cancelled, throw cancel error (success code) (auto-clean up)
        if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) throw BC_CANCELLED;

        // *************************
        // * Write Log Information *
        // *************************
        if ( m_pLogger )
        {
            m_pLogger->ProgressSuccess( LOG_HSR );

            m_pLogger->LogWrite( LOG_HSR, 0, true, _T("Clipping meshes for surface removal \t\t- " ) );
            m_pLogger->SetRewindMarker( LOG_HSR );
            m_pLogger->LogWrite( LOG_HSR, 0, false, _T("0%%" ) );
            m_pLogger->SetProgressRange( MeshCount );
            m_pLogger->SetProgressValue( 0 );
        } 
        // *************************
        // *    End of Logging     *
        // *************************

        // Now do the actual Union (HSR) clipping operations
        for ( a = 0; a < MeshCount; a++ )
        {
            // Update progress
            if ( !m_pParent->TestCompilerState() ) break;
            if ( m_pLogger ) m_pLogger->UpdateProgress( );

            // Skip any NULL bsp objects
	        if (!BSPTrees[a] || BSPTrees[a]->GetFaceCount() == 0) continue;	 

            // Clip against all other meshes
            for ( b = 0; b < MeshCount; b++ )
            {
                // Skip any NULL bsp objects (i.e. has already been clipped)
	            if (!BSPTrees[b] || BSPTrees[b]->GetFaceCount() == 0) continue;	 
		     
                // Don't Boolean Op the mesh with itself
                if (a == b) continue;
            
                // Skip if the two don't intersect (Bounds test requires a small tolerance)
                BoundsA = BSPTrees[a]->GetBounds(); BoundsB = BSPTrees[b]->GetBounds();
                if ( !BoundsA.IntersectedByBounds( BoundsB, CVector3( 0.1f, 0.1f, 0.1f ) ) ) continue;
                if ( !BSPTrees[a]->IntersectedByTree( BSPTrees[b] ) ) continue;
			    
                // TODO : Needs to handle failed return values
                // Clip tree a to tree b and vice versa
                BSPTrees[a]->ClipTree( BSPTrees[b], true, false );
                BSPTrees[b]->ClipTree( BSPTrees[a], true, true  );
					
                // Repair Unrequired Splits
	 			BSPTrees[a]->RepairSplits();
				BSPTrees[b]->RepairSplits();

            } // Next Mesh b

            // Append all of the faces from our fully clipped tree [a] to our result brush. 
	        m_pResultMesh->BuildFromBSPTree( BSPTrees[a], false );
	        
            // Tree a has now been clipped by all other meshes
            delete BSPTrees[a]; 
            BSPTrees[a] = NULL;

        } // Next Mesh a

        // If we're cancelled, throw cancel error (success code) (auto-clean up)
        if ( m_pParent->GetCompileStatus() == CS_CANCELLED ) throw BC_CANCELLED;

    } // End Try Block

    catch ( HRESULT &e )
    {
        if ( BSPTrees )
        {
            for ( i = 0; i < MeshCount; i++ ) if ( BSPTrees[i] ) delete BSPTrees[i];
            delete []BSPTrees;

        } // End if we allocated

        // Release the result mesh
        if ( m_pResultMesh ) { delete m_pResultMesh; m_pResultMesh = NULL; }
     
        // Failure??
        if ( FAILED(e) && m_pLogger ) m_pLogger->ProgressFailure( LOG_HSR );

        return e;
    
    } // End Catch Block

    // Release the BSP Trees
    if ( BSPTrees )
    {
        for ( i = 0; i < MeshCount; i++ ) if ( BSPTrees[i] ) delete BSPTrees[i];
        delete []BSPTrees;
    
    } // End if we allocated

    // Success!
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_HSR );
    return BC_OK;
 
}

//-----------------------------------------------------------------------------
// Name : GetResultMesh ()
// Desc : Retrieves the resulting mesh build during the HSR process.
//-----------------------------------------------------------------------------
CMesh * CProcessHSR::GetResultMesh() const
{
    return m_pResultMesh;
}