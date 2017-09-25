#pragma once

#include <vector>
#include <list>
#include <map>
#include "Common.h"


//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define BSP_SOLID_LEAF      0x80000000
#define MESH_DETAIL         0x1

#define MAX_LIGHTS_PER_LEAF 16
//#define PRINT_VISIBILITY_INFO
//#define PRINT_OCCLUSION_INFO

namespace Library
{
	class D3DApp;
	//class SkyboxComponent;
	class Timer;

	namespace BSPEngine
	{
		class Camera;
		class Door;

		//class SceneMaterial;
		class SceneMaterial;
		class DoorMaterial;
		class BasicMaterial;
		class DepthPassMaterial;

		//-----------------------------------------------------------------------------
		// Forward Declarations
		//-----------------------------------------------------------------------------
		class BSPTree;
		class BSPTreeNode;
		class BSPTreePortal;
		class BSPTreeLeaf;
		class Polygon;
		class Vertex;

		typedef std::vector<Polygon*> PolygonVector;
		typedef std::vector<BSPTreePortal*> PortalVector;

		//-----------------------------------------------------------------------------
		// Main Class Definitions
		//-----------------------------------------------------------------------------

		class Light {
		public:
			UINT index; // 0..NumLights-1, used from shader leaf lights to reference lightmaps (correspondent to lights 0..numLights-1)
			float origin[3];
			float radius;
			/*
			struct LightInLeafInfo {
				uint16_t leafIdx;
				uint8_t lightClusterIndex;
			};
			//std::vector<LightInLeafInfo> aleaves;
			*/
			std::map<uint16_t, uint8_t> m_lightInLeafClusterIndex;
			float  intensity;
			XMFLOAT3 color;
			bool isActive = true;
			bool isAnimated = false;
			float  frequency;
		};

		//-----------------------------------------------------------------------------
		// Name : LeafBinData (Class)
		// Desc : This class stores the leaf bin data for one texture used for a single material/shader.
		//-----------------------------------------------------------------------------
		class LeafBinData
		{
		public:
			//-------------------------------------------------------------------------
			// Public Structures, Enumerators and typedefs for This Class.
			//-------------------------------------------------------------------------
			struct RenderBatch
			{
				size_t IndexStart;
				size_t PrimitiveCount;
			};

			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			LeafBinData();
			virtual ~LeafBinData();

			void addBatch(size_t IndexStart, size_t PrimitiveCount);
			void coalesceBatches();

			void clearBatches();
			RenderBatch &getRenderBatch(size_t i);
			size_t getRenderBatchCount();
			void reserveBatches(size_t numBatches);
		
		protected:
			std::vector<RenderBatch> m_renderBatches;
		};

		class LeafBinDataTexture : public LeafBinData {
		public:
			LeafBinDataTexture(uint16_t texID, ID3D11ShaderResourceView* TextureView, ID3D11ShaderResourceView * TextureNormalView);
			~LeafBinDataTexture();
			uint16_t getTexID() { return mTexID; }
			ID3D11ShaderResourceView* getTextureView() { return mTextureView; }
			ID3D11ShaderResourceView* getTextureNormalView() { return mTextureNormalView; }
			LeafBinDataTexture *m_next; // linked list of leaf bin data
			size_t m_nVisCounter;
		protected:
			uint16_t mTexID;
			ID3D11ShaderResourceView* mTextureView; // diffuse maps texture array view
			ID3D11ShaderResourceView* mTextureNormalView; // normal maps texture array view
		};

		//-----------------------------------------------------------------------------
		// Name : LeafBin (Class)
		// Desc : This class stores the face data of each leaf for one material/shader
		//-----------------------------------------------------------------------------
		class LeafBin
		{
			friend class BSPTree;
		public:
			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			LeafBin(BSPTree *pBspTree, size_t materialID, size_t nLeafCount);
			virtual ~LeafBin();
			virtual bool AddLeafBinData(LeafBinData * pData) = 0;
			virtual void CreateVertexBuffer(ID3D11Device *D3DDevice, const byte *pMem, size_t numBytes) = 0;
			virtual void ResetBinData() = 0;
			
			//-------------------------------------------------------------------------
			// Public Functions for This Class.
			//-------------------------------------------------------------------------
			size_t   GetMaterialID() const;
			
