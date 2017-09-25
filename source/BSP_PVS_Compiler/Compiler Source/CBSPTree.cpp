#include "CBSPTree.h"

//-----------------------------------------------------------------------------
// File: CBSPTree.cpp
//
// Desc: The BSP Tree class is responsible for building and processing the tree
//       itself. It is also resposible for the tree based clipping utilised
//       during CSG operations.
//

//-----------------------------------------------------------------------------
// CBSPTree Specific Includes
//-----------------------------------------------------------------------------
#include <algorithm>
#include <stack>

#include "CBSPTree.h"
#include "CCompiler.h"
#include "..\\Support Source\\CPlane.h"
#include "..\\Support Source\\CVector.h"
#include "..\\Support Source\\CCollision.h"


//-----------------------------------------------------------------------------
// Desc : CBSPFace member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPFace() (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CBSPFace::CBSPFace()
{
    // Initialise anything we need
    UsedAsSplitter  = false; 
    OriginalIndex   = -1;
    Next            = NULL;
    Deleted         = false;
    ChildSplit[0]   = -1;
    ChildSplit[1]   = -1;
    Plane           = -1;

}

//-----------------------------------------------------------------------------
// Name : CBSPFace() (Alternate Constructor)
// Desc : Constructor for this class. Duplicates the specified face.
//-----------------------------------------------------------------------------
CBSPFace::CBSPFace( const CFace * pFace )
{
    // Initialise anything we need
    UsedAsSplitter  = false; 
    OriginalIndex   = -1;
    Next            = NULL;
    Deleted         = false;
    ChildSplit[0]   = -1;
    ChildSplit[1]   = -1;
    Plane           = -1;

    // Duplicate required values
    Normal          = pFace->Normal;
    TextureIndex    = pFace->TextureIndex;
    MaterialIndex   = pFace->MaterialIndex;
    ShaderIndex     = pFace->ShaderIndex;
    Flags           = pFace->Flags;
    SrcBlendMode    = pFace->SrcBlendMode;
    DestBlendMode   = pFace->DestBlendMode;

    // Duplicate Arrays
    AddVertices( pFace->VertexCount );
    memcpy( Vertices, pFace->Vertices, VertexCount * sizeof(CVertex));
}

