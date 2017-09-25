//-----------------------------------------------------------------------------
// File: ProcessTJR.cpp
//
// Desc: This source file houses the primary T-Junction repair processor.
//       This object can be spawned to repair all T-Junctions within a set
//       of polygons passed to the processor.

//-----------------------------------------------------------------------------
// CProcessTJR Specific Includes
//-----------------------------------------------------------------------------
#include "ProcessTJR.h"
#include "CCompiler.h"

//-----------------------------------------------------------------------------
// Name : CProcessTJR () (Constructor)
// Desc : CProcessTJR Class Constructor
//-----------------------------------------------------------------------------
CProcessTJR::CProcessTJR()
{
    // Reset / Clear all required values
    m_pParent     = NULL;
    m_pLogger     = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CProcessTJR () (Destructor)
// Desc : CProcessTJR Class Destructor
//-----------------------------------------------------------------------------
CProcessTJR::~CProcessTJR()
{
    // Clean up after ourselves
}

//-----------------------------------------------------------------------------
// Name : Process ()
// Desc : Begins the processing operation which will repair all T-Juncs
//-----------------------------------------------------------------------------
HRESULT CProcessTJR::Process( CPolygon ** ppPolys, ULONG PolyCount )
{
    ULONG      i, k;
    CBounds3  *pBounds = NULL;
    CPolygon  *pCurrentPoly, *pTestPoly;

    // Validate values
    if (!ppPolys || !PolyCount) return BCERR_INVALIDPARAMS;

    try
    {

        // Note : Pre-Compiled adjacency information would increase the speed of this process even,
        //        further. However, per-poly bounding box intersection tests (shown here) offer VAST
        //        speed increases with a relatively small amount of additional code complexity. The bounds
        //        are pre-calculated here (at the slight expense of additional memory) to prevent the
        //        bounding boxes having to be recalculated literally millions of times within the inner
        //        loop on medium sized levels (10k polygons+)
        if (!(pBounds = new CBounds3[PolyCount])) return BCERR_OUTOFMEMORY;

        // *************************
        // * Write Log Information *
        // *************************
        if ( m_pLogger )
        {
            m_pLogger->LogWrite( LOG_TJR, 0, true, _T("Pre-compiling adjacency information \t\t- " ) );
            m_pLogger->SetRewindMarker( LOG_TJR );
            m_pLogger->LogWrite( LOG_TJR, 0, false, _T("0%%" ) );
            m_pLogger->SetProgressRange( PolyCount );
            m_pLogger->SetProgressValue( 0 );
        } 
        // *************************
        // *    End of Logging     *
        // *************************

        // Calculate polygon bounds
        for ( i = 0; i < PolyCount; i++ ) 
        {
            // Update Progress
            if (!m_pParent->TestCompilerState()) throw BC_CANCELLED;
            if ( m_pLogger ) m_pLogger->UpdateProgress();

            // Build polygon bounds
            pBounds[i].CalculateFromPolygon( ppPolys[ i ]->Vertices, ppPolys[ i ]->VertexCount, sizeof(CVertex) );

            // Increase bounds slightly to relieve this operation during intersection testing
            pBounds[i].Min.x -= 0.1f; pBounds[i].Min.y -= 0.1f; pBounds[i].Min.z -= 0.1f;
            pBounds[i].Max.x += 0.1f; pBounds[i].Max.y += 0.1f; pBounds[i].Max.z += 0.1f;
        } // Next Bounds

        // *************************
        // * Write Log Information *
        // *************************
        if ( m_pLogger )
        {
            if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_TJR );
            m_pLogger->LogWrite( LOG_TJR, 0, true, _T("Repairing all encountered T-Junctions \t- " ) );
            m_pLogger->SetRewindMarker( LOG_TJR );
            m_pLogger->LogWrite( LOG_TJR, 0, false, _T("0%%" ) );
            m_pLogger->SetProgressRange( PolyCount );
            m_pLogger->SetProgressValue( 0 );
        } 
        // *************************
        // *    End of Logging     *
        // *************************

        // Loop through Faces
        for ( i = 0; i < PolyCount; i++ ) 
        {
            // Update Progress
            if (!m_pParent->TestCompilerState()) throw BC_CANCELLED;
            if ( m_pLogger ) m_pLogger->UpdateProgress();

            // Get the current poly to test and its vertex array
            pCurrentPoly = ppPolys[ i ];
            if (!pCurrentPoly) continue;

            // Test against every other face in the tree
            for ( k = 0; k < PolyCount; k++ ) 
            {
                // Don't against test self
                if (i == k) continue;

                // Get the test face and its vertices
                pTestPoly = ppPolys[ k ];
                if (!pTestPoly) continue;

                // If the two do not intersect then there is no need for testing.
                if ( !pBounds[i].IntersectedByBounds( pBounds[k] ) ) continue;

                // Repair against the testing poly
                if (!(RepairTJunctions( pCurrentPoly, pTestPoly ))) throw BCERR_OUTOFMEMORY;

                // Now we do exactly the same but in reverse order
                if (!(RepairTJunctions( pTestPoly, pCurrentPoly ))) throw BCERR_OUTOFMEMORY;
                
            } // Next Test Face

            // This outer poly has now been entirely repaired against all faces
            // It need not be processed again inside the inner loop.
            ppPolys[i] = NULL;

        } // Next Current Face

    } // End Try Block

    catch ( HRESULT &e )
    {
        // Clean up and return (failure)
        if (pBounds) delete []pBounds;
        if ( m_pLogger && FAILED(e) ) m_pLogger->ProgressFailure( LOG_TJR );
        return e;
    
    } // End Catch Block

    // Release used memory
    if (pBounds) delete []pBounds;

    // Success!
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_TJR );
    return BC_OK;
 
}