			void CreateIndexBuffer(ID3D11Device *D3DDevice, const ULONG *indices, size_t numIndices, bool use32bitIndices);
			void AddVisibleData(LeafBinData * pData, size_t IndexStart, size_t PrimitiveCount);

		protected:			
			//-------------------------------------------------------------------------
			// Private Variables for This Class.
			//-------------------------------------------------------------------------
			size_t  m_MaterialID;     // The material ID of the faces stored here
			ID3D11Buffer *m_pVertexBuffer;        // Reference to the vertex buffer used for rendering.
			ID3D11Buffer *m_pIndexBuffer;         // The index buffer used for rendering
	
			size_t m_nFaceCount;    // The total number of primitives in the index array.
			bool m_b32BitIndices;   // We're using 32 bit indices?
			size_t m_nLeafCount;    // Total number of leaves in the tree
					
			BSPTree *m_pBspTree;

			//ID3D11ShaderResourceView *mLightMapsArrayView;
			//ID3D11ShaderResourceView *mClusterMapsArrayView;
		};

		class LeafBinTexture : public LeafBin
		{
			friend class BSPTree;
		public:
			LeafBinTexture(BSPTree *pBspTree, size_t materialID, SceneMaterial *material, size_t nLeafCount);
			~LeafBinTexture();

			virtual void ResetBinData() override;
			virtual void CreateVertexBuffer(ID3D11Device *D3DDevice, const byte *pMem, size_t numBytes) override;
			virtual bool AddLeafBinData(LeafBinData * pData) override;

			void Render(ID3D11DeviceContext* pDeviceContext, CXMMATRIX wvpMat);
			LeafBinDataTexture  * GetBinData(uint16_t texID) const;
			void AddVisibleDataTexture(LeafBinDataTexture * pData, size_t IndexStart, size_t PrimitiveCount, size_t visCounter);
			
		protected:			
			//void AddVisibleData(LeafBinData * pData, size_t IndexStart, size_t PrimitiveCount);

			SceneMaterial *mMaterial;
			std::map<uint16_t, LeafBinDataTexture *> m_LeafBinData; // all the information we need to render (one per texture array for LeafBin material).
			LeafBinDataTexture *m_pLeafBinDataTextureList; // also linked list for easy access
			LeafBinDataTexture **m_pLeafBinDataTexListPtr;
		};

		class LeafBinEntity : public LeafBin
		{
			friend class BSPTree;
			friend class BrushEntity;
		public:
			LeafBinEntity(BSPTree *pBspTree, size_t materialID, DoorMaterial *material, size_t nSize);
			~LeafBinEntity();

			virtual void ResetBinData() override;
			virtual void CreateVertexBuffer(ID3D11Device *D3DDevice, const byte *pMem, size_t numBytes) override;
			virtual bool AddLeafBinData(LeafBinData * pData) override;

			void Render(ID3D11DeviceContext* pDeviceContext, CXMMATRIX wvpMat);
			LeafBinDataTexture  * GetBinData(uint16_t texID) const;
			 
		protected:
			//void AddVisibleData(LeafBinData * pData, size_t IndexStart, size_t PrimitiveCount);

			DoorMaterial *mMaterial;
			std::map<uint16_t, LeafBinDataTexture *> m_LeafBinData; // all the information we need to render (one per texture array for LeafBin material).
		};

		class LeafBinPortal : public LeafBin
		{
			friend class BSPTree;
		public:
			LeafBinPortal(BSPTree *pBspTree, BasicMaterial *material, size_t nLeafCount);
			~LeafBinPortal();

			virtual void ResetBinData() override;
			virtual void CreateVertexBuffer(ID3D11Device *D3DDevice, const byte *pMem, size_t numBytes) override;
			virtual bool AddLeafBinData(LeafBinData * pData) override;
			
			void Render(ID3D11DeviceContext* pDeviceContext, CXMMATRIX tMat, CXMVECTOR color);
			void AddVisibleData(size_t IndexStart, size_t PrimitiveCount);

		protected:
			BasicMaterial *mMaterial;
			LeafBinData m_LeafBinData;
		};

	
		//-----------------------------------------------------------------------------
		// Name : BSPTreePortal (Class)
		// Desc : Portal for the BSP spatial partitioning tree.
		//-----------------------------------------------------------------------------