bool CBSPFace::RayIntersect(const XMFLOAT3 &rayOrigin, const XMFLOAT3 &rayDir)
{
	if (VertexCount < 3) return false;
	int i0 = 0;
	XMFLOAT3 triNormal = { Normal.x, Normal.y, Normal.z };
	XMFLOAT3 v0 = { Vertices[i0].x, Vertices[i0].y, Vertices[i0].z };
	
	for (int i = 2; i != VertexCount; ++i) {
		int i1 = i - 1;
		int i2 = i;
		
		XMFLOAT3 v1 = { Vertices[i1].x, Vertices[i1].y, Vertices[i1].z };
		XMFLOAT3 v2 = { Vertices[i2].x, Vertices[i2].y, Vertices[i2].z };
		
		if (RayIntersectTriangle(rayOrigin, rayDir, v0, v1, v2, triNormal, true)) {
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Name : Split ()
// Desc : This function splits the current BSPFace, against the plane. The two
//        split fragments are returned via the FrontSplit and BackSplit
//        pointers. These must be valid pointers to already allocated faces.
//        However you CAN pass NULL to either parameter, in this case no
//        vertices will be calculated for that fragment.
//-----------------------------------------------------------------------------
HRESULT CBSPFace::Split( const CPlane3& SplitPlane, CBSPFace * FrontSplit, CBSPFace * BackSplit, bool bReturnNoSplit /* = false */ )
{

    // Call base class implementation
    HRESULT ErrCode = CFace::Split( SplitPlane, FrontSplit, BackSplit, bReturnNoSplit );
    if (FAILED(ErrCode)) return ErrCode;

    // Copy remaining values
    if (FrontSplit) 
    {
        FrontSplit->UsedAsSplitter = UsedAsSplitter;
        FrontSplit->OriginalIndex  = OriginalIndex;
        FrontSplit->Plane          = Plane;

    } // End If

    if (BackSplit) 
    {
        BackSplit->UsedAsSplitter = UsedAsSplitter;
        BackSplit->OriginalIndex  = OriginalIndex;
        BackSplit->Plane          = Plane;

    } // End If

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Desc : CBSPLeaf member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPLeaf() (CONSTRUCTOR)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CBSPLeaf::CBSPLeaf()
{
	// Initialise anything we need
    PVSIndex        = -1;
}

//-----------------------------------------------------------------------------
// Name : ~CBSPLeaf() (DESTRUCTOR)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
CBSPLeaf::~CBSPLeaf()
{
	// Clean up after ourselves
}

//-----------------------------------------------------------------------------
// Name : BuildFaceIndices ()
// Desc : Takes a linked list of faces, and builds the indices for this leaf
// Note : This function makes sure that the list only contains ONE of each
//        Original polygon, this should never really happen under normal
//        circumstances, but is a good thing to do none the less.
//-----------------------------------------------------------------------------
bool CBSPLeaf::BuildFaceIndices( CBSPFace * pFaceList )
{

    CBSPFace       *Iterator      = pFaceList;
	unsigned long   OriginalIndex = 0;

    // Reset bounds, will be rebuilt
    Bounds.Reset();

	// Iterate building list
	while ( Iterator != NULL ) {
		
        // Get the original polygons index
        OriginalIndex = Iterator->OriginalIndex;
	
        try 
        {
            // Make sure this index does not already exist in the array?
		    if (std::find(FaceIndices.begin(), FaceIndices.end(), OriginalIndex) == FaceIndices.end()) {
            
                // Resize the vector if we need to
                if (FaceIndices.size() >= (FaceIndices.capacity() - 1)) 
                {
                    FaceIndices.reserve( FaceIndices.size() + BSP_ARRAY_THRESHOLD );
                } // End If

                // Finally add this face index to the list
                FaceIndices.push_back(OriginalIndex);

		    } // End If needs adding
        
        } // End Try Block

        catch (...) { 
            // Clean up and bail
            FaceIndices.clear(); 
            return false; 
        } // End Catch

        // Build into current bounding box
		//if (Iterator->Flags == FACE_WORLD) {
			Bounds.CalculateFromPolygon(Iterator->Vertices, Iterator->VertexCount, sizeof(CVertex), false);
		//}

        // Move to next poly
        Iterator = Iterator->Next;

	} // End While

    // Optimize the vector
    if ( FaceIndices.size() < FaceIndices.capacity()) FaceIndices.resize( FaceIndices.size() );
	
    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : AddPortal ()
// Desc : Adds the specified portal index to this leaf
//-----------------------------------------------------------------------------
bool CBSPLeaf::AddPortal( unsigned long PortalIndex )
{
    try 
    {
        // Resize the vector if we need to
        if (PortalIndices.size() >= (PortalIndices.capacity() - 1)) 
        {
            PortalIndices.reserve( PortalIndices.size() + BSP_ARRAY_THRESHOLD );
        } // End If

        // Finally add this face index to the list
        PortalIndices.push_back( PortalIndex );
    
    } // End Try Block

    catch (...) 
    { 
        // Clean up and bail
        PortalIndices.clear(); 
        return false; 
    } // End Catch

    // Success
    return true;
}

bool CBSPLeaf::RayIntersection(CBSPTree *pTree, const XMFLOAT3 &rayOrigin, const XMFLOAT3 &rayDir)
{
	size_t numFaces = FaceIndices.size();
	for (size_t i = 0; i != numFaces; ++i) {
		CBSPFace *pFace = pTree->GetFace(FaceIndices[i]);
		if (!(pFace->Flags & FACE_DETAIL)) continue;
		if (pFace->RayIntersect(rayOrigin, rayDir)) {
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Desc : CBSPNode member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPNode() (CONSTRUCTOR)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CBSPNode::CBSPNode()
{
	// Initialise anything we need
    Plane   = -1;
    Front   = -1;
    Back    = -1;
}

//-----------------------------------------------------------------------------
// Name : CalculateBounds ()
// Desc : Calculates a bounding box from the list of faces passed
//-----------------------------------------------------------------------------
void CBSPNode::CalculateBounds( CBSPFace * pFaceList, bool ResetBounds )
{
    CBSPFace *Iterator = pFaceList;
    
    // Reset bounding box and grow from default state?
    if ( ResetBounds ) Bounds.Reset();

    // Calculate bounding box based on the face list passed
    while ( Iterator ) 
    {
        // Grow the bounding box values
		if (Iterator->Flags == FACE_WORLD) {
			Bounds.CalculateFromPolygon(Iterator->Vertices, Iterator->VertexCount,
				sizeof(CVertex), false);
		}

        // Move on to the next polygon
        Iterator = Iterator->Next;
    } // End If
}

//-----------------------------------------------------------------------------
// Desc : CBSPPortal member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPPortal () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CBSPPortal::CBSPPortal()
{
	// Initialise any class specific items
    LeafOwner[0]    = NO_OWNER;
    LeafOwner[1]    = NO_OWNER;
    NextPortal      = NULL;
    LeafCount       = 0;
    OwnerNode       = -1;
}

//-----------------------------------------------------------------------------
// Name : Split ()
// Desc : This function splits the current portal, against the plane. The two
//        split fragments are returned via the FrontSplit and BackSplit
//        pointers. These must be valid pointers to already allocated portals.
//        However you CAN pass NULL to either parameter, in this case no
//        vertices will be calculated for that fragment.
//-----------------------------------------------------------------------------
HRESULT CBSPPortal::Split( const CPlane3& Plane, CBSPPortal * FrontSplit, CBSPPortal * BackSplit)
{
    // Call base class implementation
    HRESULT ErrCode = CPolygon::Split( Plane, FrontSplit, BackSplit );
    if (FAILED(ErrCode)) return ErrCode;

    // Copy remaining values
    if (FrontSplit) 
    {
        FrontSplit->LeafCount    = LeafCount;
        FrontSplit->OwnerNode    = OwnerNode;
        FrontSplit->LeafOwner[0] = LeafOwner[0];
        FrontSplit->LeafOwner[1] = LeafOwner[1];

    } // End If

    if (BackSplit) 
    {
        BackSplit->LeafCount     = LeafCount;
        BackSplit->OwnerNode     = OwnerNode;
        BackSplit->LeafOwner[0]  = LeafOwner[0];
        BackSplit->LeafOwner[1]  = LeafOwner[1];
    } // End If

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Desc : CBSPTree member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPTree () (Constructor)
// Desc : CBSPTree Class Constructor
//-----------------------------------------------------------------------------
CBSPTree::CBSPTree()
{
    // Reset / Clear all required values
    m_pFaceList         = NULL;
    m_pLogger           = NULL;
    m_lActiveFaces      = 0;
    m_pPVSData          = NULL;
    m_lPVSDataSize      = 0;
    m_bPVSCompressed    = false;
    m_pParent           = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CBSPTree () (Destructor)
// Desc : CBSPTree Class Destructor
//-----------------------------------------------------------------------------
CBSPTree::~CBSPTree()
{
    // Clean up after ourselves
    ReleaseTree();
}

//-----------------------------------------------------------------------------
// Name : ReleaseTree ()
// Desc : Releases all memory allocated by the BSP Tree
//-----------------------------------------------------------------------------
void CBSPTree::ReleaseTree()
{
    unsigned long i;

    // Delete Objects inside Vector Arrays
    for ( i = 0; i < GetFaceCount()  ; i++ ) if ( GetFace(i)   ) delete GetFace(i);
    for ( i = 0; i < GetNodeCount()  ; i++ ) if ( GetNode(i)   ) delete GetNode(i);
    for ( i = 0; i < GetLeafCount()  ; i++ ) if ( GetLeaf(i)   ) delete GetLeaf(i);
    for ( i = 0; i < GetPlaneCount() ; i++ ) if ( GetPlane(i)  ) delete GetPlane(i);
    for ( i = 0; i < GetPortalCount(); i++ ) if ( GetPortal(i) ) delete GetPortal(i);
    
    // Free Garbage Collection faces
    for ( i = 0; i < m_vpGarbage.size(); i++ ) if ( m_vpGarbage[i] ) delete m_vpGarbage[i];

    // Clear Vectors
    m_vpNodes.clear();
    m_vpPlanes.clear();
    m_vpLeaves.clear();
    m_vpFaces.clear();
    m_vpPortals.clear();
    m_vpGarbage.clear();
    
    // Clear variables
    m_pPVSData      = NULL;
    m_pFaceList     = NULL;
    m_lActiveFaces  = 0;
    m_lPVSDataSize  = 0;
}

//-----------------------------------------------------------------------------
// Name : CompileTree ()
// Desc : Takes the faces that were added to the BSP Tree, and compiles
//-----------------------------------------------------------------------------
HRESULT CBSPTree::CompileTree( )
{
    HRESULT ErrCode;

    // Validate values
    if (!m_pFaceList) return BCERR_INVALIDPARAMS;

    // Calculate our plane array
    if (FAILED(ErrCode = BuildPlaneArray())) return ErrCode;

    // Allocate our root node
    if (!IncreaseNodeCount()) return BCERR_OUTOFMEMORY;

    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger )
    {
        m_pLogger->LogWrite( LOG_BSP, 0, true, _T("Building binary space partition tree \t\t- " ));
        m_pLogger->SetRewindMarker( LOG_BSP );
        m_pLogger->LogWrite( LOG_BSP, LOGF_ITALIC | LOGF_WARNING, false, _T("Please Wait..." ) );
        m_pLogger->SetProgressRange( m_lActiveFaces );
        m_pLogger->SetProgressValue( 0 );
    } 
    // *************************
    // *    End of Logging     *
    // *************************
    
    // Compile the BSP Tree
    if (FAILED(ErrCode = BuildBSPTree(0, m_pFaceList))) return ErrCode;

    // Did we cancel ?
    if ( m_pParent && m_pParent->GetCompileStatus() == CS_CANCELLED )
    {
        // Trash any remaining faces
        TrashFaceList( m_pFaceList );
        return BC_CANCELLED;
    
    } // End if

    // Reset Face List
    m_pFaceList = NULL;

    // Build the trees bounding box
    m_Bounds.Reset();
    for ( ULONG i = 0; i < GetFaceCount(); i++ )
    {
		if (GetFace(i)->Flags != FACE_WORLD) continue;
        m_Bounds.CalculateFromPolygon( GetFace(i)->Vertices, GetFace(i)->VertexCount, sizeof(CVertex), false );

    } // Next Face

    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_BSP );
    // *************************
    // *    End of Logging     *
    // *************************

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : AddFaces ()
// Desc : Takes the faces passed in and adds them to the face list
//-----------------------------------------------------------------------------
HRESULT CBSPTree::AddFaces( CFace ** ppFaces, unsigned long lFaceCount )
{
    CBSPFace *NewFace = NULL;

    // Validate Parameters
    if ( !ppFaces || lFaceCount <= 0 ) return BCERR_INVALIDPARAMS;

    // Release the old tree data (if any) should this be our first set of faces
    if (!m_pFaceList) ReleaseTree();

    // First add the mesh polygons to the initial linked list
    for ( unsigned long i = 0; i < lFaceCount; i++ ) 
    {
        
        // Add if available
        if (ppFaces[i]) 
		{
            // Allocate the new BSPFace
            if (!(NewFace = AllocBSPFace( ppFaces[i] ))) 
            {
                ReleaseTree(); 
                return BCERR_OUTOFMEMORY; 
            } // End If Out of Memory

            // Store original index for later use
            NewFace->OriginalIndex = m_lActiveFaces;
            m_lActiveFaces++;

            // Attach this to the list
            NewFace->Next = m_pFaceList;
            m_pFaceList   = NewFace;
            
            // Now we add the original face into the BSP Trees
            // RESULT face list (if requested) when compiling
            // a non-split resulting BSP Tree. In this case the
            // Indices stored in the leaves will point to the
            // relevant location in the m_vpFaces array.    
            if ( m_OptionSet.TreeType == BSP_TYPE_NONSPLIT )
            {
                // Allocate some storage space
                if (!IncreaseFaceCount()) { ReleaseTree(); return BCERR_OUTOFMEMORY; }
                
                // Allocate a duplicate of the original original
                if (!(NewFace = AllocBSPFace( ppFaces[i] ))) { ReleaseTree(); return BCERR_OUTOFMEMORY; }

                // Store this new pointer
                SetFace( GetFaceCount() - 1, NewFace );

            } // End if backup required

        } // End If Face Available

    } // Next Face

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : AddBoundingPolys ()
// Desc : This function adds 6, inward facing, polygons which bound the level
// Note : Can only be called prior to compilation, it simply adds these 6 polys
//        onto the linked list to be considered for compilation.
//-----------------------------------------------------------------------------
HRESULT CBSPTree::AddBoundingPolys( bool BackupOriginals )
{
    CBSPFace *pIterator;
    CBounds3  Bounds;
    ULONG     i, j;

    // Cannot backup originals in a split type tree
    if ( m_OptionSet.TreeType == BSP_TYPE_SPLIT ) BackupOriginals = false;

    // Build a bounding box from the data ready for compilation
    Bounds.Reset();
    for ( pIterator = m_pFaceList; pIterator; pIterator = pIterator->Next )
    {
        Bounds.CalculateFromPolygon( pIterator->Vertices, pIterator->VertexCount, sizeof(CVertex), false );

    } // Next Face

    // Increase the size of the bounds a little to ensure no intersection occurs.
    Bounds.Max += CVector3( 10.0f, 10.0f, 10.0f );
    Bounds.Min -= CVector3( 10.0f, 10.0f, 10.0f );

    CVector3 Size   = Bounds.GetDimensions() / 2.0f;
    CVector3 Centre = Bounds.GetCentre();

    // Generate 6 bounding polys
    for ( i = 0; i < 6; i++ )
    {
        CBSPFace * pFace = AllocBSPFace();
        if (!pFace) return BCERR_OUTOFMEMORY;

        // Generate the face
        if ( pFace->AddVertices( 4 ) < 0 ) { delete pFace; return BCERR_OUTOFMEMORY; }

        // Store original index for later use
        pFace->OriginalIndex = m_lActiveFaces;
        m_lActiveFaces++;

        // Which face are we building?
        switch ( i )
        {
            case 0:
                // Top Face
                pFace->Vertices[ 0 ] = Centre + CVector3( -Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3(  Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3(  Size.x,  Size.y,  Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3( -Size.x,  Size.y,  Size.z );
                pFace->Normal = CVector3( 0.0f, -1.0f, 0.0f ); // Points Down
                break;

            case 1:
                // Bottom Face
                pFace->Vertices[ 0 ] = Centre + CVector3( -Size.x, -Size.y,  Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3(  Size.x, -Size.y,  Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3(  Size.x, -Size.y, -Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3( -Size.x, -Size.y, -Size.z );
                pFace->Normal = CVector3( 0.0f, 1.0f, 0.0f ); // Points Up
                break;

            case 2:
                // Left Face
                pFace->Vertices[ 0 ] = Centre + CVector3( -Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3( -Size.x,  Size.y,  Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3( -Size.x, -Size.y,  Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3( -Size.x, -Size.y, -Size.z );
                pFace->Normal = CVector3( 1.0f, 0.0f, 0.0f ); // Points Right
                break;

            case 3:
                // Right Face
                pFace->Vertices[ 0 ] = Centre + CVector3(  Size.x,  Size.y,  Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3(  Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3(  Size.x, -Size.y, -Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3(  Size.x, -Size.y,  Size.z );
                pFace->Normal = CVector3( -1.0f, 0.0f, 0.0f ); // Points Left
                break;

            case 4:
                // Front Face
                pFace->Vertices[ 0 ] = Centre + CVector3(  Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3( -Size.x,  Size.y, -Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3( -Size.x, -Size.y, -Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3(  Size.x, -Size.y, -Size.z );
                pFace->Normal = CVector3( 0.0f, 0.0f, 1.0f ); // Points Back
                break;

            case 5:
                // Back Face
                pFace->Vertices[ 0 ] = Centre + CVector3( -Size.x,  Size.y,  Size.z );
                pFace->Vertices[ 1 ] = Centre + CVector3(  Size.x,  Size.y,  Size.z );
                pFace->Vertices[ 2 ] = Centre + CVector3(  Size.x, -Size.y,  Size.z );
                pFace->Vertices[ 3 ] = Centre + CVector3( -Size.x, -Size.y,  Size.z );
                pFace->Normal = CVector3( 0.0f, 0.0f, -1.0f ); // Points Forward
                break;

        } // End Switch

        // Copy over vertex normal
        for ( j = 0; j < 4; j++ ) pFace->Vertices[ j ].Normal = pFace->Normal;
        
        // Add to head of linked list
        pFace->Next = m_pFaceList;
        m_pFaceList = pFace;

        // Now we add the original face into the BSP Trees
        // RESULT face list (if requested) when compiling
        // a non-split resulting BSP Tree. In this case the
        // Indices stored in the leaves will point to the
        // relevant location in the m_vpFaces array.    
        if (BackupOriginals) 
        {
            CBSPFace * NewFace = NULL;

            // Allocate some storage space
            if (!IncreaseFaceCount()) { ReleaseTree(); return BCERR_OUTOFMEMORY; }
            
            // Allocate a duplicate of the original original
            if (!(NewFace = AllocBSPFace( pFace ))) { ReleaseTree(); return BCERR_OUTOFMEMORY; }

            // Store this new pointer
            SetFace( GetFaceCount() - 1, NewFace );

        } // End if backup required

    } // Next Poly

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : BuildPlaneArray () (Private)
// Desc : Takes a list of all the faces stored, and builds the plane array from
//        them, combining wherever possible.
//-----------------------------------------------------------------------------
HRESULT CBSPTree::BuildPlaneArray()
{
    CBSPFace * Iterator = NULL;
    CPlane3    Plane, * TestPlane;
    CVector3   Normal, CentrePoint;
    int        i, PlaneCount;
    float      Distance;

    // *************************
    // * Write Log Information *
    // *************************
    if ( m_pLogger )
    {
        m_pLogger->LogWrite( LOG_BSP, 0, true, _T("Building initial combined plane array \t- " ) );
        m_pLogger->SetRewindMarker( LOG_BSP );
        m_pLogger->LogWrite( LOG_BSP, 0, false, _T("0%%" ) );
        m_pLogger->SetProgressRange( m_lActiveFaces );
        m_pLogger->SetProgressValue( 0 );
    }
    // *************************
    // *    End of Logging     *
    // *************************

    // Loop through each face
    for ( Iterator = m_pFaceList; Iterator; Iterator = Iterator->Next )
    {
		//if (Iterator->Flags != FACE_WORLD) continue;

        // Calculate polygons centre point
        CentrePoint = Iterator->Vertices[0];
        for ( i = 1; i < (signed)Iterator->VertexCount; i++ ) CentrePoint += Iterator->Vertices[i];
        CentrePoint /= (float)Iterator->VertexCount;

        // Calculate polygons plane
        Plane = CPlane3( Iterator->Normal, CentrePoint );

        // Search through plane list to see if one already exists (must use small tolerances)
        PlaneCount = GetPlaneCount();
        for ( i = 0; i < PlaneCount; i++ )
        {
            // Retrieve the plane details
            TestPlane = GetPlane(i);
            
            // Test the plane details
            if ( fabsf(TestPlane->Distance - Plane.Distance) < 1e-5f &&
                 Plane.Normal.FuzzyCompare( TestPlane->Normal, 1e-5f )) break;

        } // Next Plane

        // Add plane if none found
        if ( i == PlaneCount )
        {
            if (!IncreasePlaneCount()) return BCERR_OUTOFMEMORY;
            *GetPlane( PlaneCount ) = Plane;

        } // End if no plane found

        // Store this plane index
        Iterator->Plane = i;

        // Retrieve the plane details
        Normal   = GetPlane(i)->Normal;
        Distance = GetPlane(i)->Distance;

        // Ensure that all vertices are on the selected plane
        for ( unsigned long v = 0; v < Iterator->VertexCount; v++ )
        {
            float result = Iterator->Vertices[v].Dot( Normal ) + Distance;
            Iterator->Vertices[v] += (Normal * -result);
        
        } // Next Vertex

        // Write Log Information etc.
        if ( m_pParent && !m_pParent->TestCompilerState() ) return BC_CANCELLED;
        if ( m_pLogger ) m_pLogger->UpdateProgress();

    } // Next Face

    // Success
    if ( m_pLogger ) m_pLogger->ProgressSuccess( LOG_BSP );
    return BC_OK;
    
}

//-----------------------------------------------------------------------------
// Name : BuildBSPTree () (Private, Recursive)
// Desc : Build's the entire BSP Tree using the already initialized poly data
// Note : We use 'goto' for the bail procedure for speed and stack space 
//        purposes only.
//-----------------------------------------------------------------------------
HRESULT CBSPTree::BuildBSPTree( unsigned long Node, CBSPFace * pFaceList )
{  
    // 49 Bytes including Parameter list (based on __thiscall declaration)
    CBSPFace     *TestFace	 = NULL, *NextFace	= NULL;
	CBSPFace     *FrontList	 = NULL, *BackList	= NULL;
    CBSPFace     *FrontSplit = NULL, *BackSplit	= NULL;
    CBSPFace     *Splitter   = NULL;
    HRESULT       ErrCode    = BC_OK;
    CLASSIFYTYPE  Result;
    int           v;

    // Check for pause / cancelled state
    if ( m_pParent && !m_pParent->TestCompilerState() ) { ErrCode = BC_CANCELLED; goto BuildError; }
    
    // Write Log Information
    if ( m_pLogger ) m_pLogger->UpdateProgress();

    // Select the best splitter from the list of faces passed
    Splitter = SelectBestSplitter( pFaceList, m_OptionSet.SplitterSample, m_OptionSet.SplitHeuristic);
    if (!Splitter) { ErrCode = BCERR_BSP_INVALIDGEOMETRY; goto BuildError; }

    // Flag as used, and store plane
    Splitter->UsedAsSplitter = true;
    GetNode(Node)->Plane     = Splitter->Plane;

    // Begin face iteration....
    // At each step we set the top level face list to the next face as well
    // as the iterator so that we will have a chance to release the remaining faces.
    for ( TestFace = pFaceList; TestFace != NULL; TestFace = NextFace, pFaceList = NextFace ) 
    {
        // Store plane for easy access
        CPlane3 * pPlane = GetPlane( TestFace->Plane );

        // Store next face, as 'TestFace' may be modified / deleted 
		NextFace = TestFace->Next;

        // Classify the polygon
        if ( TestFace->Plane == Splitter->Plane )
        {
            Result = CLASSIFY_ONPLANE;
        
        } // End if uses same plane
        else
        {
            Result = GetPlane(Splitter->Plane)->ClassifyPoly( TestFace->Vertices, TestFace->VertexCount, sizeof(CVertex));
        
        } // End if differing plane

        // Classify the polygon against the selected plane
		switch ( Result ) 
        {
            case CLASSIFY_ONPLANE:

                // Test the direction of the face against the plane.
                if ( GetPlane(Splitter->Plane)->SameFacing( pPlane->Normal ) ) 
                {
                    // Mark matching planes as used
                    if (!TestFace->UsedAsSplitter && TestFace->Flags == FACE_WORLD)
                    {
                        TestFace->UsedAsSplitter = true;

                        // Also Update progress info (Removed a potential splitter)
                        if ( m_pParent && !m_pParent->TestCompilerState() ) { ErrCode = BC_CANCELLED; goto BuildError; }
                        if ( m_pLogger ) m_pLogger->UpdateProgress( 1 );

                    } // End if !UsedAsSplitter

                    TestFace->Next	= FrontList;
                    FrontList		= TestFace;
                } 
                else 
                {
                    TestFace->Next	= BackList;
                    BackList		= TestFace;	
                } // End if Plane Facing
                break;

            case CLASSIFY_INFRONT:
			    // Pass the face straight down the front list.
			    TestFace->Next		= FrontList;
			    FrontList			= TestFace;		
			    break;

            case CLASSIFY_BEHIND:
			    // Pass the face straight down the back list.
			    TestFace->Next		= BackList;
			    BackList			= TestFace;	
			    break;

            case CLASSIFY_SPANNING:
                // Allocate new front fragment
                if (!(FrontSplit    = AllocBSPFace())) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
                FrontSplit->Next	= FrontList;
			    FrontList			= FrontSplit;
                
                // Allocate new back fragment
                if (!(BackSplit	    = AllocBSPFace())) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
                BackSplit->Next		= BackList;
			    BackList			= BackSplit;

                // Split the polygon
                if (FAILED( ErrCode = TestFace->Split(*GetPlane(Splitter->Plane), FrontSplit, BackSplit))) goto BuildError; 

                // Ensure that all vertices of the new fragment are on the original plane
                for ( v = 0; v < (signed)FrontSplit->VertexCount; v++ )
                {
                    float result = FrontSplit->Vertices[v].Dot( pPlane->Normal ) + pPlane->Distance;
                    FrontSplit->Vertices[v] += (pPlane->Normal * -result);
        
                } // Next Vertex

                // Ensure that all vertices of the new fragment are on the original plane
                for ( v = 0; v < (signed)BackSplit->VertexCount; v++ )
                {
                    float result = BackSplit->Vertices[v].Dot( pPlane->Normal ) + pPlane->Distance;
                    BackSplit->Vertices[v] += (pPlane->Normal * -result);
        
                } // Next Vertex

                // Update Progress Details (Added new potential splitters ??)
                if ( m_pParent && !m_pParent->TestCompilerState() ) { ErrCode = BC_CANCELLED; goto BuildError; }
                if ( !FrontSplit->UsedAsSplitter && m_pLogger ) m_pLogger->UpdateProgress( -1 );
                if ( !BackSplit->UsedAsSplitter  && m_pLogger ) m_pLogger->UpdateProgress( -1 );

                // + 2 Fragments - 1 Original
                m_lActiveFaces++; 

                // Free up original face (Also update progress info (Removed a potential Splitter?))
                if ( m_pParent && !m_pParent->TestCompilerState() ) { ErrCode = BC_CANCELLED; goto BuildError; }
                if ( !TestFace->UsedAsSplitter && m_pLogger ) m_pLogger->UpdateProgress( 1 );
			    delete TestFace;
                
			    break;

		} // End Switch

	} // End while loop

    // Should We Back Leaf Cull ?
    if ( m_OptionSet.RemoveBackLeaves ) 
    {
        // If No potential splitters remain, free the back list
        if ( BackList && CountSplitters( BackList ) == 0 )
        { 
            // Release illegal polygon fragments
			TestFace = BackList;
			while (TestFace) {
				CBSPFace *next = TestFace->Next;
				delete TestFace;
				m_lActiveFaces--;
				TestFace = next;
			}

            BackList = NULL; 

        } // End if No Splitters

    } // End If RemoveBackLeaves
   
    // Calculate the nodes bounding box
	GetNode(Node)->CalculateBounds( FrontList, true );
    GetNode(Node)->CalculateBounds( BackList, false );

	// If the front list is empty flag this as an empty leaf
	// otherwise push the front list down the front of this node
    if ( CountSplitters( FrontList ) == 0 ) 
    {
        // Add a new leaf and store the resulting faces
        if (!IncreaseLeafCount()) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
		GetNode(Node)->Front = -((long)GetLeafCount()); // Equates to -(Leaf Index + 1)
        if (FAILED(ErrCode = ProcessLeafFaces( GetLeaf( GetLeafCount() - 1 ), FrontList ))) goto BuildError;
    } 
    else 
    {
        // Allocate a new node and step into it
        if (!IncreaseNodeCount()) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
	    GetNode(Node)->Front = GetNodeCount() - 1;
	    ErrCode = BuildBSPTree( GetNode(Node)->Front, FrontList );

    } // End If FrontList

    // Front list has been passed off, we no longer own these
    FrontList = NULL;
    if ( FAILED(ErrCode) ) goto BuildError;

    // If the back list is empty flag this as solid
	// otherwise push the back list down the back of this node
	if ( !BackList ) 
    {
        // Set the back as a solid leaf
		GetNode(Node)->Back = BSP_SOLID_LEAF;
	} 
    else 
    {
        // No splitters remaining?
        if ( CountSplitters( BackList ) == 0 ) 
        {
            // Add a new leaf and store the resulting faces
            if (!IncreaseLeafCount()) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
            GetNode(Node)->Back = -((long)GetLeafCount()); // Equates to -(Leaf Index + 1)
            if (FAILED(ErrCode = ProcessLeafFaces( GetLeaf( GetLeafCount() - 1 ), BackList ))) goto BuildError;

        } // End if no splitters
        else
        {
            // Allocate a new node and step into it
            if (!IncreaseNodeCount()) { ErrCode = BCERR_OUTOFMEMORY; goto BuildError; }
	        GetNode(Node)->Back = GetNodeCount() - 1;
	        ErrCode = BuildBSPTree( GetNode(Node)->Back, BackList);

        } // End if remaining splitters

	} // End If BackList

    // Back list has been passed off, we no longer own these
    BackList = NULL;
    if ( FAILED(ErrCode) ) goto BuildError;

    // Success
    return BC_OK;

BuildError:
    // Add all currently allocated faces to garbage heap
    TrashFaceList( pFaceList );
    TrashFaceList( FrontList );
    TrashFaceList( BackList  );

    // Failed
    return ErrCode;
}

//-----------------------------------------------------------------------------
// Name : ProcessLeafFaces () (Private)
// Desc : This function decides what to do with the faces, intended for use
//        within a leaf, based upon the type of tree being compiled.
//----------------------------------------------------------------------------- 
HRESULT CBSPTree::ProcessLeafFaces( CBSPLeaf * pLeaf, CBSPFace * pFaceList )
{
    HRESULT ErrCode = BC_OK;

    // Depending on the tree type we may need to store these resulting faces
    if ( m_OptionSet.TreeType == BSP_TYPE_NONSPLIT )
    {
        // Add these faces to the leaf
        if ( FAILED( ErrCode = pLeaf->BuildFaceIndices( pFaceList ))) return ErrCode;

        // Release the faces
        FreeFaceList( pFaceList );
    
    } // End if NSR Type
    else
    {
        // Split tree, we need to store the face pointers
        CBSPFace * Iterator = pFaceList;
        while ( Iterator != NULL )
        {
            if (!IncreaseFaceCount()) return BCERR_OUTOFMEMORY;
            SetFace( GetFaceCount() - 1, Iterator );

            // Set our original index to the final position it 
            // is stored in the arrray (Split type only)
            Iterator->OriginalIndex = GetFaceCount() - 1;
            Iterator = Iterator->Next;
        
        } // Next Face

        // Add these faces to the leaf
        if ( FAILED( ErrCode = pLeaf->BuildFaceIndices( pFaceList ))) return ErrCode;

    } // End if Split Type

    // Success
    return BC_OK;
}
        

//-----------------------------------------------------------------------------
// Name : CountSplitters () (Private)
// Desc : Counts the number of splitters remaining in the list passed
//----------------------------------------------------------------------------- 
unsigned long CBSPTree::CountSplitters( CBSPFace * pFaceList ) const
{
    unsigned long SplitterCount = 0;
    CBSPFace     *Iterator      = pFaceList;
    
    // Count the number of splitters
	while ( Iterator != NULL ) 
    {
		if ( !Iterator->UsedAsSplitter && Iterator->Flags == FACE_WORLD) SplitterCount++;
		Iterator = Iterator->Next;

	} // End If

    // Return number of splitters remaining
    return SplitterCount;

}

//-----------------------------------------------------------------------------
// Name : SelectBestSplitter () (Private)
// Desc : Picks the next face in the list to be used as the Splitting plane.
// Note : You can pass a value to SplitHeuristic, the higher the value
//        the higher the importance is put on reducing splits.
//-----------------------------------------------------------------------------   
CBSPFace * CBSPTree::SelectBestSplitter( CBSPFace * pFaceList, unsigned long SplitterSample, float SplitHeuristic )
{
    unsigned long Score, Splits, BackFaces, FrontFaces;
	unsigned long BestScore = 10000000, SplitterCount = 0;
    CBSPFace *Splitter = pFaceList, *CurrentFace = NULL, *SelectedFace = NULL;
	
	// Traverse the Face Linked List
	while ( Splitter != NULL ) 
    {
		
        // If this has NOT been used as a splitter then
		if ( !Splitter->UsedAsSplitter && Splitter->Flags == FACE_WORLD)
        {
			
            // Create testing splitter plane
			CPlane3 SplittersPlane( Splitter->Normal, Splitter->Vertices[0] );
			
            // Test against the other poly's and count the score
			Score = Splits = BackFaces = FrontFaces = 0;
			for (CurrentFace = pFaceList; CurrentFace != NULL; CurrentFace = CurrentFace->Next)
            {
				if (CurrentFace->Flags != FACE_WORLD) continue;
                CLASSIFYTYPE Result = SplittersPlane.ClassifyPoly( CurrentFace->Vertices, CurrentFace->VertexCount, sizeof(CVertex) );
				switch ( Result ) 
                {
                    case CLASSIFY_INFRONT:
				        FrontFaces++;
					    break;

                    case CLASSIFY_BEHIND:
					    BackFaces++;
					    break;

				    case CLASSIFY_SPANNING:
					    Splits++;
					    break;

				    default:
					    break;

				} // switch
				
            } // Next Face
			
            // Tally the score (modify the splits * n )
			Score = (unsigned long)((long)abs( (long)(FrontFaces - BackFaces) ) + (Splits * SplitHeuristic));
			// Is this the best score ?
			if ( Score < BestScore) 
            {
				BestScore	 = Score;
				SelectedFace = Splitter;
            } // End if better score

            SplitterCount++;
		} // End if this splitter has not been used yet

		Splitter = Splitter->Next;
        // Break if we reached our splitter sample limit.
        if (SplitterSample != 0 && SplitterCount >= SplitterSample && SelectedFace) break;

    } // Next Splitter

    // This will be NULL if no faces remained to be used
    return SelectedFace;
}

//-----------------------------------------------------------------------------
// Name : ClipTree ()
// Desc : Used to clip the BSP Tree passed in, against this tree.
// Note : This function is primarily used by CSG operations.
//-----------------------------------------------------------------------------  
HRESULT CBSPTree::ClipTree( CBSPTree * pTree, bool ClipSolid, bool RemoveCoPlanar, ULONG CurrentNode, CBSPFace * pFaceList )
{
    // 50 Bytes including Parameter list (based on __thiscall declaration)
    CBSPFace     *TestFace	 = NULL, *NextFace	= NULL;
	CBSPFace     *FrontList	 = NULL, *BackList	= NULL;
    CBSPFace     *FrontSplit = NULL, *BackSplit	= NULL;
    unsigned long Plane      = 0;
    HRESULT       ErrCode    = BC_OK;

    // Validate Params
    if (!pTree) return BCERR_INVALIDPARAMS;

    // Did Someone use or pass in a silly tree ?
	if (pTree->GetFaceCount() < 1 || GetFaceCount() < 1 ) return BCERR_BSP_INVALIDGEOMETRY;

	// If this is the first call to ClipTree, then we must build our 
	// lists to work with (this just takes some work away from the caller)
	if ( pFaceList == NULL ) 
	{
		// Build our sequential linked list
		UINT i;
		for (i = 0; i < pTree->GetFaceCount(); i++ )
		{
			pTree->GetFace(i)->ChildSplit[0]= -1;
			pTree->GetFace(i)->ChildSplit[1]= -1;
            if ( i > 0 ) pTree->GetFace(i - 1)->Next = pTree->GetFace(i);

		} // Next Face
		
        // Reset last face just to be certain
		pTree->GetFace(i - 1)->Next = NULL;
        pFaceList = pTree->GetFace(0);

	} // End If No Faces

    // All polygons are now hooked together in a linked list with the last one in the array pointing to NULL;
	// Now we loop through the linked list starting at the first face in the array and send them down this tree.
    Plane = GetNode( CurrentNode )->Plane;
    
    for ( TestFace = pFaceList; TestFace; TestFace = NextFace )
    {   
        // Store next face, as 'TestFace' may be modified / deleted 
		NextFace = TestFace->Next;

        // Skip this polygon it has been deleted in some previous csg op
		if ( TestFace->Deleted ) continue;

        // Classify the polygon against the selected plane
		switch ( GetPlane(Plane)->ClassifyPoly( TestFace->Vertices, TestFace->VertexCount, sizeof(CVertex) ) ) 
        {
            case CLASSIFY_ONPLANE:
			    // Test the direction of the face against the plane.
                if ( GetPlane(Plane)->SameFacing( TestFace->Normal ) ) 
                {
                    if ( RemoveCoPlanar ) { TestFace->Next = BackList;  BackList  = TestFace; } 
                    else                  { TestFace->Next = FrontList; FrontList = TestFace; }	
			    } 
                else 
                {
				    TestFace->Next	= BackList;
				    BackList		= TestFace;	
			    } // End if Plane Facing
                break;

            case CLASSIFY_INFRONT:
			    // Pass the face straight down the front list.
			    TestFace->Next		= FrontList;
			    FrontList			= TestFace;		
			    break;

            case CLASSIFY_BEHIND:
			    // Pass the face straight down the back list.
			    TestFace->Next		= BackList;
			    BackList			= TestFace;	
			    break;

            case CLASSIFY_SPANNING:
			    // Allocate new front within the passed tree
                if (!(FrontSplit = AllocBSPFace())) { ErrCode = BCERR_OUTOFMEMORY; goto ClipError; }
                if (!pTree->IncreaseFaceCount()) { ErrCode = BCERR_OUTOFMEMORY; goto ClipError; }
                pTree->SetFace( pTree->GetFaceCount() - 1, FrontSplit );

                // Allocate new back fragment within the passed tree
                if (!(BackSplit	= AllocBSPFace())) { ErrCode = BCERR_OUTOFMEMORY; goto ClipError; }
                if (!pTree->IncreaseFaceCount()) { ErrCode = BCERR_OUTOFMEMORY; goto ClipError; }
                pTree->SetFace( pTree->GetFaceCount() - 1, BackSplit );
			    
                // Split the polygon
                if (FAILED( ErrCode = TestFace->Split(*GetPlane(Plane), FrontSplit, BackSplit))) goto ClipError;
                
                // Since this is a coincidental pre-process on mini bsp trees
				// we don't actually need to update the leaf polys. Which is 
				// convenient =)
				TestFace->Deleted    = true;
				TestFace->ChildSplit[0] = pTree->GetFaceCount() - 2;
				TestFace->ChildSplit[1] = pTree->GetFaceCount() - 1;

                // Add it to the head of our recursion lists
                FrontSplit->Next = FrontList;
                FrontList        = FrontSplit;
                BackSplit->Next  = BackList;
                BackList         = BackSplit;

			    break;

		} // End Switch

    } // Next Face
	

    // Now onto the clipping
	if ( ClipSolid )
	{
		if ( GetNode(CurrentNode)->Back == BSP_SOLID_LEAF ) 
		{
			// Iterate through and flag all back polys as deleted
			for ( TestFace = BackList; TestFace; TestFace = TestFace->Next ) TestFace->Deleted = true;
			
			// Empty Back List
			BackList = NULL;     
        
        } // End if Back == Solid
	
    }  // End if clipping solid
	else
	{
        if ( GetNode(CurrentNode)->Back < 0 ) 
        {
            // Iterate through and flag all back polys as deleted
            for ( TestFace = BackList; TestFace; TestFace = TestFace->Next ) TestFace->Deleted = true;

            // Empty Back List
            BackList = NULL;     

        } // End if Back == Empty

		if ( GetNode(CurrentNode)->Front < 0 ) 
		{
			// Iterate through and flag all front polys as deleted
			for ( TestFace = FrontList; TestFace; TestFace = TestFace->Next ) TestFace->Deleted = true;
			
			// Empty Front List
			FrontList = NULL;     
        
        } // End if Front == Empty
		
	} // End if clipping empty

	
	// Pass down the front of the node
    if ( FrontList && GetNode(CurrentNode)->Front >= 0 )
		ErrCode = ClipTree( pTree, ClipSolid, RemoveCoPlanar, GetNode(CurrentNode)->Front, FrontList);
    
    if ( FAILED( ErrCode ) ) goto ClipError;

    // Pass down the back of the node
    if ( BackList && GetNode(CurrentNode)->Back >= 0 )
		ErrCode = ClipTree( pTree, ClipSolid, RemoveCoPlanar, GetNode(CurrentNode)->Back, BackList);

    if ( FAILED( ErrCode ) ) goto ClipError;

	// Success!
	return BC_OK;

ClipError:
    // Failed
    return ErrCode;
}

//-----------------------------------------------------------------------------
// Name : RepairSplits ()
// Desc : Repairs any unnecesary splits caused during the ClipTree operation.
// Note : Because polygons are only flagged as being deleted during clipping
//        this function merely alters the deleted flag values.
//----------------------------------------------------------------------------- 
void CBSPTree::RepairSplits()
{
    // Test for child polygon survival and replace with parent polygons if possible
    for ( int i = GetFaceCount() - 1; i >= 0; i--)
    {
        // If one isn't -1, neither is the other
        if ( GetFace(i)->ChildSplit[0] != -1) 
        {
            // If the two children are valid this split should be repaired
            if ( !GetFace( GetFace(i)->ChildSplit[0] )->Deleted && !GetFace( GetFace(i)->ChildSplit[1] )->Deleted )
            {
                // Restore the parent, and delete the children
                GetFace( i )->Deleted = false;
                GetFace( GetFace(i)->ChildSplit[0] )->Deleted = true;
                GetFace( GetFace(i)->ChildSplit[1] )->Deleted = true;

            } // End if Both Valid

        } // End if Has Children

    } // Next Face
}

//-----------------------------------------------------------------------------
// Name : IntersectedByTree()
// Desc : Tests each face in the tree passed to see if it intersects this.
//-----------------------------------------------------------------------------
bool CBSPTree::IntersectedByTree( const CBSPTree * pTree ) const
{
    // Loop through each face testing for intersection
    for ( ULONG i = 0; i < pTree->GetFaceCount(); i++ ) 
    {
        if ( IntersectedByFace( pTree->GetFace(i) ) ) return true;

    } // Next Face

    // No Intersection
    return false;
}

//-----------------------------------------------------------------------------
// Name : IntersectedByFace () (Recursive)
// Desc : This function tests the passed face against the BSP Tree. If any part
//        of the face ends up in solid space, then this face is considered to
//        be intersecting the BSP Tree.
//-----------------------------------------------------------------------------
bool CBSPTree::IntersectedByFace( const CFace * pFace, ULONG Node /* = 0 */ ) const
{
    // Validate Params
    if (!pFace || Node < 0 || Node >= GetNodeCount() ) return false;

    int NodeFront = GetNode( Node )->Front;
    int NodeBack  = GetNode( Node )->Back;

    // Classify this poly against the nodes plane
    switch ( GetPlane(GetNode(Node)->Plane)->ClassifyPoly( pFace->Vertices, pFace->VertexCount, sizeof(CVertex) ) ) 
    {
        case CLASSIFY_SPANNING:
        case CLASSIFY_ONPLANE:
            // Solid Leaf
            if (NodeBack == BSP_SOLID_LEAF ) return true;
            // Pass down the front
            if (NodeFront >= 0 ) { if ( IntersectedByFace( pFace, NodeFront ) ) return true; }
            // Pass down the back
            if (NodeBack  >= 0 ) { if ( IntersectedByFace( pFace, NodeBack  ) ) return true; }
            break;

        case CLASSIFY_INFRONT:
            // Pass down the front
            if (NodeFront >= 0 ) { if ( IntersectedByFace( pFace, NodeFront ) ) return true; }
            break;

        case CLASSIFY_BEHIND:
            // Solid Leaf
            if (NodeBack == BSP_SOLID_LEAF ) return true;
            // Pass down the back
            if (NodeBack  >= 0 ) { if ( IntersectedByFace( pFace, NodeBack  ) ) return true; }
            break;

    } // End Classify Switch

    // No Intersection at this level
    return false;
}

bool CBSPTree::RayIntersect(const CVector3 & p, const CVector3 & dir)
{
	return RayIntersectRecurse(0, { p.x, p.y, p.z }, { dir.x, dir.y, dir.z });
}

// Intersect ray/segment against bsp tree
bool CBSPTree::RayIntersectRecurse(long iNode, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir)
{
	CLASSIFYTYPE pointA, pointB;
	XMFLOAT3    rayEnd, intersection, newDir;
	float       t;

	// No operation if the node was null
	//if (!pNode) return false;

	// If this node stores a leaf, just add it to the list and return
	if (iNode < 0)
	{
		if (iNode == BSP_SOLID_LEAF) {
			return true;
		}

		CBSPLeaf *pLeaf = m_vpLeaves[abs(iNode + 1)];
		if (pLeaf->RayIntersection(this, rayOrigin, rayDir)) {
			return true;
		}

		return false;
	} // End if a leaf

	// Calculate the end point of the ray
	//RayEnd = RayOrigin + rayDir;
	XMStoreFloat3(&rayEnd,
		XMVectorAdd(
			XMLoadFloat3(&rayOrigin),
			XMLoadFloat3(&rayDir)));

	CBSPNode *pNode = m_vpNodes[iNode];
	// Retrieve the plane, and classify the ray points against it
	CPlane3  *pPlane = m_vpPlanes[pNode->Plane];
	XMFLOAT3 planeNormal = XMFLOAT3(pPlane->Normal.x, pPlane->Normal.y, pPlane->Normal.z);
	pointA = PointClassifyPlane(rayOrigin, planeNormal, pPlane->Distance);
	pointB = PointClassifyPlane(rayEnd, planeNormal, pPlane->Distance);

	// Test for the combination of ray point positions
	if (pointA == CLASSIFY_ONPLANE && pointB == CLASSIFY_ONPLANE)
	{
		// Traverse down the front and back
		if (RayIntersectRecurse(pNode->Front, rayOrigin, rayDir)) return true;
		if (RayIntersectRecurse(pNode->Back, rayOrigin, rayDir)) return true;
		return false;
	} // End If both points on plane
	else if (pointA == CLASSIFY_INFRONT && pointB == CLASSIFY_BEHIND)
	{
		// The ray is spanning the plane, with the origin in front. Get the intersection point
		RayIntersectPlane(rayOrigin, rayDir, planeNormal, pPlane->Distance, t, true);
		//intersection = rayOrigin + (rayDir * t);
		XMStoreFloat3(&intersection,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&rayDir),
				XMVectorReplicate(t),
				XMLoadFloat3(&rayOrigin)));

		// Traverse down both sides passing the relevant segments of the ray
		XMStoreFloat3(&newDir, XMVectorSubtract(XMLoadFloat3(&intersection), XMLoadFloat3(&rayOrigin)));
		if (RayIntersectRecurse(pNode->Front, rayOrigin, newDir)) return true;
		XMStoreFloat3(&newDir, XMVectorSubtract(XMLoadFloat3(&rayEnd), XMLoadFloat3(&intersection)));
		if (RayIntersectRecurse(pNode->Back, intersection, newDir)) return true;
		return false;
	} // End If Spanning with origin in front
	else if (pointA == CLASSIFY_BEHIND && pointB == CLASSIFY_INFRONT)
	{
		// The ray is spanning the plane, with the origin in front. Get the intersection point
		RayIntersectPlane(rayOrigin, rayDir, planeNormal, pPlane->Distance, t, true);
		//intersection = rayOrigin + (rayDir * t);
		XMStoreFloat3(&intersection,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&rayDir),
				XMVectorReplicate(t),
				XMLoadFloat3(&rayOrigin)));

		// Traverse down both sides passing the relevant segments of the ray
		XMStoreFloat3(&newDir, XMVectorSubtract(XMLoadFloat3(&rayEnd), XMLoadFloat3(&intersection)));
		if (RayIntersectRecurse(pNode->Front, intersection, newDir)) return true;
		XMStoreFloat3(&newDir, XMVectorSubtract(XMLoadFloat3(&intersection), XMLoadFloat3(&rayOrigin)));
		if (RayIntersectRecurse(pNode->Back, rayOrigin, newDir)) return true;
		return false;
	} // End If Spanning with origin in front
	else if (pointA == CLASSIFY_INFRONT || pointB == CLASSIFY_INFRONT)
	{
		// Either of the points are in front (but not spanning), just pass down the front
		return RayIntersectRecurse(pNode->Front, rayOrigin, rayDir);
	} // End if either point in front
	else
	{
		// Either of the points are behind (but not spanning), just pass down the back
		return RayIntersectRecurse(pNode->Back, rayOrigin, rayDir);
	} // End if either point behind
}

//-----------------------------------------------------------------------------
// Name : FindLeaf () 
// Desc : Given a position, this function attempts to determine which leaf
//        that point falls into.
//-----------------------------------------------------------------------------
long CBSPTree::FindLeaf( const CVector3& Position ) const
{
    long Node = 0, Leaf = 0;
	
	for (;;)
    {
        CBSPNode * pNode = GetNode( Node );

		switch ( GetPlane( pNode->Plane )->ClassifyPoint( Position ) ) 
        {
            case CLASSIFY_ONPLANE:    
            case CLASSIFY_INFRONT:
			
                if ( pNode->Front < 0 )
                {
                    return abs( pNode->Front + 1 );
                
                } // End if Leaf
                else
                {
                    Node = pNode->Front;
                
                } // End if Node
				break;

			case CLASSIFY_BEHIND: 
				
                if ( pNode->Back == BSP_SOLID_LEAF ) 
                {
					return BSP_SOLID_LEAF;
                
                } // End if Solid Leaf
                else if ( pNode->Back < 0 )
                {
                    return abs( pNode->Back + 1 );

                } // End if Leaf
                else 
                {
					Node = pNode->Back;

				} // End if Node
				break;

        } // End Switch

	} // Next Iteration

    // Can NEVER happen, but hey :)
    return BSP_SOLID_LEAF;
}

std::vector<unsigned long> CBSPTree::FindPVSLeafIndices(const CBSPLeaf *pCurrentLeaf) const
{
	
	std::vector<unsigned long> pvsLeavesIndices;

	if (!m_pPVSData) {
		size_t LeafCount = GetLeafCount();
		for (size_t LeafIndex = 0; LeafIndex < LeafCount; ++LeafIndex)
		{
			pvsLeavesIndices.push_back(LeafIndex);
		}
		return pvsLeavesIndices;
	}

	// We are in a valid leaf, let's test its PVS.
	UCHAR *pPVS = &m_pPVSData[pCurrentLeaf->PVSIndex];
	size_t LeafCount = GetLeafCount();

	// Loop through and render applicable leaves
	for (size_t LeafIndex = 0; LeafIndex < LeafCount; )
	{
		// Is this a non 0 PVS byte (or ZRLE not used) ?
		if (*pPVS != 0)
		{
			// Check the 8 bits in this byte
			for (int i = 0; i < 8; i++)
			{
				UCHAR Mask = 1 << i;
				UCHAR Data = *pPVS;

				// Is this leaf visible ?
				if (Data & Mask)
				{
					pvsLeavesIndices.push_back(LeafIndex);
				} // End if leaf visible

				  // Move on to the next leaf
				LeafIndex++;

				// Break out if we are about to overflow
				if (LeafIndex == LeafCount) break;

			} // Next Leaf in Byte

		} // End if Non-Zero
		else
		{
			// Is compression in use ?
			if (m_bPVSCompressed)
			{
				// Step to the run length byte
				pPVS++;
				// This is a ZRLE Compressed Packet, read run length * 8 leaves (8 bits per byte)
				size_t RunLength = (*pPVS) * 8;
				// Skip this amount of leaves
				LeafIndex += RunLength;
			} // Compressed
			else
			{
				// Simply skip 8 leaves
				LeafIndex += 8;
			} // Uncompressed
		} // End if ZRLE Packet

		  // Move to next byte
		pPVS++;
	}

	return pvsLeavesIndices;
}

//-----------------------------------------------------------------------------
// Name : SetPVSData ()
// Desc : Allocate and store any passed PVS Data associated with this tree.
//-----------------------------------------------------------------------------   
HRESULT CBSPTree::SetPVSData( UCHAR PVSData[], unsigned long PVSSize, bool PVSCompressed )
{
    // Release any previous data
    if (m_pPVSData) delete[] m_pPVSData;

    try
    {
        // Allocate the PVS Set
        m_pPVSData = new UCHAR[ PVSSize ];
        if (!m_pPVSData) throw std::bad_alloc(); // VC++ Compat

        // Copy over the data
        memcpy( m_pPVSData, PVSData, PVSSize );

    } // End Try Block

    catch ( std::bad_alloc )
    {
        return BCERR_OUTOFMEMORY;

    } // End Catch Block

    // Store Values
    m_lPVSDataSize      = PVSSize;
    m_bPVSCompressed    = PVSCompressed;

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : FreeFaceList () (Private)
// Desc : Frees the face linked list passed to the function
//-----------------------------------------------------------------------------   
void CBSPTree::FreeFaceList( CBSPFace * pFaceList )
{
    // Free up the linked list
    CBSPFace * TestFace = pFaceList, *NextFace = NULL;
    while ( TestFace != NULL ) 
    {
        NextFace = TestFace->Next;
        delete TestFace;
        m_lActiveFaces--;
        TestFace = NextFace;
    } // Next Face
}

//-----------------------------------------------------------------------------
// Name : TrashFaceList () (Private)
// Desc : Unlike 'FreeFaceList' this function simply adds the faces from the
//        list to the garbage list for clean up later. This is provided to
//        prevent the same face pointer from being freed multiple times 
//        during recursion unwind, and causing a mem exception.
//-----------------------------------------------------------------------------   
void CBSPTree::TrashFaceList( CBSPFace * pFaceList )
{
    CBSPFace * Iterator = pFaceList;

    while ( Iterator != NULL ) 
    {
        // If it doesn't already exist then add it to the list
        if (std::find(m_vpGarbage.begin(), m_vpGarbage.end(), Iterator) == m_vpGarbage.end()) 
        {
           
            try 
            {
                // Resize the vector if we need to
                if (m_vpGarbage.size() >= (m_vpGarbage.capacity() - 1)) 
                {
                    m_vpGarbage.reserve( m_vpGarbage.size() + BSP_ARRAY_THRESHOLD );
                } // End If
                // Push back this new item
                m_vpGarbage.push_back(Iterator);
            
            } // End Try

            // On exception, we can do nothing but bail and leak
            catch (std::exception&) { return; }
        
        } // End If Exists

        Iterator = Iterator->Next;

    } // Next Face
}

//-----------------------------------------------------------------------------
// Name : IncreaseNodeCount () (Private)
// Desc : Reallocate Memory For Node Array as Needed
//-----------------------------------------------------------------------------
bool CBSPTree::IncreaseNodeCount()
{
	CBSPNode *NewNode = NULL;

    try 
    {
        // Resize the vector if we need to
        if (m_vpNodes.size() >= (m_vpNodes.capacity() - 1)) 
        {
            m_vpNodes.reserve( m_vpNodes.size() + BSP_ARRAY_THRESHOLD );
        } // End If

        // Allocate a new Node ready for storage
        // Note : VC++ new does not throw an exception on failure (easily ;)
        if (!(NewNode = new CBSPNode)) throw std::bad_alloc();
        
        // Push back this new Node
        m_vpNodes.push_back(NewNode);
    } // Try vector ops
    
    // Catch Failures
    catch (std::bad_alloc) 
    { 
        return false; 
    } 
    catch (...) 
    { 
        if (NewNode) delete NewNode; 
        return false;
    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : IncreaseLeafCount () (Private)
// Desc : Reallocate Memory For Leaf Array as Needed
//-----------------------------------------------------------------------------
bool CBSPTree::IncreaseLeafCount()
{
	CBSPLeaf *NewLeaf = NULL;

    try 
    {
        // Resize the vector if we need to
        if (m_vpLeaves.size() >= (m_vpLeaves.capacity() - 1)) 
        {
            m_vpLeaves.reserve( m_vpLeaves.size() + BSP_ARRAY_THRESHOLD );
        } // End If
        
        // Allocate a new leaf ready for storage
        // Note : VC++ new does not throw an exception on failure (easily ;)
        if (!(NewLeaf = new CBSPLeaf)) throw std::bad_alloc();

        // Push back this new leaf
        m_vpLeaves.push_back(NewLeaf);
    } // Try vector ops

    // Catch Failures
    catch (std::bad_alloc) 
    { 
        return false; 
    } 
    catch (...) 
    { 
        if (NewLeaf) delete NewLeaf; 
        return false;
    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : IncreasePlaneCount () (Private)
// Desc : Reallocate Memory For Plane Array as Needed
//-----------------------------------------------------------------------------
bool CBSPTree::IncreasePlaneCount()
{
	CPlane3 *NewPlane = NULL;

    try 
    {
        // Resize the vector if we need to
        if (m_vpPlanes.size() >= (m_vpPlanes.capacity() - 1)) 
        {
            m_vpPlanes.reserve( m_vpPlanes.size() + BSP_ARRAY_THRESHOLD );
        } // End If

        // Allocate a new plane ready for storage
        // Note : VC++ new does not throw an exception on failure (easily ;)
        if (!(NewPlane = new CPlane3)) throw std::bad_alloc();

        // Push back this new plane
        m_vpPlanes.push_back(NewPlane);
    } // Try vector ops

    // Catch Failures
    catch (std::bad_alloc) 
    { 
        return false; 
    } 
    catch (...) 
    { 
        if (NewPlane) delete NewPlane;
        return false;
    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : IncreaseFaceCount () (Private)
// Desc : Reallocate Memory For resulting BSPFace Array as Needed
//-----------------------------------------------------------------------------
bool CBSPTree::IncreaseFaceCount()
{

    try 
    {
        // Resize the vector if we need to
        if (m_vpFaces.size() >= (m_vpFaces.capacity() - 1)) 
        {
            m_vpFaces.reserve( m_vpFaces.size() + BSP_ARRAY_THRESHOLD );
            
        } // End If

        // Push back a NULL pointer ( will already be allocated on storage)
        m_vpFaces.push_back( NULL );
    
    } // Try vector ops

    // Catch Failures
    catch (...) 
    { 
        return false; 
    
    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : IncreasePortalCount () (Private)
// Desc : Reallocate Memory For resulting BSPPortal Array as Needed
//-----------------------------------------------------------------------------
bool CBSPTree::IncreasePortalCount()
{
    try 
    {
        // Resize the vector if we need to
        if (m_vpPortals.size() >= (m_vpPortals.capacity() - 1)) 
        {
            m_vpPortals.reserve( m_vpPortals.size() + BSP_ARRAY_THRESHOLD );
            
        } // End If

        // Push back a NULL pointer (will already be allocated on storage)
        m_vpPortals.push_back( NULL );
    
    } // Try vector ops

    // Catch Failures
    catch (...) 
    { 
        return false; 
    
    } // End Catch

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : AllocBSPFace () (Static)
// Desc : Simply allocate a brand new CBSPFace object and return it.
// Note : Provided for ease of use, and to provide some level of portability
//        without polluting the main code base.
//-----------------------------------------------------------------------------
CBSPFace * CBSPTree::AllocBSPFace( const CFace * pDuplicate )
{
    CBSPFace * NewFace = NULL;

    try 
    {
        // Call the corresponding constructor
        if (pDuplicate != NULL) 
        {
            NewFace = new CBSPFace( pDuplicate );
        } 
        else 
        {
            NewFace = new CBSPFace;
        } // End If pDuplicate
        
        // Note : VC++ new does not throw an exception on failure (easily ;)
        if (!NewFace) throw std::bad_alloc();

    } // End Try
    
    catch (...) { return NULL; }

    // Success!
    return NewFace;
}

//-----------------------------------------------------------------------------
// Name : AllocBSPPortal () (Static)
// Desc : Simply allocate a brand new CBSPPortal object and return it.
// Note : Provided for ease of use, and to provide some level of portability
//        without polluting the main code base.
//-----------------------------------------------------------------------------
CBSPPortal * CBSPTree::AllocBSPPortal( )
{
    CBSPPortal * NewPortal = NULL;

    try 
    {
        // Allocate new portal
        NewPortal = new CBSPPortal;

        // Note : VC++ new does not throw an exception on failure (easily ;)
        if (!NewPortal) throw std::bad_alloc();

    } // End Try
    
    catch (...) { return NULL; }

    // Success!
    return NewPortal;
}

double SquaredDistPointAABB(const CVector3 & p, const CBounds3 & aabb)
{
	auto check = [&](
		const double pn,
		const double bmin,
		const double bmax) -> double
	{
		double out = 0;
		double v = pn;

		if (v < bmin)
		{
			double val = (bmin - v);
			out += val * val;
		}

		if (v > bmax)
		{
			double val = (v - bmax);
			out += val * val;
		}

		return out;
	};

	// Squared distance
	double sq = 0.0;

	sq += check(p.x, aabb.Min.x, aabb.Max.x);
	sq += check(p.y, aabb.Min.y, aabb.Max.y);
	sq += check(p.z, aabb.Min.z, aabb.Max.z);

	return sq;
}