//-----------------------------------------------------------------------------
// Name : RepairTJunctions ()
// Desc : Tests and repairs one polygon against another.
//-----------------------------------------------------------------------------
bool CProcessTJR::RepairTJunctions( CPolygon *pPoly1, CPolygon *pPoly2 ) const
{
    CVector3   Delta;
    float       Percent;
    ULONG      v1, v2, v1a;
    CVertex    Vert1, Vert2, Vert1a;
    

    // Validate Parameters
    if (!pPoly1 || !pPoly2) return false;

    // For each edge of this face
    for ( v1 = 0; v1 < pPoly1->VertexCount; v1++ )
    {
        // Retrieve the next edge vertex (wraps to 0)
        v2 = ((v1 + 1) % pPoly1->VertexCount);

        // Store verts (Required because indices may change)
        Vert1 = pPoly1->Vertices[v1];
        Vert2 = pPoly1->Vertices[v2];

        // Now loop through each vertex in the test face
        for ( v1a = 0; v1a < pPoly2->VertexCount; v1a++ ) 
        {
            // Store test point for easy access
            Vert1a = pPoly2->Vertices[v1a];

            // Test if this vertex is close to the test edge
            // (Also returns out of range value if the point is past the line ends)
            if ( Vert1a.DistanceToLine( Vert1, Vert2 ) < EPSILON )
            {
                // Insert a new vertex within this edge
                long NewVert = pPoly1->InsertVertex( v2 );
                if (NewVert < 0) return false;

                // Set the vertex pos
                CVertex * pNewVert = &pPoly1->Vertices[ NewVert ];
                pNewVert->x = Vert1a.x; pNewVert->y = Vert1a.y; pNewVert->z = Vert1a.z;

                // Calculate the percentage for interpolation calcs
                Percent = (*pNewVert - Vert1).Length() / (Vert2 - Vert1).Length();

                // Interpolate texture coordinates
                Delta.x      = Vert2.tu - Vert1.tu;
			    Delta.y      = Vert2.tv - Vert1.tv;
			    pNewVert->tu = Vert1.tu + ( Delta.x * Percent );
			    pNewVert->tv = Vert1.tv + ( Delta.y * Percent );

                // Interpolate normal
                Delta            = Vert2.Normal - Vert1.Normal;
                pNewVert->Normal = Vert1.Normal + (Delta * Percent);
                pNewVert->Normal.Normalize();    

                // Update the edge for which we are testing
                Vert2 = *pNewVert;

            } // End if on edge

        } // Next Vertex v1a

    } // Next Vertex  v1

    // Success!
    return true;
}