		class BSPTreePortal {
		public:
			BSPTreePortal();
			~BSPTreePortal();

			uint32_t mOwnerNode;
			uint32_t mFrontOwner;
			uint32_t mBackOwner;
			Polygon *mPolygon;
		};

		//-----------------------------------------------------------------------------
		// Name : BSPTreeLeaf (Class)
		// Desc : Leaf for the BSP spatial partitioning tree.
		//-----------------------------------------------------------------------------
		class BSPTreeLeaf {
			friend class BSPTree;
			friend class LeafBinDepthPass;
			friend class Collision;
		public:
			//-------------------------------------------------------------------------
			// Public Structures, Enumerators and typedefs for This Class.
			//-------------------------------------------------------------------------
			struct RenderData
			{
				struct Element
				{
					size_t   IndexStart;         // The index at which this leaf's face data starts
					size_t   PrimitiveCount;     // The number of primitives used by this leaf
					uint16_t   texID;     // 
					LeafBinDataTexture *pLeafBinData = nullptr;
				};

				size_t materialID;              // The attribute ID that this render data item maps to
				LeafBinTexture * pLeafBin = nullptr;                 // Pointer to the actual leaf bin
				std::vector<Element> elements;
			};


			BSPTreeLeaf(BSPTree * pTree);
			~BSPTreeLeaf();

			bool IsVisible() const;
			void SetVisible();
			void AddRenderBatches();

			size_t   GetPolygonCount() const;
			Polygon *      GetPolygon(size_t nIndex);
			void            GetBoundingBox(XMFLOAT3 & Min, XMFLOAT3 & Max) const;
			void     SetIndex(size_t index) { m_index = index; }
			size_t   GetPortalCount() const;
			BSPTreePortal *GetPortal(size_t nIndex);

			void                    SetBoundingBox(const XMFLOAT3 & Min, const XMFLOAT3 & Max);
			void                    AddPolygon(Polygon * pPolygon);
			void				    AddPortal(BSPTreePortal * pPortal);
			RenderData            * AddRenderData(size_t materialID);
			RenderData::Element   * AddRenderDataElement(size_t materialID);
			RenderData            * GetRenderData(size_t materialID);
			void UpdateShaderLeafData(ID3D11DeviceContext* pDeviceContext);
			//bool IsOccluded() const { return m_isOccluded; }
			//void setOccluded(bool occluded) { m_isOccluded = occluded; }
			//ID3D11Query * m_pQuery;
			void addDoor(size_t doorIdx);
			void removeDoor(size_t doorIdx);
			BSPTree *getTree() { return m_pTree; }
			
		protected:
			size_t           m_nVisCounter;      // The counter at which this was flagged as visible
			size_t           m_nPVSIndex;        // The index at which this leaf's visibility set begins
			BSPTreeLeaf    *m_nextVisible;
			//BSPTreeLeaf    *m_nextOcclusion;
			size_t			m_index; // leaf index in leaves bsp array
			char m_LastFrustumPlane;   // The frame-to-frame coherence 'last plane' index.
			//bool m_isOccluded;
			PolygonVector m_Polygons;         // Array of polygon pointers considered part of this leaf.
			PortalVector  m_Portals;          // Array of portal polygon pointers of this leaf.
			size_t m_portalIndexStart;       // used for rendering portals
			size_t m_portalPrimitiveCount;   // used for rendering portals
			XMFLOAT3 m_vecBoundsMin;     // Minimum bounding box extents
			XMFLOAT3 m_vecBoundsMax;     // Maximum bounding box extents.
			std::vector<RenderData> m_renderData;
			BSPTree *m_pTree;            // The tree to which this leaf belongs.
			BSPTreeNode *m_pParentNode;  // Parent node in BSPtree
			Light *m_lights[16] = { nullptr }; // indices in bsptree lights vec
			uint16_t m_activeLightsMask = 0;
			bool m_updateShaderLeafData = true;
			std::set<size_t> m_doorsIndices;
		};

		//-----------------------------------------------------------------------------
		// Name : Level Brush Entity (Class)
		// Desc : 
		//-----------------------------------------------------------------------------

