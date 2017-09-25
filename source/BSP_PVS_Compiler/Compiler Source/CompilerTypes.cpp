//-----------------------------------------------------------------------------
// File: CompilerTypes.cpp
//
// Desc: This file houses the various implementations of the classes such as
//       brushes, faces etc required by the various compilation processes
//

//-----------------------------------------------------------------------------
// Specific includes required for these classes
//-----------------------------------------------------------------------------
#include <new>
#include "CompilerTypes.h"
#include "..\\Support Source\\CPlane.h"
#include "..\\Support Source\\CBounds.h"
#include "..\\Compiler Source\\CBSPTree.h"

//-----------------------------------------------------------------------------
// Desc : CMesh member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CMesh () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CMesh::CMesh()
{
	// Initialise anything we need
	Faces       = NULL;
    FaceCount   = 0;
    Flags       = 0;
    Name        = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CMesh () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
CMesh::~CMesh()
{
	// Clean up after ourselves
	ReleaseFaces();
    if ( Name ) delete[] Name;
}

//-----------------------------------------------------------------------------
// Name : AddFaces()
// Desc : Adds the specified number of faces to this mesh, this function
//        returns an index to the first face added, allowing you to step
//        through the newly allocated faces on return.
//-----------------------------------------------------------------------------
long CMesh::AddFaces( unsigned long nFaceCount )
{
    CFace **FaceBuffer = NULL;

    // Validate Requirements
	if ( nFaceCount == 0 ) return -1;
	
    try 
    {
        // Allocate brand new buffer
        FaceBuffer = new CFace*[ FaceCount + nFaceCount ];
        if (!FaceBuffer) throw std::bad_alloc(); // VC++ Compat
        ZeroMemory( FaceBuffer, (FaceCount + nFaceCount) * sizeof(CFace*) );

        // Copy over any old data
	    if (Faces) memcpy( FaceBuffer, Faces, FaceCount * sizeof(CFace*));
            
        // Allocate new faces
        for ( long i = 0; i < (signed)nFaceCount; i++ )
        {
            FaceBuffer[FaceCount + i] = new CFace;
            if (!FaceBuffer[FaceCount + i]) throw std::bad_alloc(); // VC++ Compat
        
        } // Next face

    } // End Try Block

    // Did an exception get thrown ?
    catch (...)
    {
        if (FaceBuffer) 
        {
            // Release any already allocated faces
            for ( long i = 0; i < (signed)nFaceCount; i++ )
            {
                if (FaceBuffer[FaceCount + i]) delete FaceBuffer[FaceCount + i];
        
            } // Next face

            // Release buffer
            delete []FaceBuffer;
        
        } // End if FaceBuffer

        return -1;
    
    } // End Catch Block
		
    // Free up old buffer and store new
    if (Faces) delete []Faces;
	Faces = FaceBuffer;

	// Increment face count
	FaceCount += nFaceCount;

	// Return the base face
	return FaceCount - nFaceCount;

}

//-----------------------------------------------------------------------------
// Name : ReleaseFaces ()
// Desc : Use this function to release all the faces and their associated
//        data / variables. This does not destroy the mesh itself.
//-----------------------------------------------------------------------------
void CMesh::ReleaseFaces()
{
    // Clean up after ourselves
    if ( Faces ) 
    { 
        // Release all face pointers
        for ( long i = 0; i < (signed)FaceCount; i++ )
        {
            if ( Faces[i] ) delete Faces[i];
        
        } // Next face

        // Release the array itself
        delete []Faces; 
        Faces = NULL;
    
    } // End if Faces

    FaceCount = 0;
}

//-----------------------------------------------------------------------------
// Name : BuildFromBSPTree ()
// Desc : Rebuild this mesh based on the BSP Tree information passed
//-----------------------------------------------------------------------------
bool CMesh::BuildFromBSPTree( const CBSPTree * pTree, bool Reset /* = false */ )
{
    ULONG Counter = 0, i;

    // Validate Data
    if (!pTree) return false;

    // Reset if requested
    if (Reset) { ReleaseFaces(); Matrix.Identity(); }
    
    // First count the number of non-deleted faces,
    // so that we can allocate in one batch.
    for ( i = 0; i < pTree->GetFaceCount(); i++ )
    {
        if ( !pTree->GetFace(i)->Deleted && pTree->GetFace(i)->VertexCount >= 3 ) Counter++;

    } // Next BSP Face

    // Allocate our faces
    Counter = AddFaces( Counter );
    if ( Counter < 0 ) return false;

    // Now we loop through and copy over the data
    for ( i = 0; i < pTree->GetFaceCount(); i++ )
    {
        CBSPFace * pTreeFace = pTree->GetFace(i);

        // Validate tree face
        if ( pTreeFace->Deleted ) continue;
        if ( pTreeFace->VertexCount < 3 ) continue;

        // Retrieve faces for processing
        CFace    * pFace     = Faces[Counter];

        // Copy over data
        pFace->MaterialIndex = pTreeFace->MaterialIndex;
        pFace->TextureIndex  = pTreeFace->TextureIndex;
        pFace->ShaderIndex   = pTreeFace->ShaderIndex;
        pFace->Flags         = pTreeFace->Flags;
        pFace->SrcBlendMode  = pTreeFace->SrcBlendMode;
        pFace->DestBlendMode = pTreeFace->DestBlendMode;
        pFace->Normal        = pTreeFace->Normal;
        
        // Copy over vertices
        if ( pFace->AddVertices( pTreeFace->VertexCount ) < 0 ) return false;
        memcpy( pFace->Vertices, pTreeFace->Vertices, pFace->VertexCount * sizeof(CVertex) );

        // We used a new face
        Counter++;

    } // Next BSP Face

    // Calculate out bounding box
    CalculateBoundingBox();

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : CalculateBoundingBox ()
// Desc : Calculates the object space bounding box for this mesh based on the
//        face / vertex data stored.
//-----------------------------------------------------------------------------
const CBounds3& CMesh::CalculateBoundingBox()
{
    // Reset bounding box
    Bounds.Reset();

    // Loop through each face
    for ( long i = 0; i < (signed)FaceCount; i++ )
    {
        Bounds.CalculateFromPolygon( Faces[i]->Vertices, Faces[i]->VertexCount, sizeof(CVertex), false );

    } // Next face

    // Return our bounds reference
    return Bounds;
}

//-----------------------------------------------------------------------------
// Desc : CPolygon member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPolygon () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPolygon::CPolygon()
{
	// Initialise anything we need
	Vertices			= NULL;
    VertexCount			= 0;
}

//-----------------------------------------------------------------------------
// Name : ~CPolygon () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
CPolygon::~CPolygon()
{
	// Clean up after ourselves
	ReleaseVertices();
}

//-----------------------------------------------------------------------------
// Name : AddVertices()
// Desc : Adds the specified number of vertices to this face, this function
//        returns an index to the first vertex added, allowing you to step
//        through the newly allocated vertices on return.
//-----------------------------------------------------------------------------
long CPolygon::AddVertices( unsigned long nVertexCount )
{
    CVertex *VertexBuffer = NULL;

    // Validate Requirements
	if ( nVertexCount == 0 ) return -1;
	
	// Allocate brand new buffer
    try 
    {
        VertexBuffer = new CVertex[ VertexCount + nVertexCount ];
        if (!VertexBuffer) throw std::bad_alloc(); // VC++ Compat

        // If any old data
	    if (Vertices) 
        {    
            // Copy over old data
		    memcpy( VertexBuffer, Vertices, VertexCount * sizeof(CVertex));
		    
            // Release the memory allocated for the original vertex array
            delete []Vertices;
		    
	    } // End if old vertices created
    
    } // End try block

    // Was an exception thrown?
    catch (...)
    {
        if (VertexBuffer) delete []VertexBuffer;
        return -1;

    } // End catch block

	// Store new buffer
	Vertices    = VertexBuffer;

	// Increment vertex count
	VertexCount += nVertexCount;

	// Return the base vertex
	return VertexCount - nVertexCount;

}

//-----------------------------------------------------------------------------
// Name : InsertVertex ()
// Desc : Inserts a single vertex at the position specified by nVertexPos.
//-----------------------------------------------------------------------------
long CPolygon::InsertVertex( unsigned long nVertexPos )
{
    CVertex *VertexBuffer = NULL;

	// Add a vertex to the end
    if ( AddVertices( 1 ) < 0 ) return -1;

    // Reshuffle the array unlesss we have inserted at the end
    if ( nVertexPos != VertexCount - 1 )
    {
        // Move all the verts after the insert location, up a ways.
        memmove( &Vertices[nVertexPos + 1], &Vertices[nVertexPos], ((VertexCount-1) - nVertexPos) *  sizeof(CVertex) );

    } // End if not at end.
		   
    // Initialize data to default values
    Vertices[ nVertexPos ] = CVertex( 0.0f, 0.0f, 0.0f );

    // Return the position
    return nVertexPos;

}

//-----------------------------------------------------------------------------
// Name : GenerateFromPlane()
// Desc : Given a Plane and a Bounding Box, we calculate and store a polygon
//        which AT LEAST fills this bbox along that plane.
// Note : The poly could be MUCH larger than the bbox, but can never be smaller
//-----------------------------------------------------------------------------
bool CPolygon::GenerateFromPlane( const CPlane3& Plane, const CBounds3& Bounds )
{
    CVector3 CB, CP, U, V, A;

	// Calculate BBOX Centre Point
	CB = Bounds.GetCentre();
	// Calculate the Distance from the centre of the bounding box to the plane
    float DistanceToPlane = CB.DistanceToPlane( Plane );
	// Calculate Centre of Plane
	CP = CB + (Plane.Normal * -DistanceToPlane );

	// Calculate Major Axis Vector
	A = CVector3(0.0f,0.0f,0.0f);
	if( fabs(Plane.Normal.y) > fabs(Plane.Normal.z) ) {
		if( fabs(Plane.Normal.z)  < fabs(Plane.Normal.x) ) A.z = 1; else A.x = 1;
	} else {
		if (fabs(Plane.Normal.y) <= fabs(Plane.Normal.x) ) A.y = 1; else A.x = 1;
    } // End if

	// Generate U and V vectors
    U = A.Cross(Plane.Normal);
    V = U.Cross(Plane.Normal);
    U.Normalize(); V.Normalize();

	float Length = (Bounds.Max - CB).Length();

	// Scale the UV Vectors up by half the BBOX Length
	U *= Length; V *= Length;

	CVector3 P[4];
    P[0] = CP + U - V; // Bottom Right
    P[1] = CP + U + V; // Top Right
	P[2] = CP - U + V; // Top Left
	P[3] = CP - U - V; // Bottom Left

    // Allocate new vertices
    if (AddVertices( 4 ) < 0) return false;
	
	// Place vertices in poly
	for ( int i = 0; i < 4; i++) 
    {
        Vertices[i] = CVertex(P[i]);
        Vertices[i].Normal = Plane.Normal;
    
    } // Next vertex

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ReleaseVertices ()
// Desc : Use this function to release all the vertices and their associated
//        data / variables. This does not destroy the face itself.
//-----------------------------------------------------------------------------
void CPolygon::ReleaseVertices()
{
    // Clean up after ourselves
    if ( Vertices ) { delete []Vertices; Vertices = NULL; }
    VertexCount = 0;
}

//-----------------------------------------------------------------------------
// Name : Split ()
// Desc : This function splits the current face, against the plane. The two
//        split fragments are returned via the FrontSplit and BackSplit
//        pointers. These must be valid pointers to already allocated faces.
//        However you CAN pass NULL to either parameter, in this case no
//        vertices will be calculated for this polygon.
//-----------------------------------------------------------------------------
HRESULT CPolygon::Split(const CPlane3& Plane, CPolygon * FrontSplit, CPolygon * BackSplit, bool bReturnNoSplit /* = false */ )
{
    CVertex         *FrontList = NULL, *BackList = NULL;
	unsigned long   CurrentVertex = 0, i = 0;
    unsigned long   InFront = 0, Behind = 0, OnPlane = 0;
    unsigned long   FrontCounter = 0, BackCounter = 0;
    CLASSIFYTYPE   *PointLocation = NULL, Location;
    CVertex         NewVert;
    float           fDelta;

    // Bail if no fragments passed (No-Op).
    if (!FrontSplit && !BackSplit) return BC_OK;

    try 
    {
        // Allocate temp buffers for split operation
        PointLocation = new CLASSIFYTYPE[VertexCount];
        if (!PointLocation) throw std::bad_alloc(); // VC++ Compat
        
        if (FrontSplit) 
        {
            FrontList = new CVertex[VertexCount + 1];
            if (!FrontList) throw std::bad_alloc(); // VC++ Compat
        } // End If

        if ( BackSplit )
        {
            BackList = new CVertex[VertexCount + 1];
            if (!FrontList) throw std::bad_alloc(); // VC++ Compat
        } // End If

    } // End Try

    catch (...) 
    {
        // Catch any bad allocation exceptions
        if (FrontList)      delete []FrontList;
        if (BackList)       delete []BackList;
        if (PointLocation)  delete []PointLocation;
        return BCERR_OUTOFMEMORY;
    } // End Catch

    // Determine each points location relative to the plane.
	for ( i = 0; i < VertexCount; i++)	
    {
        // Retrieve classification
        Location = Plane.ClassifyPoint( Vertices[i] );

        // Classify the location of the point
		if (Location == CLASSIFY_INFRONT ) InFront++;
		else if (Location == CLASSIFY_BEHIND ) Behind++;
		else OnPlane++;
        
        // Store location
		PointLocation[i] = Location;

	} // Next Vertex

    // If there are no vertices in front of the plane
	if (!InFront) 
    {
        if ( bReturnNoSplit ) { delete []PointLocation; return true; }
        if ( BackList )
    {
		memcpy(BackList, Vertices, VertexCount * sizeof(CVertex));
		BackCounter = VertexCount;
        } // End if

    } // End if none in front

    // If there are no vertices behind the plane
	if (!Behind) 
    {
        if ( bReturnNoSplit ) { delete []PointLocation; return BC_OK; }
        if ( FrontList )
    {
		memcpy(FrontList, Vertices, VertexCount * sizeof(CVertex));
		FrontCounter = VertexCount;
        } // End if

    } // End if none behind

    // All were onplane
    if ( !InFront && !Behind && bReturnNoSplit ) { delete []PointLocation; return BC_OK; }

    // We can allocate the memory here if we want to return when no split occured
    if ( bReturnNoSplit )
    {
        try 
        {
            // Allocate these only if we need them
            if (FrontSplit) 
            {
                FrontList = new CVertex[VertexCount + 1];
                if (!FrontList) throw std::bad_alloc(); // VC++ Compat
            } // End If

            if ( BackSplit )
            {
                BackList = new CVertex[VertexCount + 1];
                if (!BackList) throw std::bad_alloc(); // VC++ Compat
            } // End If

        } // End Try

        catch (...) 
        {
            // Catch any bad allocation exceptions
            if (FrontList)      delete []FrontList;
            if (BackList)       delete []BackList;
            if (PointLocation)  delete []PointLocation;
            return BCERR_OUTOFMEMORY;
        } // End Catch

    } // End if we're returning if no split occured

    // Compute the split if there are verts both in front and behind
	if (InFront && Behind) 
    {
		for ( i = 0; i < VertexCount; i++) 
        {
			// Store Current vertex remembering to MOD with number of vertices.
			CurrentVertex = (i+1) % VertexCount;

			if (PointLocation[i] == CLASSIFY_ONPLANE ) 
            {
                if (FrontList) FrontList[FrontCounter++] = Vertices[i];
				if (BackList)  BackList [BackCounter ++] = Vertices[i];
				continue; // Skip to next vertex
            } // End if On Plane

			if (PointLocation[i] == CLASSIFY_INFRONT ) 
            {
				if (FrontList) FrontList[FrontCounter++] = Vertices[i];
			} 
            else 
            {
				if (BackList) BackList[BackCounter++] = Vertices[i];

            } // End if In front or otherwise
			
			// If the next vertex is not causing us to span the plane then continue
			if (PointLocation[CurrentVertex] == CLASSIFY_ONPLANE || PointLocation[CurrentVertex] == PointLocation[i]) continue;
			
			// Calculate the intersection point
            Plane.GetRayIntersect( Vertices[i], Vertices[CurrentVertex], NewVert, &fDelta );
			
            // Interpolate Texture Coordinates
            CVector3 Delta;
            Delta.x    = Vertices[CurrentVertex].tu - Vertices[i].tu;
			Delta.y    = Vertices[CurrentVertex].tv - Vertices[i].tv;
			NewVert.tu = Vertices[i].tu + ( Delta.x * fDelta );
			NewVert.tv = Vertices[i].tv + ( Delta.y * fDelta );

            // Interpolate normal
            Delta          = Vertices[CurrentVertex].Normal - Vertices[i].Normal;
            NewVert.Normal = Vertices[i].Normal + (Delta * fDelta);
            NewVert.Normal.Normalize();

            // Store in both lists.
			if (BackList)  BackList[BackCounter++]   = NewVert;			
			if (FrontList) FrontList[FrontCounter++] = NewVert;

        } // Next Vertex

    } // End if spanning

    // Allocate front face
    if (FrontCounter && FrontSplit) 
    {
        // Copy over the vertices into the new poly
        FrontSplit->AddVertices( FrontCounter );
        memcpy(FrontSplit->Vertices, FrontList, FrontCounter * sizeof(CVertex));

    } // End If

    // Allocate back face
    if (BackCounter && BackSplit) 
    {
        // Copy over the vertices into the new poly
        BackSplit->AddVertices( BackCounter );
        memcpy(BackSplit->Vertices, BackList, BackCounter * sizeof(CVertex));

    } // End If

    // Clean up
    if (FrontList)      delete []FrontList;
    if (BackList)       delete []BackList;
    if (PointLocation)  delete []PointLocation;

    // Success!!
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Desc : CFace member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CFace () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CFace::CFace()
{
	// Initialise anything we need
	MaterialIndex = -1;
    TextureIndex  = -1;
    ShaderIndex   = -1;
    Flags         =  0;
    SrcBlendMode  =  0;
    DestBlendMode =  0;
}

//-----------------------------------------------------------------------------
// Name : Split ()
// Desc : This function splits the current face, against the plane. The two
//        split fragments are returned via the FrontSplit and BackSplit
//        pointers. These must be valid pointers to already allocated faces.
//        However you CAN pass NULL to either parameter, in this case no
//        vertices will be calculated for that fragment.
//-----------------------------------------------------------------------------
HRESULT CFace::Split( const CPlane3& Plane, CFace * FrontSplit, CFace * BackSplit, bool bReturnNoSplit /* = false */ )
{
    // Call base class implementation
    HRESULT ErrCode = CPolygon::Split( Plane, FrontSplit, BackSplit, bReturnNoSplit );
    if (FAILED(ErrCode)) return ErrCode;

    // Copy remaining values
    if (FrontSplit) 
    {
	    FrontSplit->Normal        = Normal;
        FrontSplit->MaterialIndex = MaterialIndex;
        FrontSplit->TextureIndex  = TextureIndex;
        FrontSplit->ShaderIndex   = ShaderIndex;
        FrontSplit->Flags         = Flags;
        FrontSplit->SrcBlendMode  = SrcBlendMode;
        FrontSplit->DestBlendMode = DestBlendMode;

    } // End If

    if (BackSplit) 
    {
        BackSplit->Normal         = Normal;
        BackSplit->MaterialIndex  = MaterialIndex;
        BackSplit->TextureIndex   = TextureIndex;
        BackSplit->ShaderIndex    = ShaderIndex;
        BackSplit->Flags          = Flags;
        BackSplit->SrcBlendMode   = SrcBlendMode;
        BackSplit->DestBlendMode  = DestBlendMode;
    } // End If

    // Success
    return BC_OK;
}

//-----------------------------------------------------------------------------
// Name : GenerateFromPlane()
// Desc : Given a Plane and a Bounding Box, we calculate and store a polygon
//        which AT LEAST fills this bbox along that plane.
// Note : The poly could be MUCH larger than the bbox, but can never be smaller
//-----------------------------------------------------------------------------
bool CFace::GenerateFromPlane( const CPlane3& Plane, const CBounds3& Bounds )
{
    // Call base class implementation
    if (!CPolygon::GenerateFromPlane( Plane, Bounds )) return false;

    // Copy remaining values
    Normal = Plane.Normal;

    // Success
    return true;
}
