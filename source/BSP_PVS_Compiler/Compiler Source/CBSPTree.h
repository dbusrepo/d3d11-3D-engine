#ifndef _CBSPTREE_H_
#define _CBSPTREE_H_

//-----------------------------------------------------------------------------
// CBSPTree Specific Includes
//-----------------------------------------------------------------------------
#include <vector>
#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"
#include "..\\Support Source\\CBounds.h"
#include "..\\Support Source\\CCollision.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CPlane3;
class CCompiler;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define BSP_ARRAY_THRESHOLD     100
#define BSP_SOLID_LEAF          0x80000000

// Global Definitions
double SquaredDistPointAABB(const CVector3 & p, const CBounds3 & aabb);

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBSPFace (Class)
// Desc : Derived polygon class designed solely for BSP Compilation purposes
//-----------------------------------------------------------------------------
class CBSPFace : public CFace
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//------------------------------------------------------------
    CBSPFace( );
    CBSPFace( const CFace * pFace );
    
    //------------------------------------------------------------
	// Public Variables for This Class
	//------------------------------------------------------------
    bool        UsedAsSplitter;         // Polygon used as splitter
    long        OriginalIndex;          // Original polygon index
    bool        Deleted;                // Face has been 'Virtually' deleted
    long        ChildSplit[2];          // During CSG, records the resulting 2 children
    CBSPFace   *Next;                   // Next poly in linked list
    long        Plane;                  // The plane to which this face is linked

	bool	    RayIntersect(const XMFLOAT3 &rayOrigin, const XMFLOAT3 &rayDir);
    
	//-------------------------------------------------------------------------
	// Public Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual HRESULT Split( const CPlane3& Plane, CBSPFace * FrontSplit, CBSPFace * BackSplit, bool bReturnNoSplit = false );
};

//-----------------------------------------------------------------------------
// Name : CBSPLeaf (Class)
// Desc : Leaf class, designed solely for BSP Compilation purposes
//-----------------------------------------------------------------------------
class CBSPLeaf
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             CBSPLeaf( );
    virtual ~CBSPLeaf( );

    //-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    bool            BuildFaceIndices( CBSPFace * pFaceList );
    bool            AddPortal( unsigned long PortalIndex );
	bool			RayIntersection(CBSPTree *pTree, const XMFLOAT3 &rayOrigin, const XMFLOAT3 &rayDir);

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    std::vector<long>   FaceIndices;    // Indices to faces in this leaf
    std::vector<long>   PortalIndices;  // Indices to portals in this leaf
    unsigned long       PVSIndex;       // Index into the master PVS array
    CBounds3            Bounds;         // Leaf Bounding Box
	//long lightsIndicesArr[16];
	uint8_t numLights = 0;
	uint16_t* lightClusters = nullptr;
};

//-----------------------------------------------------------------------------
// Name : CBSPNode (Class)
// Desc : Node class, designed solely for BSP Compilation purposes
//-----------------------------------------------------------------------------
class CBSPNode
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             CBSPNode( );
    virtual ~CBSPNode( ) {};

    //------------------------------------------------------------
	// Public Functions for This Class
	//------------------------------------------------------------
    void            CalculateBounds( CBSPFace * pFaceList, bool ResetBounds = true );

    //------------------------------------------------------------
	// Public Variables for This Class
	//------------------------------------------------------------
    long            Plane;              // Index to the plane for this node
    long            Front;              // Child in front of this node
    long            Back;               // Child behind this node
    CBounds3        Bounds;             // Bounds of everything under this node

};

//-----------------------------------------------------------------------------
// Name : CBSPPortal (Class)
// Desc : Derived portal class for... portal compilation ;)
//-----------------------------------------------------------------------------
class CBSPPortal : public CPolygon
{
public:
    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CBSPPortal( );

    //------------------------------------------------------------
	// Public Variables for This Class
	//------------------------------------------------------------
    CBSPPortal     *NextPortal;         // Linked List Next Portal Item
    unsigned char   LeafCount;          // Used to determine how many leaves this portal fell into
    unsigned long   OwnerNode;          // Node that created this portal
    unsigned long   LeafOwner[2];       // Front / Back Leaf Owner Array

    //------------------------------------------------------------
	// Virtual Public Functions for This Class
	//------------------------------------------------------------
    virtual HRESULT Split( const CPlane3& Plane, CBSPPortal * FrontSplit, CBSPPortal * BackSplit );

};

//-----------------------------------------------------------------------------
// Typedefs utilised for shorthand versions of our STL types.
//-----------------------------------------------------------------------------
typedef std::vector<CBSPNode*>   vectorNode;
typedef std::vector<CPlane3*>    vectorPlane;
typedef std::vector<CBSPLeaf*>   vectorLeaf;
typedef std::vector<CBSPFace*>   vectorBSPFace;
typedef std::vector<CBSPPortal*> vectorBSPPortal;