		class BrushEntity {
		public:
			
			struct RenderData
			{
				struct Element
				{
					size_t   IndexStart;         // The index at which this leaf's face data starts
					size_t   PrimitiveCount;     // The number of primitives used by this leaf
					uint16_t   texID;     // arr tex				
				};

				size_t materialID;              // The attribute ID that this render data item maps to
				LeafBinEntity * pLeafBin = nullptr; // Pointer to the actual leaf bin
				std::vector<Element> elements;
			};
		
			BrushEntity();
			virtual ~BrushEntity();
			virtual void Update(float timeScale) = 0;

			void SetIndex(size_t index) { m_index = index; }
			size_t GetIndex() { return m_index; }
			void Render(ID3D11DeviceContext * pDeviceContext, CXMMATRIX vpMat);
			size_t GetPolygonCount() { return m_pPolygons.size(); }
			Polygon *GetPolygon(size_t i) { return m_pPolygons[i]; }
			bool BuildRenderData(BSPTree *pBspTree, DoorMaterial *pMaterial);
			bool IsMarked() { return m_isMarked; }
			void SetMarked(bool marked) { m_isMarked = marked; }
			void SetBounds(const XMFLOAT3 &bmin, const XMFLOAT3 &bmax) {
				m_boundsMin = bmin;
				m_boundsMax = bmax;
			}
			void GetBoundingBox(XMFLOAT3 & Min, XMFLOAT3 & Max) const;
			void AddPolygon(Polygon *polygon) {
				m_pPolygons.push_back(polygon);
			}
			bool HasMoved() { return m_hasMoved; }
			const XMFLOAT4X4 &GetModel() { return m_model; }
			char *GetLastFrustumPlane() { return &m_LastFrustumPlane; }

		protected:
			size_t m_index;
			BSPTree *m_pBspTree = nullptr;
			PolygonVector m_pPolygons;
			std::map<size_t, LeafBinEntity*> m_leafBins; // The array of all of the "leaf bin" objects.
			std::vector<BSPTreeLeaf *> m_leafVector;
			bool m_isMarked;
			XMFLOAT4X4 m_model;
			XMFLOAT3  m_boundsMin;
			XMFLOAT3  m_boundsMax;
			std::vector<RenderData> m_renderData;
			bool m_hasMoved;	
			char m_LastFrustumPlane;
		};




		//-----------------------------------------------------------------------------
		// Name : BSPTreeNode (Class)
		// Desc : Node for the BSP spatial partitioning tree.
		//-----------------------------------------------------------------------------
		class BSPTreeNode
		{
		public:
			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			BSPTreeNode();
			~BSPTreeNode();

			//-------------------------------------------------------------------------
			// Public Functions for This Class
			//-------------------------------------------------------------------------

			//-------------------------------------------------------------------------
			// Public Variables for This Class
			//-------------------------------------------------------------------------
			XMFLOAT4       Plane;              // Splitting plane for this node
			BSPTreeNode *  Front;              // Node in front of the plane
			BSPTreeNode *  Back;               // Node behind the plane
			BSPTreeNode *  Parent;			   // Parent Node in the tree
			BSPTreeLeaf *  Leaf;               // If this is a leaf, store here.
			XMFLOAT3     BoundsMin;          // Minimum bounding box extents
			XMFLOAT3     BoundsMax;          // Maximum bounding box extents
			signed char     LastFrustumPlane;   // The frame-to-frame coherence 'last plane' index.
			size_t           m_nVisCounter;      
		};

		//-----------------------------------------------------------------------------
		// Name : BSPTree (Class)
		// Desc : The implementation of a renderable, binary partitioning KD tree.
		//-----------------------------------------------------------------------------
		

		class TextureResources;

		class BSPTree
		{
			friend class Scene;
			friend class BSPTreeLeaf;
			friend class BrushEntity;
			friend class LeafBinEntity;
			friend class LeafBinTexture;

			
		public:
			typedef std::vector<BSPTreeLeaf*> LeafVector;
			typedef std::vector<Door*> DoorVector;

			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			BSPTree(D3DApp *app, const char *FileName);
			~BSPTree();
				
			bool	Load(FILE *file);
			bool    Build(TextureResources *textureResources);		
			void    Initialize(Camera *pCamera);
			void	Render(const Timer &timer);
			
