#ifndef _COMPILERTYPES_H_
#define _COMPILERTYPES_H_

#include <cstdint>

//-----------------------------------------------------------------------------
// CBSPTree Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Support Source\\Common.h"
#include "..\\Support Source\\CVector.h"
#include "..\\Support Source\\CMatrix.h"
#include "..\\Support Source\\CBounds.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CBSPTree;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define ESR_POS_CUSTOM      0
#define ESR_POS_SPAWNPOINT  1
#define ESR_POS_AUTOMATIC   2

#define BSP_TYPE_NONSPLIT   0
#define BSP_TYPE_SPLIT      1

#define FRONT_OWNER         0
#define BACK_OWNER          1
#define NO_OWNER            2

//-----------------------------------------------------------------------------
// Typedefs, Structures and Enumerators
//-----------------------------------------------------------------------------
typedef struct _HSROPTIONS {            // Hidden Surface Removal Options
    bool            Enabled;            // Process Enabled ?
} HSROPTIONS;

typedef struct _BSPOPTIONS {            // BSP Compilation Options
    bool            Enabled;            // Process Enabled ?
    unsigned long   TreeType;           // What type of tree to compile ?
    float           SplitHeuristic;     // Split vs Balance Importance
    unsigned long   SplitterSample;     // Number of splitters to sample
    bool            RemoveBackLeaves;   // Remove illegal back leaves
    bool            AddBoundingPolys;   // Add inverted scene-bounding polgons
} BSPOPTIONS;

typedef struct _PRTOPTIONS {            // Portal Compilation Options
    bool            Enabled;            // Process Enabled ?
} PRTOPTIONS;

typedef struct _PVSOPTIONS {            // PVS Compilation Options
    bool            Enabled;            // Process Enabled ?
    bool            FullCompile;        // Perform Full PVS Compile
    unsigned char   ClipTestCount;      // Number of portal clip tests to perform
} PVSOPTIONS;

typedef struct _TJROPTIONS {            // T-Junction Repair Options
    bool            Enabled;            // Process Enabled ?
} TJROPTIONS;

typedef struct _LIGTHMAPOPTIONS {
	bool Enabled;
	unsigned int lightmap_res_factor;
	unsigned int clustermap_lightmap_factor;
	unsigned int lm_width, lm_height;
	unsigned int sampleCount;
	float sampleDistanceFactor;
} LIGHTMAPOPTIONS;

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CPlane3;
class CBounds3;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CVertex (Class)
// Desc : Primary vertex class, derived from CVector3 to allow vector / matrix
//        ops to be performed seamlessly on them.
//-----------------------------------------------------------------------------
class CVertex : public CVector3
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    CVertex( ) { x = y = z = tu = tv = 0.0f; }
	CVertex( float _x, float _y, float _z ) { x = _x; y = _y; z = _z; tu = tv = 0.0; }
    CVertex( const CVector3& vec ) { x = vec.x; y = vec.y; z = vec.z; tu = tv = 0.0f; }
    CVertex( float _x, float _y, float _z, float _tu, float _tv ) { x = _x; y = _y; z = _z; tu = _tu; tv = _tv; }
    CVertex( float _x, float _y, float _z, const CVector3& _Normal, float _tu, float _tv ) { x = _x; y = _y; z = _z; Normal = _Normal; tu = _tu; tv = _tv; }
    
    //-------------------------------------------------------------------------
	// Public Variables For This Class
	//-------------------------------------------------------------------------
    // X/Y/Z components inherited
    CVector3    Normal;                     // Vertex Normal
	float       tu;				            // Vertex U texture coordinate
	float       tv;				            // Vertex V texture coordinate
	float       lu;    // lightmap texture coordinates
	float       lv;
};

//-----------------------------------------------------------------------------
// Name : CPolygon (Class)
// Desc : Simple base polygon class, contains basic polygon routines but only
//        core member variables. This allows us to derive other classes such
//        as different types of portals which share much of the same
//        functionality, but do not require materials, textures and so on.
//-----------------------------------------------------------------------------
class CPolygon
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             CPolygon( );
    virtual ~CPolygon( );

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
	CVertex		   *Vertices;				// Polygon vertices
	unsigned long   VertexCount;			// Vertices in this poly

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
	long            AddVertices( unsigned long nVertexCount = 1 );
    long            InsertVertex( unsigned long nVertexPos );
    void            ReleaseVertices();

    //-------------------------------------------------------------------------
	// Public Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual HRESULT Split( const CPlane3& Plane, CPolygon * FrontSplit, CPolygon * BackSplit, bool bReturnNoSplit = false );
    virtual bool    GenerateFromPlane( const CPlane3& Plane, const CBounds3& Bounds );

};

//-----------------------------------------------------------------------------
// Name : CFace (Class)
// Desc : Primary Face class, stores all the face specific data including
//        vertices and any other required data.
//-----------------------------------------------------------------------------
class CFace : public CPolygon
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    CFace( );

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    CVector3        Normal;                 // Face Normal
    short           TextureIndex;           // Index into texture look up
    short           MaterialIndex;          // Index into material look up
    short           ShaderIndex;            // Index into the shader look up
    ULONG           Flags;                  // Face Flags
    UCHAR           SrcBlendMode;           // Face Source Blend Mode
    UCHAR           DestBlendMode;          // Face Dest Blend Mode

    //-------------------------------------------------------------------------
	// Public Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual HRESULT Split( const CPlane3& Plane, CFace * FrontSplit, CFace * BackSplit, bool bReturnNoSplit = false );
    virtual bool    GenerateFromPlane( const CPlane3& Plane, const CBounds3& Bounds );

};

//-----------------------------------------------------------------------------
// Name : CMesh (Class)
// Desc : Primary Mesh class, stores all the mesh specific data including
//        faces and any other required data.
//-----------------------------------------------------------------------------
class CMesh
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             CMesh( );
    virtual ~CMesh( );

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    char           *Name;                   // Stored name loaded from level file
	CFace         **Faces;				    // Mesh faces
	unsigned long   FaceCount;			    // Faces in this poly
    CMatrix4        Matrix;                 // Meshes object matrix
    CBounds3        Bounds;                 // Meshes local space bounding box
    unsigned long   Flags;                  // Mesh flags (i.e. detail object)

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
	long            AddFaces( unsigned long nFaceCount = 1 );
    void            ReleaseFaces();
    bool            BuildFromBSPTree( const CBSPTree * pTree, bool Reset = false );
    const CBounds3& CalculateBoundingBox();

};

#endif // _COMPILERTYPES_H_