//-----------------------------------------------------------------------------
// Name : CBSPTree (Class)
// Desc : 'Binary Space Partition Tree' compiler class. This class accepts a
//        set of any number of arbitrary, non-intersecting, convex polygons
//        from which it will build the tree.
//-----------------------------------------------------------------------------
class CBSPTree
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CBSPTree();
    virtual ~CBSPTree();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         CompileTree( );
    HRESULT         AddFaces( CFace ** ppFaces, unsigned long lFaceCount = 1 );
    HRESULT         AddBoundingPolys( bool BackupOriginals = false );
    void            ReleaseTree( );

    const CBounds3& GetBounds( ) const { return m_Bounds; }

    unsigned long   GetNodeCount  ( ) const { return m_vpNodes.size()  ; }
    unsigned long   GetLeafCount  ( ) const { return m_vpLeaves.size() ; }
    unsigned long   GetPlaneCount ( ) const { return m_vpPlanes.size() ; }
    unsigned long   GetFaceCount  ( ) const { return m_vpFaces.size()  ; }
    unsigned long   GetPortalCount( ) const { return m_vpPortals.size(); }

    CBSPNode       *GetNode  ( unsigned long Index ) const { return (Index < m_vpNodes.size())   ? m_vpNodes[Index]   : NULL; }
    CPlane3        *GetPlane ( unsigned long Index ) const { return (Index < m_vpPlanes.size())  ? m_vpPlanes[Index]  : NULL; }
    CBSPLeaf       *GetLeaf  ( unsigned long Index ) const { return (Index < m_vpLeaves.size())  ? m_vpLeaves[Index]  : NULL; }
    CBSPFace       *GetFace  ( unsigned long Index ) const { return (Index < m_vpFaces.size())   ? m_vpFaces[Index]   : NULL; }
    CBSPPortal     *GetPortal( unsigned long Index ) const { return (Index < m_vpPortals.size()) ? m_vpPortals[Index] : NULL; }

    void            SetNode   ( unsigned long Index, CBSPNode   * pNode   ) { if (Index < m_vpNodes.size())   m_vpNodes[Index]   = pNode  ; }
    void            SetPlane  ( unsigned long Index, CPlane3    * pPlane  ) { if (Index < m_vpPlanes.size())  m_vpPlanes[Index]  = pPlane ; }
    void            SetLeaf   ( unsigned long Index, CBSPLeaf   * pLeaf   ) { if (Index < m_vpLeaves.size())  m_vpLeaves[Index]  = pLeaf  ; }
    void            SetFace   ( unsigned long Index, CBSPFace   * pFace   ) { if (Index < m_vpFaces.size())   m_vpFaces[Index]   = pFace  ; }
    void            SetPortal ( unsigned long Index, CBSPPortal * pPortal ) { if (Index < m_vpPortals.size()) m_vpPortals[Index] = pPortal; }
    
    HRESULT         SetPVSData( UCHAR PVSData[], unsigned long PVSSize, bool PVSCompressed );

    void            SetOptions( const BSPOPTIONS& Options ) { m_OptionSet = Options; }
    void            SetLogger ( ILogger * pLogger )         { m_pLogger = pLogger; }
    void            SetParent ( CCompiler * pParent )       { m_pParent = pParent; }

    bool            IncreaseFaceCount();
    bool            IncreaseNodeCount();
    bool            IncreaseLeafCount();
    bool            IncreasePlaneCount();
    bool            IncreasePortalCount();

    HRESULT         ClipTree( CBSPTree * pTree, bool ClipSolid, bool RemoveCoPlanar, ULONG CurrentNode = 0, CBSPFace * pFaceList = NULL );
    void            RepairSplits( );

    long            FindLeaf( const CVector3& Position ) const;
	std::vector<unsigned long> FindPVSLeafIndices(const CBSPLeaf *pCurrentLeaf) const;

    bool            IntersectedByTree  ( const CBSPTree * pTree ) const;
    bool            IntersectedByFace  ( const CFace * pFace, ULONG Node = 0 ) const;

	bool			RayIntersect(const CVector3 &p, const CVector3 &dir);
	
    //-------------------------------------------------------------------------
    // Public Static Functions for This Class.
    //-------------------------------------------------------------------------
    static CBSPFace   *AllocBSPFace  ( const CFace * pDuplicate = NULL );
    static CBSPPortal *AllocBSPPortal( );

    //-------------------------------------------------------------------------
    // Public Variables for This Class.
    //-------------------------------------------------------------------------
    UCHAR          *m_pPVSData;             // PVS Data set (array)
    unsigned long   m_lPVSDataSize;         // Size of the PVS data set
    bool            m_bPVSCompressed;       // Is the PVS data compressed

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    HRESULT         BuildPlaneArray( );
    HRESULT         BuildBSPTree( unsigned long lNode, CBSPFace * pFaceList );
    HRESULT         ProcessLeafFaces( CBSPLeaf * pLeaf, CBSPFace * pFaceList );
    CBSPFace       *SelectBestSplitter( CBSPFace * pFaceList, unsigned long SplitterSample, float SplitHeuristic );
    
    unsigned long   CountSplitters( CBSPFace * pFaceList ) const;
    void            FreeFaceList( CBSPFace * pFaceList );
    void            TrashFaceList( CBSPFace * pFaceList );

	bool			RayIntersectRecurse(long iNode, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir);

    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    CBSPFace       *m_pFaceList;        // Linked List Head for pre-compiled faces.
    unsigned long   m_lActiveFaces;     // Number of active faces in the pre-compiled list    
    vectorNode      m_vpNodes;          // Nodes created by the BSP compiler
    vectorPlane     m_vpPlanes;         // Node planes created by the BSP compiler
    vectorLeaf      m_vpLeaves;         // Leaves created by the BSP compiler
    vectorBSPFace   m_vpFaces;          // Resulting faces. Either the originals or split versions.
    vectorBSPPortal m_vpPortals;        // A set of portals built by the CProcessPRT compiler.
    vectorBSPFace   m_vpGarbage;        // Garabage collection for releasing faces on error.
    CBounds3        m_Bounds;           // BSP Trees Bounding Box
    
    BSPOPTIONS      m_OptionSet;        // The option set for BSP Compilation.
    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    CCompiler      *m_pParent;          // Parent Compiler Pointer

};

#endif // _CBSPTREE_H_