			bool    CollectLeavesAABB(LeafVector & List, const XMFLOAT3 & Min, const XMFLOAT3 & Max);
			bool    CollectLeavesRay(LeafVector & List, const XMFLOAT3 & RayOrigin, const XMFLOAT3 & Velocity);
			//void    DebugDraw(Camera & Camera);
			bool    GetSceneBounds(XMFLOAT3 & Min, XMFLOAT3 & Max);

			//-------------------------------------------------------------------------
			// Public Functions for This Class
			//-------------------------------------------------------------------------
			size_t           GetVisCounter() const { return m_nVisCounter; }

			struct bspFileNode
			{
				long     PlaneIndex;
				XMFLOAT3 BoundsMin;
				XMFLOAT3 BoundsMax;
				long     FrontIndex;
				long     BackIndex;
			};
			
			void				AddLight(Light *light);
			void			    SetClusterMapSize(XMFLOAT2 size);
			void                AddPolygon(Polygon * pPolygon);
			void				AddDoor(Door *pDoor);
			Door			   *getDoor(size_t i) { return m_doors[i]; }
			bool                Repair();
			PolygonVector      &GetPolygonList();
			LeafVector         &GetLeafList();
			LeafVector         &GetVisibleLeafList();
			
			LeafBinTexture * GetLeafBin(size_t materialID);
			void             AddLeaf(BSPTreeLeaf * pLeaf);
			void			 AddPortal(BSPTreePortal * pPortal);
			void             AddVisibleLeaf(BSPTreeLeaf * pLeaf);
			bool             BuildRenderData();
			
			//bool            DebugDrawRecurse(BSPTreeNode * pNode, Camera & Camera, bool bRenderInLeaf);
			bool    CollectAABBRecurse(BSPTreeNode * pNode, LeafVector & List, const XMFLOAT3 & Min, const XMFLOAT3 & Max, bool bAutoCollect = false);
			bool    CollectRayRecurse(BSPTreeNode * pNode, LeafVector & List, const XMFLOAT3 & RayOrigin, const XMFLOAT3 & Velocity);
			void    SortLeavesFrontToBackRecurse(BSPTreeNode * pNode);
			void    ReleaseFileData();
			BSPTreeLeaf *FindLeaf(const XMFLOAT3& Position);

			bool  CommitBuffers(std::map<size_t, byte*> & leafBinVertexData,
				std::map<size_t, size_t> &leafBinVertexSizes,
				std::map<size_t, ULONG*> &leafBinIndices,
				std::map<size_t, size_t> &leafBinIndexSizes,
				std::map<size_t, std::set<uint16_t>> &leafBinTextureIds);

			bool BuildLightsBuffer();

			bool CommitPortalBuffers(byte* &vertices, size_t numVertices, ULONG *&indices, size_t numIndices);

			bool            PostBuild();

			bool			BuildTree(bspFileNode * pFileNode, BSPTreeNode * pNode = NULL);
			//void                    DrawBoundingBox(const XMFLOAT3 & Min, const XMFLOAT3 & Max, ULONG Color, bool bZEnable = false);
			//void                    DrawScreenTint(ULONG Color);
			void            CalculatePolyBounds();
			void            RepairTJunctions(Polygon * pPoly1, Polygon * pPoly2);
			//size_t                   WeldBuffers(size_t VertexCount, Vertex * pVertices, std::map<size_t, size_t*> & BinIndexData, std::map<size_t, size_t> & BinSizes, size_t * pFirstVertex = NULL, size_t * pPreviousVertex = NULL);

			void UpdateMovingDoors(float TimeScale);
			bool CheckDoorOpenCondition(Door *pDoor, bool testIfActive);

		protected:

			struct LightShader
			{
				XMFLOAT3 position;
				float  intensity;
				XMFLOAT3 color;
				UINT lightIndex; // level light index used to sample lightMapsArray
				float radius;
			};

			struct LeafDataShader
			{
				XMFLOAT3 ambient;
				UINT activeLightsMask;
				LightShader lights[16];
			};

			void	InitD3DStates();
			void    InitD3DQueryObjects();
			void	InitAABBRenderData();
			void	InitSkyBox(Camera *pCamera);
			void    ProcessVisibility();
			void    ProcessVisibilityLeaves(const LeafVector &leaves);
			void    ProcessVisibilityPVS(BSPTreeLeaf * pCurrentLeaf);
			void	MarkLeafAncestors(BSPTreeLeaf * pLeaf);
			void    SortLeavesFrontToBack();
			//void    ResetRenderBinData();
			void    ProcessLeafDoors(BSPTreeLeaf *pLeaf);

			void    RenderSubsets(CXMMATRIX tMat);
			void    RenderSubset(CXMMATRIX tMat, size_t materialID);
			void    RenderPortals(CXMMATRIX tMat, CXMVECTOR color);
			void	RenderLeavesAABB(CXMMATRIX tMat, CXMVECTOR playerLeafColor, CXMVECTOR color);
			void	RenderAABB(const XMFLOAT3 &Min, const XMFLOAT3 &Max, CXMMATRIX tMat, CXMVECTOR color);
			
			void    RenderDoors(CXMMATRIX tMat);
			
			bool m_PVSEnabled = true;      // Is PVS culling enabled in this application?
			bool m_FrustumEnabled = true;  // Is Frustum culling enabled in this application?
			
			bool m_sortLeavesFrontToBack = true;
			bool m_renderPortals = false;
			bool m_renderLeavesAABB = false;

			//-------------------------------------------------------------------------
			// Protected Variables for This Class
			//-------------------------------------------------------------------------
			D3DApp *m_pApp;
			ID3D11Device *m_pD3DDevice;       // The Direct3D Device to create the vertex / index buffers + to render
			ID3D11DeviceContext* m_pD3DDeviceContext;

			Camera *m_pCamera;
			BSPTreeLeaf *m_pCurrentLeaf;									
			LeafBinPortal *m_LeafBinPortals;		
			std::map<size_t, LeafBinTexture*> m_LeafBins; // The array of all of the "leaf bin" objects.

			LeafVector m_Leaves;      // The array of physical leaf items.
			PolygonVector m_Polygons; // Array of Stored polygon data (prior to building).
			PortalVector m_Portals;   // Array of Stored polygon data (prior to building).
			DoorVector m_doors;		  // 
			DoorVector m_pvsDoors;
			DoorVector m_movingDoors;
			//SkyboxComponent *m_pSkyBox;

			LeafVector m_pvsLeaves;    // pvs leaves
			BSPTreeLeaf *m_visibleLeavesList; // pvs-frustum culled leaves
			BSPTreeLeaf **m_pVisleavesPtr;
			std::vector<LeafDataShader> m_leafDataVec;

			ID3D11Buffer *m_pVBufferAABB;
			
			bspFileNode  * m_pFileNodes;        // Node data loaded from file
			XMFLOAT4     * m_pFilePlanes;       // Plane data loaded from file
			size_t         m_nFileNodeCount;    // Number of nodes loaded from file
			size_t         m_nFilePlaneCount;   // Number of planes loaded from file

			BSPTreeNode * m_pRootNode;         // The root node of the tree
			char        * m_strFileName;       // The name of the file we are loading from.

			size_t          m_nVisCounter;       // Counter for visibility processing

			UCHAR         *m_pPVSData;          // The actual visibility bit array
			size_t          m_nPVSSize;          // The size of the PVS array
			bool           m_bPVSCompressed;    // Is the PVS set ZRLE compressed?
			bool		   m_useLighting;
			XMFLOAT3 m_cachedCameraPos;

			TextureResources *m_pTextureResources;
			XMFLOAT2 m_clusterMapSize;

			SceneMaterial *m_pSceneMaterial;
			DoorMaterial *m_pDoorMaterial;
			BasicMaterial *m_pBasicMaterial; // used for portals and aabb
			

			ID3D11Buffer *m_pLeafDataBuffer;
			ID3D11ShaderResourceView *m_pLeafDataBufferSRV;
			
			ID3D11DepthStencilState* m_pWorldDepthStencilState;
			ID3D11DepthStencilState* m_pAABBRenderDepthStencilState;

			ID3D11RasterizerState* m_pWorldRasterizerState;
			ID3D11RasterizerState* m_pBasicRasterizeState; // for portals/aabb
		};

		
	}

}