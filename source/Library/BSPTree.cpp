#include "Common.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "SkyboxComponent.h"
#include "ColorHelper.h"
#include "BSPTree.h"
#include "Polygon.h"
#include "Collision.h"
#include "MathUtility.h"
#include "Camera.h"
#include "SceneMaterial.h"
#include "Door.h"
#include "DoorMaterial.h"
#include "BasicMaterial.h"
#include "DepthPassMaterial.h"
#include "TextureResources.h"

//#include <iostream>

using namespace Library;
using namespace Library::BSPEngine;


Library::BSPEngine::LeafBinData::LeafBinData()
{
}

Library::BSPEngine::LeafBinData::~LeafBinData()
{
}

void Library::BSPEngine::LeafBinData::addBatch(size_t IndexStart, size_t PrimitiveCount)
{
	m_renderBatches.push_back({ IndexStart, PrimitiveCount });
}

void Library::BSPEngine::LeafBinData::coalesceBatches()
{
	int i, j;
	int numBatches = (int)m_renderBatches.size();
	i = -1;
	j = 0;
	while (j != numBatches) {
		++i;
		m_renderBatches[i] = m_renderBatches[j];
		++j;
		size_t lastIndex = m_renderBatches[i].IndexStart + m_renderBatches[i].PrimitiveCount * 3;
		while (j != numBatches && m_renderBatches[j].IndexStart == lastIndex) {
			size_t numPrimitives = m_renderBatches[j].PrimitiveCount; // primitives to add
			m_renderBatches[i].PrimitiveCount += numPrimitives;
			lastIndex += numPrimitives * 3;
			++j;
		}
	}

	m_renderBatches.resize(i + 1);

	/*
	while (i != numBatches) {
		size_t j = i + 1;
		size_t lastIndex = m_renderBatches[i].IndexStart + m_renderBatches[i].PrimitiveCount * 3;
		while (j != numBatches && m_renderBatches[j].IndexStart == lastIndex) {
			size_t numPrimitives = m_renderBatches[j].PrimitiveCount; // primitives to add
			m_renderBatches[i].PrimitiveCount += numPrimitives;
			lastIndex += numPrimitives * 3;
			m_renderBatches[j].PrimitiveCount = 0;
			++j;
		}
		i = j;

	}
	*/
}

void Library::BSPEngine::LeafBinData::clearBatches()
{
	m_renderBatches.clear();
}

Library::BSPEngine::LeafBinData::RenderBatch & Library::BSPEngine::LeafBinData::getRenderBatch(size_t i)
{
	return m_renderBatches[i];
}

size_t Library::BSPEngine::LeafBinData::getRenderBatchCount()
{
	return m_renderBatches.size();
}

void Library::BSPEngine::LeafBinData::reserveBatches(size_t numBatches)
{
	m_renderBatches.reserve(numBatches);
}


Library::BSPEngine::LeafBin::LeafBin(BSPTree *pBspTree, size_t materialID, size_t LeafCount)
{
	// Store passed values
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_nFaceCount = 0;
	m_b32BitIndices = false;
	m_MaterialID = materialID;
	m_nLeafCount = LeafCount;
	m_pBspTree = pBspTree;
}

Library::BSPEngine::LeafBin::~LeafBin()
{
	ReleaseObject(m_pVertexBuffer);
	ReleaseObject(m_pIndexBuffer);

}

size_t Library::BSPEngine::LeafBin::GetMaterialID() const
{
	return m_MaterialID;
}


void Library::BSPEngine::LeafBin::CreateIndexBuffer(ID3D11Device * pD3DDevice, const ULONG * indices, size_t numIndices, bool use32bitIndices)
{
	m_b32BitIndices = use32bitIndices;
	m_nFaceCount = numIndices / 3;
	UCHAR     IndexStride = (use32bitIndices) ? 4 : 2;

	D3D11_SUBRESOURCE_DATA indexSubResourceData;
	ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));

	// Copy over the buffer data
	if (use32bitIndices)
	{
		// We can do a straight copy if it's a 32 bit index buffer
		indexSubResourceData.pSysMem = indices;
		m_pIndexBuffer = CreateD3DIndexBuffer(pD3DDevice, numIndices * IndexStride, false, &indexSubResourceData);
	}
	else {
		USHORT *indices16bit = new USHORT[numIndices];
		const ULONG *pIndices = indices;
		for (size_t i = 0; i < numIndices; ++i)
		{
			// Cast everything down to 16 bit
			indices16bit[i] = (USHORT)pIndices[i];
		}
		indexSubResourceData.pSysMem = indices16bit;
		m_pIndexBuffer = CreateD3DIndexBuffer(pD3DDevice, numIndices * IndexStride, false, &indexSubResourceData);
		delete[] indices16bit;
	}

}

//-----------------------------------------------------------------------------
// Name : AddVisibleData ()
// Desc : Given the specified IndexStart and PrimitiveCount, the render queues
//        with the data given
//-----------------------------------------------------------------------------
void Library::BSPEngine::LeafBin::AddVisibleData(LeafBinData * pData, size_t IndexStart, size_t PrimitiveCount)
{
	pData->addBatch(IndexStart, PrimitiveCount);
}


Library::BSPEngine::BSPTreeLeaf::BSPTreeLeaf(BSPTree * pTree)
{
	// Reset required variables

	// Store the tree
	m_pTree = pTree;

	// Reset / Clear all required values
	m_nVisCounter = 0;
	m_nPVSIndex = 0;

	m_nextVisible = nullptr;
	//m_nextOcclusion = nullptr;

	m_portalIndexStart = 0;
	m_portalPrimitiveCount = 0;
	
	m_LastFrustumPlane = -1;

	m_pParentNode = nullptr;

	//m_pQuery = nullptr;

	//m_isOccluded = false;
}

Library::BSPEngine::BSPTreeLeaf::~BSPTreeLeaf()
{

	m_pTree = nullptr;
	  // Note: We don't own the polygons or the detail areas 
	  // themselves, we merely store pointers to them, so we 
	  // should not delete them here.
	m_Polygons.clear();
	m_Portals.clear();
	//m_DetailAreas.clear();

	// Reset / Clear all required values
	m_nVisCounter = 0;
	m_nPVSIndex = 0;

	m_nextVisible = nullptr;
	//m_nextOcclusion = nullptr;

	m_pParentNode = nullptr;

	//ReleaseObject(m_pQuery);
}

bool Library::BSPEngine::BSPTreeLeaf::IsVisible() const
{
	return (m_nVisCounter == m_pTree->GetVisCounter());
}

void Library::BSPEngine::BSPTreeLeaf::SetVisible()
{
	m_nVisCounter = m_pTree->GetVisCounter();
}

void Library::BSPEngine::BSPTreeLeaf::AddRenderBatches()
{
	// Inform the renderer
	if (m_renderData.size() > 0)
	{
		// Loop through each renderable set in this leaf.
		for (size_t i = 0; i < m_renderData.size(); ++i)
		{
			BSPTreeLeaf::RenderData *pData = &m_renderData[i];
			LeafBinTexture *pLeafBin = pData->pLeafBin;

			// Loop through each element texture to render
			for (size_t j = 0; j < pData->elements.size(); ++j)
			{
				BSPTreeLeaf::RenderData::Element *pElement = &pData->elements[j];
				if (pElement->PrimitiveCount == 0) continue;

				LeafBinDataTexture * pBinData = pElement->pLeafBinData;//pLeafBin->GetBinData(pElement->texID);
				//if (!pData) continue;

				// Add this to the leaf bin
				pLeafBin->AddVisibleDataTexture(pBinData, pElement->IndexStart, pElement->PrimitiveCount, m_nVisCounter);

			} // Next Element
		} // Next RenderData Item
	} //  End if visible
}

size_t Library::BSPEngine::BSPTreeLeaf::GetPolygonCount() const
{
	// Return number of polygons stored in our internal vector
	return m_Polygons.size();
}

Library::BSPEngine::Polygon * Library::BSPEngine::BSPTreeLeaf::GetPolygon(size_t nIndex)
{
	// Validate the index
	if (nIndex >= m_Polygons.size()) return NULL;

	// Return the actual pointer
	return m_Polygons[nIndex];
}

void Library::BSPEngine::BSPTreeLeaf::GetBoundingBox(XMFLOAT3 & Min, XMFLOAT3 & Max) const
{
	// Retrieve the bounding boxes
	Min = m_vecBoundsMin;
	Max = m_vecBoundsMax;
}

size_t Library::BSPEngine::BSPTreeLeaf::GetPortalCount() const
{
	// Return number of portal polygons stored in our internal vector
	return m_Portals.size();
}

Library::BSPEngine::BSPTreePortal * Library::BSPEngine::BSPTreeLeaf::GetPortal(size_t nIndex)
{
	// Validate the index
	if (nIndex >= m_Portals.size()) return NULL;

	// Return the actual pointer
	return m_Portals[nIndex];
}

void Library::BSPEngine::BSPTreeLeaf::AddPortal(BSPTreePortal * pPortal)
{
	// Add to the portal list
	m_Portals.push_back(pPortal);
}

void Library::BSPEngine::BSPTreeLeaf::SetBoundingBox(const XMFLOAT3 & Min, const XMFLOAT3 & Max)
{
	// Store the bounding boxes
	m_vecBoundsMin = Min;
	m_vecBoundsMax = Max;
}

void Library::BSPEngine::BSPTreeLeaf::AddPolygon(Polygon * pPolygon)
{
	// Add to the polygon list
	m_Polygons.push_back(pPolygon);
}



BSPTreeLeaf::RenderData * Library::BSPEngine::BSPTreeLeaf::AddRenderData(size_t materialID)
{
	m_renderData.push_back(RenderData());

	// Get the element we just created
	RenderData *pData = &m_renderData[m_renderData.size()-1];

	// Clear the new entry
	ZeroMemory(pData, sizeof(RenderData));

	// Store the attribute ID, it's used to look up the render data later
	pData->materialID = materialID;

	// Return the item we created
	return pData;

}

BSPTreeLeaf::RenderData::Element * Library::BSPEngine::BSPTreeLeaf::AddRenderDataElement(size_t materialID)
{
	RenderData * pData = GetRenderData(materialID);
	if (!pData) return NULL;

	pData->elements.push_back(RenderData::Element());

    // Get a pointer to the new element
	RenderData::Element *pElement = &pData->elements[pData->elements.size()-1];

	// Clear the new entry
	ZeroMemory(pElement, sizeof(RenderData::Element));

	// Return the element we created
	return pElement;
}

BSPTreeLeaf::RenderData * Library::BSPEngine::BSPTreeLeaf::GetRenderData(size_t materialID)
{
	// Search for the correct render data item
	for (size_t i = 0; i < m_renderData.size(); ++i)
	{
		if (m_renderData[i].materialID == materialID) return &m_renderData[i];

	} // Next RenderData Item

	  // We didn't find anything
	return nullptr;
}

void Library::BSPEngine::BSPTreeLeaf::UpdateShaderLeafData(ID3D11DeviceContext* pDeviceContext)
{
	if (!m_pTree->m_useLighting) return;
	if (!m_updateShaderLeafData) return;
	m_updateShaderLeafData = false;

	BSPTree::LeafDataShader *pLightShader = &m_pTree->m_leafDataVec[m_index];
	XMFLOAT3 ambientColor(0.0f, 0.0f, 0.0f);
	
	uint16_t activeLightsMask = m_activeLightsMask;

	while (activeLightsMask) {

		// count #trailing bit, or firstbitlow(activeLightsMask)
		uint16_t v = activeLightsMask;
		int c;
		v = (v ^ (v - 1)) >> 1;  // Set v's trailing 0s to 1s and zero rest
		for (c = 0; v; c++) v >>= 1;
		activeLightsMask &= ~(1 << c);

		int iLight = c;
		Light *pLight = m_lights[iLight];
		///*
		XMStoreFloat3(&ambientColor,
			XMVectorAdd(XMLoadFloat3(&ambientColor),
				XMVectorMultiply(XMLoadFloat3(&pLight->color), XMVectorReplicate(pLight->intensity))));
		//*/
	}


	// Compute ambient and exposure. No physics was consulted to come up with this math.
	//float luminance = dot(ambientColor, float3(0.3f, 0.59f, 0.11f));
	//m_Exposure = 10.0f * expf(-0.5f * luminance);

	float luminance = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&ambientColor), XMLoadFloat3(&XMFLOAT3(0.3f, 0.59f, 0.11f))));
	float exposure = 10.0f * expf(-0.5f * luminance);
	
	///*
	//m_Ambient *= 0.001f;
	XMStoreFloat3(&ambientColor,
			XMVectorMultiply(XMLoadFloat3(&ambientColor), XMVectorReplicate(0.01f)));
	//m_Ambient += 0.0003f;
	XMStoreFloat3(&ambientColor,
		XMVectorAdd(XMLoadFloat3(&ambientColor), XMVectorReplicate(0.003f)));
	//*/
    pLightShader->ambient = ambientColor;
	pLightShader->activeLightsMask = m_activeLightsMask;

	// update gpu leaf data buffer
	D3D11_BOX destbox;
	destbox.left = (UINT)(sizeof(BSPTree::LeafDataShader) * m_index);
	destbox.top = 0;
	destbox.front = 0;
	destbox.right = (UINT)(sizeof(BSPTree::LeafDataShader) * (m_index + 1));
	destbox.bottom = 1;
	destbox.back = 1;

	pDeviceContext->UpdateSubresource(m_pTree->m_pLeafDataBuffer, 0, &destbox, pLightShader, 0, 0);
}

void Library::BSPEngine::BSPTreeLeaf::addDoor(size_t doorIdx)
{
	m_doorsIndices.insert(doorIdx);
}

void Library::BSPEngine::BSPTreeLeaf::removeDoor(size_t doorIdx)
{
	m_doorsIndices.erase(doorIdx);
}

Library::BSPEngine::BSPTreeNode::BSPTreeNode()
{
	// Reset / Clear all required values
	Front = NULL;
	Back = NULL;
	Leaf = NULL;
	Parent = NULL;
	LastFrustumPlane = -1;
	m_nVisCounter = 0;
}

Library::BSPEngine::BSPTreeNode::~BSPTreeNode()
{
	// Delete nodes if they exists
	if (Front) delete Front;
	if (Back) delete Back;

	// Note : We don't own the leaf, the tree does, so we don't delete it here

	// Reset / Clear all required values
	Front = NULL;
	Back = NULL;
	Leaf = NULL;
}


Library::BSPEngine::BSPTree::BSPTree(D3DApp *app, const char * FileName)
{
	m_pApp = app;
	// Store the D3D Device and other details
	m_pD3DDevice = app->Direct3DDevice();
	m_pD3DDeviceContext = app->Direct3DDeviceContext();

	// Add ref the device (if available)
	m_pD3DDevice->AddRef();
	m_pD3DDeviceContext->AddRef();

	m_pWorldDepthStencilState = nullptr;
	m_pAABBRenderDepthStencilState = nullptr;

	m_pWorldRasterizerState = nullptr;
	m_pBasicRasterizeState = nullptr;

	m_pCamera = nullptr;
	//m_pSkyBox = nullptr;


	// Clear required variables
	m_pRootNode = NULL;
	m_pFileNodes = NULL;
	m_pFilePlanes = NULL;
	m_nFileNodeCount = 0;
	m_nFilePlaneCount = 0;
	m_nVisCounter = 0;

	// Visibility variables
	m_pPVSData = NULL;
	m_nPVSSize = 0;
	m_bPVSCompressed = false;

	// Make a copy of the file name
	m_strFileName = _strdup(FileName);

	m_pTextureResources = nullptr;

	m_pSceneMaterial = new SceneMaterial;
	m_pSceneMaterial->Initialize(m_pD3DDevice, app->WindowHandle());

	m_pDoorMaterial = new DoorMaterial;
	m_pDoorMaterial->Initialize(m_pD3DDevice, app->WindowHandle());

	m_pBasicMaterial = new BasicMaterial;
	m_pBasicMaterial->Initialize(m_pD3DDevice, app->WindowHandle());

	m_pLeafDataBuffer = nullptr;
	m_pLeafDataBufferSRV = nullptr;

	m_LeafBinPortals = nullptr;

	m_pCurrentLeaf = nullptr;
	m_visibleLeavesList = nullptr;
	m_pVisleavesPtr = nullptr;

	m_useLighting = false;

	//m_PVSEnabled = true;
	//m_FrustumEnabled = true;

	m_pVBufferAABB = nullptr;

}

Library::BSPEngine::BSPTree::~BSPTree()
{
	m_pSceneMaterial = nullptr;
	m_pDoorMaterial = nullptr;
	m_pBasicMaterial = nullptr;

	DeleteObject(m_LeafBinPortals);

	m_pCamera = nullptr;
	m_pCurrentLeaf = nullptr;
	m_visibleLeavesList = nullptr;
	m_pVisleavesPtr = nullptr;

	// Release any resources
	if (m_pRootNode) delete m_pRootNode;
	if (m_strFileName) free(m_strFileName); // Allocated with _tcsdup
	if (m_pPVSData) delete[] m_pPVSData;

	// Release file data in case it wasn't freed
	ReleaseFileData();

	// Visibility variables
	m_pPVSData = NULL;
	m_nPVSSize = 0;
	m_bPVSCompressed = false;
	
	auto PolyIterator = m_Polygons.begin();
	auto BinIterator = m_LeafBins.begin();
	auto LeafIterator = m_Leaves.begin();
	auto PortalIterator = m_Portals.begin();

	// Iterate through any stored polygons and destroy them
	for (; PolyIterator != m_Polygons.end(); ++PolyIterator)
	{
		Polygon * pPoly = *PolyIterator;
		if (pPoly) delete pPoly;

	} // Next Polygon
	m_Polygons.clear();

	// Iterate through the leaf bins and destroy them
	for (; BinIterator != m_LeafBins.end(); ++BinIterator)
	{
		LeafBin * pBin = BinIterator->second;
		if (pBin) delete pBin;

	} // Next Leaf Bin
	m_LeafBins.clear();

	// Iterate through the leaves and destroy them
	for (; LeafIterator != m_Leaves.end(); ++LeafIterator)
	{
		BSPTreeLeaf * pLeaf = *LeafIterator;
		if (pLeaf) delete pLeaf;

	} // Next Leaf
	m_Leaves.clear();

	// Iterate through the portals and destroy them
	for (; PortalIterator != m_Portals.end(); ++PortalIterator)
	{
		BSPTreePortal * pPortal = *PortalIterator;
		if (pPortal) delete pPortal;

	} // Next Leaf
	m_Portals.clear();

	//DeleteObject(m_pSkyBox);

	ReleaseObject(m_pVBufferAABB);

	ReleaseObject(m_pLeafDataBuffer);
	ReleaseObject(m_pLeafDataBufferSRV);

	ReleaseObject(m_pWorldDepthStencilState);
	ReleaseObject(m_pAABBRenderDepthStencilState);

	ReleaseObject(m_pWorldRasterizerState);
	ReleaseObject(m_pBasicRasterizeState);

	ReleaseObject(m_pD3DDeviceContext);
	ReleaseObject(m_pD3DDevice);

	m_pApp = nullptr;
}

bool Library::BSPEngine::BSPTree::Load(FILE * file)
{
	// PLANES
	uint32_t numPlanes;

	fread(&numPlanes, sizeof(uint32_t), 1, file);

	m_nFilePlaneCount = numPlanes;
	m_pFilePlanes = new XMFLOAT4[m_nFilePlaneCount];

	for (uint32_t i = 0; i != numPlanes; ++i) {
		float normal[3];
		float dist;
		fread(normal, sizeof(float), 3, file);
		fread(&dist, sizeof(float), 1, file);
		m_pFilePlanes[i].x = normal[0]; // a
		m_pFilePlanes[i].y = normal[1]; // b
		m_pFilePlanes[i].z = normal[2]; // c
		m_pFilePlanes[i].w = dist; // d
	}

	// NODES
	uint32_t numNodes;
	fread(&numNodes, sizeof(uint32_t), 1, file);

	m_nFileNodeCount = numNodes;
	m_pFileNodes = new bspFileNode[m_nFileNodeCount];

	for (uint32_t i = 0; i != numNodes; ++i) {
		uint32_t planeIdx;
		XMFLOAT3 boundsMin, boundsMax;
		uint32_t frontIdx, backIdx;
		fread(&planeIdx, sizeof(uint32_t), 1, file);
		fread(&boundsMin, sizeof(float), 3, file);
		fread(&boundsMax, sizeof(float), 3, file);
		fread(&frontIdx, sizeof(uint32_t), 1, file);
		fread(&backIdx, sizeof(uint32_t), 1, file);
		m_pFileNodes[i].PlaneIndex = planeIdx;
		m_pFileNodes[i].BoundsMin = boundsMin;
		m_pFileNodes[i].BoundsMax = boundsMax;
		m_pFileNodes[i].FrontIndex = frontIdx;
		m_pFileNodes[i].BackIndex = backIdx;
	}

	// PORTALS

	uint32_t numPortals;
	fread(&numPortals, sizeof(uint32_t), 1, file);

	m_Portals.reserve(numPortals);

	for (uint32_t i = 0; i != numPortals; ++i) {
		BSPTreePortal *portal = new BSPTreePortal;
		uint32_t VertexCount;

		fread(&portal->mOwnerNode, sizeof(uint32_t), 1, file);
		fread(&portal->mFrontOwner, sizeof(uint32_t), 1, file);
		fread(&portal->mBackOwner, sizeof(uint32_t), 1, file);
		fread(&VertexCount, sizeof(uint32_t), 1, file);

		portal->mPolygon = new Polygon;
		portal->mPolygon->AddVertex(VertexCount);
		Vertex *pVertices = portal->mPolygon->m_pVertex;

		for (uint32_t k = 0; k < VertexCount; k++) {
			float point[3];
			fread(point, sizeof(float), 3, file);
			pVertices[k].x = point[0];
			pVertices[k].y = point[1];
			pVertices[k].z = point[2];
		}

		AddPortal(portal);
	}

	// LEAVES
	uint32_t numLeaves;
	fread(&numLeaves, sizeof(uint32_t), 1, file);

	m_Leaves.reserve(numLeaves);

	for (uint32_t i = 0; i != numLeaves; ++i) {
		BSPTreeLeaf *leaf = new BSPTreeLeaf(this);

		XMFLOAT3 boundsMin, boundsMax;
		uint32_t pvsIndex, polygonCount, portalCount;

		fread(&boundsMin, sizeof(float), 3, file);
		fread(&boundsMax, sizeof(float), 3, file);
		fread(&pvsIndex, sizeof(uint32_t), 1, file);
		fread(&polygonCount, sizeof(uint32_t), 1, file);
		fread(&portalCount, sizeof(uint32_t), 1, file);

		leaf->SetBoundingBox(boundsMin, boundsMax);
		leaf->m_nPVSIndex = pvsIndex;

		// Read the face indices
		for (uint32_t j = 0; j < polygonCount; ++j) {
			uint32_t polygonIdx;
			fread(&polygonIdx, sizeof(uint32_t), 1, file);
			leaf->AddPolygon(m_Polygons[polygonIdx]);
		}

		// Read the portal indices
		for (uint32_t j = 0; j < portalCount; ++j) {
			uint32_t portalIdx;
			fread(&portalIdx, sizeof(uint32_t), 1, file);
			leaf->AddPortal(m_Portals[portalIdx]);
		}

		// Add this leaf to our leaf array
		AddLeaf(leaf);
	}

	// PVS
	uint32_t pvsSize;
	bool isCompressed;
	fread(&pvsSize, sizeof(uint32_t), 1, file);
	fread(&isCompressed, sizeof(bool), 1, file);

	m_nPVSSize = pvsSize;
	m_bPVSCompressed = isCompressed;

	m_pPVSData = new UCHAR[m_nPVSSize];
	fread(m_pPVSData, m_nPVSSize, 1, file);

	return true;
}

bool Library::BSPEngine::BSPTree::Build(TextureResources *textureResources)
{
	// No BSP tree data loaded?
	if (m_nFileNodeCount == 0 || m_Leaves.size() == 0) return false;

	// Reconstruct the tree structure from that loaded
	BuildTree(&m_pFileNodes[0], NULL);

	// Release the file data we had loaded.
	ReleaseFileData();

	m_pTextureResources = textureResources;

	return PostBuild();
}

void Library::BSPEngine::BSPTree::Initialize(Camera *pCamera)
{
	InitD3DStates();
	InitD3DQueryObjects();
	InitAABBRenderData();
	InitSkyBox(pCamera);
	m_pCamera = pCamera;
}

void Library::BSPEngine::BSPTree::Render(const Timer &timer)
{
	if (!m_pD3DDeviceContext) return;

	// Allow the spatial tree to process visibility
	ProcessVisibility();

	m_pSceneMaterial->SetUseLighting(m_useLighting);

	//XMMATRIX world = XMLoadFloat4x4(&MatrixHelper::Identity);
	//XMMATRIX wv = world * mCamera->ViewMatrix();
	//world * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
	CXMMATRIX vpMat = m_pCamera->ViewProjectionMatrix();

	m_pD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pD3DDeviceContext->RSSetState(m_pWorldRasterizerState);
	m_pD3DDeviceContext->OMSetDepthStencilState(m_pWorldDepthStencilState, 0);
	
	RenderDoors(vpMat);
	RenderSubsets(vpMat);

	if (m_renderPortals) {
		m_pD3DDeviceContext->RSSetState(m_pBasicRasterizeState);
		RenderPortals(vpMat, ColorHelper::Green);
	}

	if (m_renderLeavesAABB) {
		m_pD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_pD3DDeviceContext->OMSetDepthStencilState(m_pAABBRenderDepthStencilState, 0);
		m_pD3DDeviceContext->RSSetState(m_pBasicRasterizeState);
		RenderLeavesAABB(vpMat, ColorHelper::Red, ColorHelper::Blue);
	}	

	//m_pSkyBox->Draw(timer);
}

void Library::BSPEngine::BSPTree::ProcessVisibility()
{
	// Increment our visibility counter for this loop
	m_nVisCounter++;

	// Cache camera position if camera is not locked
	//if (!Camera.IsFrustumLocked()) m_cachedCameraPos = Camera.GetPosition();
	m_cachedCameraPos = m_pCamera->GetPosition();

	// Find the leaf that the camera is currently in
	BSPTreeLeaf *pCurrentLeaf = FindLeaf(m_cachedCameraPos);
	
	// If we were not within any valid leaf, or PVS rendering is disabled or we have at most 1 leaf, 
	// then simply mark all of the leaves as visible
	if (!pCurrentLeaf || !m_PVSEnabled || m_Leaves.size() <= 1)
	{
		m_pCurrentLeaf = pCurrentLeaf;//nullptr;
		ProcessVisibilityLeaves(m_Leaves); 
		// We're done
		return;
	} // End if render all leaves

#ifdef PRINT_VISIBILITY_INFO
	printf("\n\nCurrent Leaf: %zu* ", pCurrentLeaf->m_index);
#endif

	if (pCurrentLeaf != m_pCurrentLeaf) {
		m_pvsLeaves.clear();
		ProcessVisibilityPVS(pCurrentLeaf);
		m_pCurrentLeaf = pCurrentLeaf;
		//std::cout << "change leaf: " << m_pCurrentLeaf->m_index << "\n";
		if (m_sortLeavesFrontToBack) {
			SortLeavesFrontToBack();
		}
	}

	ProcessVisibilityLeaves(m_pvsLeaves);

}

void Library::BSPEngine::BSPTree::ProcessVisibilityLeaves(const LeafVector &leaves)
{
	XMFLOAT3    vecMin, vecMax;

#ifdef PRINT_VISIBILITY_INFO
	printf("\n\nVisible Leaves: ");
#endif

	m_pvsDoors.clear();

	// prepare list pointer to concatenate visible leaves
	m_pVisleavesPtr = &m_visibleLeavesList; 

	for (size_t i = 0; i < leaves.size(); ++i)
	{
		BSPTreeLeaf * pVisibleLeaf = (BSPTreeLeaf*)leaves[i];
		if (!pVisibleLeaf) continue;

		// Bounding box in the camera frustum?
		pVisibleLeaf->GetBoundingBox(vecMin, vecMax);
		if (!m_FrustumEnabled || m_pCamera->BoundsInFrustum(vecMin, vecMax, nullptr, nullptr, &pVisibleLeaf->m_LastFrustumPlane) != Camera::FRUSTUM_OUTSIDE)
		{
			pVisibleLeaf->SetVisible();
			*m_pVisleavesPtr = pVisibleLeaf;
			m_pVisleavesPtr = &pVisibleLeaf->m_nextVisible;
#ifdef PRINT_VISIBILITY_INFO
			printf("%zu ", pVisibleLeaf->m_index);
#endif		
		}

		// check leaf doors
		ProcessLeafDoors(pVisibleLeaf);

	} // Next Leaf

	// end list
	*m_pVisleavesPtr = nullptr;
}

void Library::BSPEngine::BSPTree::ProcessVisibilityPVS(BSPTreeLeaf * pCurrentLeaf)
{

	// We are in a valid leaf, let's test its PVS.
	UCHAR *pPVS = &m_pPVSData[pCurrentLeaf->m_nPVSIndex];
	size_t LeafCount = m_Leaves.size();

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
					BSPTreeLeaf * pVisibleLeaf = m_Leaves[LeafIndex];
					if (!pVisibleLeaf) continue;
					
					if (m_sortLeavesFrontToBack) {
						MarkLeafAncestors(pVisibleLeaf);
					}
					else {
						// or add leaf in pvs order
						AddVisibleLeaf(pVisibleLeaf);
					}
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
}

void Library::BSPEngine::BSPTree::MarkLeafAncestors(BSPTreeLeaf * pLeaf)
{
	BSPTreeNode *pNode = pLeaf->m_pParentNode;
	while (pNode && pNode->m_nVisCounter != m_nVisCounter) {
		pNode->m_nVisCounter = m_nVisCounter;
		pNode = pNode->Parent;
	}
}

void Library::BSPEngine::BSPTree::SortLeavesFrontToBack()
{
	SortLeavesFrontToBackRecurse(m_pRootNode);
}

void Library::BSPEngine::BSPTree::ProcessLeafDoors(BSPTreeLeaf * pLeaf)
{
	for (auto it = std::begin(pLeaf->m_doorsIndices); it != std::end(pLeaf->m_doorsIndices); ++it)
	{
		size_t doorIdx = *it;
		Door *pDoor = m_doors[doorIdx];
		//if (door->IsActive())
		if (!pDoor->IsMarked()) {
			pDoor->SetMarked(true);
			m_pvsDoors.push_back(pDoor);
			
			if (CheckDoorOpenCondition(pDoor, true))
			{
				pDoor->Active();
				m_movingDoors.push_back(pDoor);
			}
		}
	}
}


bool Library::BSPEngine::BSPTree::CollectLeavesAABB(LeafVector & List, const XMFLOAT3 & Min, const XMFLOAT3 & Max)
{
	// Trigger Recursion
	return CollectAABBRecurse(m_pRootNode, List, Min, Max);
}

bool Library::BSPEngine::BSPTree::CollectLeavesRay(LeafVector & List, const XMFLOAT3 & RayOrigin, const XMFLOAT3 & Velocity)
{
	return CollectRayRecurse(m_pRootNode, List, RayOrigin, Velocity);
}

bool Library::BSPEngine::BSPTree::GetSceneBounds(XMFLOAT3 & Min, XMFLOAT3 & Max)
{
	if (!m_pRootNode) return false;
	Min = m_pRootNode->BoundsMin;
	Max = m_pRootNode->BoundsMax;
	return true;
}

void Library::BSPEngine::BSPTree::AddLight(Light * light)
{
	for (auto it = std::begin(light->m_lightInLeafClusterIndex); 
		it != std::end(light->m_lightInLeafClusterIndex); ++it) {
		uint16_t leafIdx = it->first;
		BSPTreeLeaf *leaf = m_Leaves[leafIdx];
		uint8_t lightClusterIndex = it->second;
		leaf->m_lights[lightClusterIndex] = light;
		if (light->isActive) {
			leaf->m_activeLightsMask |= (1 << lightClusterIndex);
		}
		else {
			leaf->m_activeLightsMask &= ~(1 << lightClusterIndex);
		}
	}
}

void Library::BSPEngine::BSPTree::SetClusterMapSize(XMFLOAT2 size)
{
	m_clusterMapSize = size;
}

void Library::BSPEngine::BSPTree::AddPolygon(Polygon * pPolygon)
{
	// Add to the polygon list
	m_Polygons.push_back(pPolygon);
}

void Library::BSPEngine::BSPTree::AddDoor(Door * pDoor)
{
	m_doors.push_back(pDoor);
}

bool Library::BSPEngine::BSPTree::Repair()
{
	size_t                SrcCounter, DestCounter, i;
	Polygon               *pCurrentPoly, *pTestPoly;
	PolygonVector::iterator SrcIterator, DestIterator;
	LeafVector::iterator      LeafIterator;
	LeafVector                LeafVector;

	// No-Op ?
	if (m_Polygons.size() == 0) return true;

	try
	{
		// Loop through Faces
		for (SrcCounter = 0, SrcIterator = m_Polygons.begin(); SrcIterator != m_Polygons.end(); ++SrcCounter, ++SrcIterator)
		{
			// Get poly pointer and bounds references for easy access
			pCurrentPoly = *SrcIterator;
			if (!pCurrentPoly) continue;
			XMFLOAT3 & SrcMin = pCurrentPoly->m_vecBoundsMin;
			XMFLOAT3 & SrcMax = pCurrentPoly->m_vecBoundsMax;

			// Retrieve the list of leaves intersected by this poly
			LeafVector.clear();
			CollectLeavesAABB(LeafVector, SrcMin, SrcMax);

			// Loop through each leaf in the set
			for (LeafIterator = LeafVector.begin(); LeafIterator != LeafVector.end(); ++LeafIterator)
			{
				BSPTreeLeaf * pLeaf = *LeafIterator;
				if (!pLeaf) continue;

				// Loop through Faces
				DestCounter = pLeaf->GetPolygonCount();
				for (i = 0; i < DestCounter; ++i)
				{
					// Get poly pointer and bounds references for easy access
					pTestPoly = pLeaf->GetPolygon(i);
					if (!pTestPoly || pTestPoly == pCurrentPoly) continue;
					XMFLOAT3 & DestMin = pTestPoly->m_vecBoundsMin;
					XMFLOAT3 & DestMax = pTestPoly->m_vecBoundsMax;

					// If the two do not intersect then there is no need for testing.
					if (!Collision::AABBIntersectAABB(SrcMin, SrcMax, DestMin, DestMax)) continue;

					// Repair against the testing poly
					RepairTJunctions(pCurrentPoly, pTestPoly);

				} // Next Test Face

			} // Next Leaf

		} // Next Current Face

	} // End Try Block

	catch (...)
	{
		// Clean up and return (failure)
		return false;

	} // End Catch Block

	  // Success!
	return true;
}

PolygonVector & Library::BSPEngine::BSPTree::GetPolygonList()
{
	return m_Polygons;
}

BSPTree::LeafVector & Library::BSPEngine::BSPTree::GetLeafList()
{
	return m_Leaves;
}

BSPTree::LeafVector & Library::BSPEngine::BSPTree::GetVisibleLeafList()
{
	return m_pvsLeaves;
}


void Library::BSPEngine::BSPTree::RenderSubsets(CXMMATRIX tMat)
{
	//ResetRenderBinData();

	for (BSPTreeLeaf *pLeaf = m_visibleLeavesList;
		pLeaf;
		pLeaf = pLeaf->m_nextVisible)
	{
		//ResetRenderBinData();
		pLeaf->AddRenderBatches();
		pLeaf->UpdateShaderLeafData(m_pD3DDeviceContext);
		//RenderSubset(tMat, 0);
		//printf("%zd\n", pLeaf->m_index);
		//break;
	}

	// render each rendering subset
	RenderSubset(tMat, 0);

}

void Library::BSPEngine::BSPTree::RenderSubset(CXMMATRIX tMat, size_t materialID)
{
	// Retrieve the applicable leaf bin for this attribute
	LeafBinTexture * pLeafBin = GetLeafBin(materialID);
	if (!pLeafBin) return;

	// Render the leaf bin
	pLeafBin->Render(m_pD3DDeviceContext, tMat);
}

void Library::BSPEngine::BSPTree::RenderPortals(CXMMATRIX tMat, CXMVECTOR color)
{
	//if (!m_renderPortals) return;
	m_LeafBinPortals->ResetBinData();

	// use only pvs leaves
	for (auto leafIt = begin(m_pvsLeaves); leafIt != end(m_pvsLeaves); ++leafIt) {
		BSPTreeLeaf *pLeaf = *leafIt;
		if (pLeaf->IsVisible() && pLeaf->m_portalPrimitiveCount != 0) { // && !pLeaf->m_isOccluded
			m_LeafBinPortals->AddVisibleData(pLeaf->m_portalIndexStart, pLeaf->m_portalPrimitiveCount);
		}
	}
	
	// Render the leaf bin
	m_LeafBinPortals->Render(m_pD3DDeviceContext, tMat, color);
}

void Library::BSPEngine::BSPTree::RenderLeavesAABB(CXMMATRIX tMat, CXMVECTOR playerLeafColor, CXMVECTOR color)
{	
	//if (!m_renderLeavesAABB) return;
	
	// use only pvs leaves
	for (auto leafIt = begin(m_pvsLeaves); leafIt != end(m_pvsLeaves); ++leafIt) {
		BSPTreeLeaf *pLeaf = *leafIt;
		if (!pLeaf->IsVisible()) continue; // || pLeaf->m_isOccluded
		if (pLeaf == m_pCurrentLeaf) continue;
		CXMVECTOR leafColor = (pLeaf == m_pCurrentLeaf) ? playerLeafColor : color;
		RenderAABB(pLeaf->m_vecBoundsMin, pLeaf->m_vecBoundsMax, tMat, leafColor);
	}

	if (m_pCurrentLeaf) {
		RenderAABB(m_pCurrentLeaf->m_vecBoundsMin, m_pCurrentLeaf->m_vecBoundsMax, tMat, playerLeafColor);
	}
}

void Library::BSPEngine::BSPTree::RenderAABB(const XMFLOAT3 & Min, const XMFLOAT3 & Max, CXMMATRIX vpMat, CXMVECTOR color)
{
	// compute world matrix
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world,
		XMMatrixScalingFromVector(XMVectorSubtract(XMLoadFloat3(&Max), XMLoadFloat3(&Min))));

	XMFLOAT3 BoundsCenter;
	XMStoreFloat3(&BoundsCenter,
		XMVectorMultiply(XMVectorAdd(XMLoadFloat3(&Min), XMLoadFloat3(&Max)), XMVectorReplicate(0.5)));

	// Translate the bounding box matrix
	world._41 = BoundsCenter.x;
	world._42 = BoundsCenter.y;
	world._43 = BoundsCenter.z;

	XMMATRIX wvp = XMLoadFloat4x4(&world) * vpMat;

	// set vertex buffer
	UINT stride = (UINT)m_pBasicMaterial->VertexSize();
	UINT offset = 0;
	m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVBufferAABB, &stride, &offset);

	m_pBasicMaterial->Apply(m_pD3DDeviceContext);

	// update material wvp variable
	m_pBasicMaterial->Update(m_pD3DDeviceContext, wvp, color);

	m_pD3DDeviceContext->Draw(24, 0);
	
}



void Library::BSPEngine::BSPTree::RenderDoors(CXMMATRIX tMat)
{
	XMFLOAT3 vecMin, vecMax;

	for (auto it = std::begin(m_pvsDoors); it != std::end(m_pvsDoors); ++it)
	{
		Door *pDoor = *it;
		pDoor->SetMarked(false);

		//pDoor->GetBoundingBox(vecMin, vecMax);
		//if (!m_FrustumEnabled || m_pCamera->BoundsInFrustum(vecMin, vecMax, nullptr, nullptr, pDoor->GetLastFrustumPlane()) != Camera::FRUSTUM_OUTSIDE)
		//{
			pDoor->Render(m_pD3DDeviceContext, tMat);
			
			//printf("door %zu\n", pDoor->GetIndex());
		
		//}	
		
	}
}


LeafBinTexture * Library::BSPEngine::BSPTree::GetLeafBin(size_t materialID)
{
	auto Item = m_LeafBins.find(materialID);

	// We don't store this attribute ID
	if (Item == m_LeafBins.end()) return nullptr;

	// Return the actual item
	return Item->second;
}

void Library::BSPEngine::BSPTree::AddLeaf(BSPTreeLeaf * pLeaf)
{
	// Add to the leaf list
	m_Leaves.push_back(pLeaf);
	pLeaf->SetIndex(m_Leaves.size() - 1);
}

void Library::BSPEngine::BSPTree::AddPortal(BSPTreePortal * pPortal)
{
	m_Portals.push_back(pPortal);
}

void Library::BSPEngine::BSPTree::AddVisibleLeaf(BSPTreeLeaf * pLeaf)
{
	// Add to the leaf list
	m_pvsLeaves.push_back(pLeaf);
}

bool Library::BSPEngine::BSPTree::BuildRenderData()
{
	std::map<size_t, size_t> leafBinVertexSizes; // num vertices for each material/shader
	std::map<size_t, byte *> leafBinVertexData; // vertex buffer for each material/shader
	std::map<size_t, size_t> leafBinVertexDataOffsets; // vertex byte offsets for each material/shader

	std::map<size_t, ULONG *> leafBinIndices; // indices buffer for each material/shader
	std::map<size_t, size_t> leafBinIndexSizes; // num indices for each material/shader
	std::map<size_t, std::set<uint16_t>> leafBinTextureIds; // texture ids for each leafBin
	
	std::map<size_t, std::map<uint16_t, size_t>> leafBinDataIndexSizes; // num indices for each (material/shader, texture)
	std::map<size_t, std::map<uint16_t, size_t>> leafBinDataIndexOffsets; // indices offsets for each (material/shader, texture)

	// Portals data
	size_t numPortalsVertices = 0;
	size_t numPortalsIndices = 0;
	byte *portalsVertices = nullptr;
	size_t portalsVerticesOffset = 0;
	ULONG *portalIndices = nullptr;
	size_t portalIndicesOffset = 0;

	LeafVector           Leaves;

	//bool               b32BitIndices = false;
	XMFLOAT3        Min, Max;

	// Instructed not to build data?
	if (!m_pD3DDevice) return true;

	// Anything to do ?
	if (m_Leaves.size() == 0) return false;

	// First thing we need to do is repair any T-Junctions created during the build
	Repair();

	// Retrieve the leaf list in TRAVERSAL order to ensure that the 
	// render batching works as efficiently as possible
	GetSceneBounds(Min, Max);
	CollectLeavesAABB(Leaves, Min, Max);

	m_LeafBins.clear();

	// Iterate through all of the polygons in the tree and allocate leaf bins for each matching
	// attribute ID. In addition, total up the index counts required for each bin.
	for (auto LeafIterator = Leaves.begin(); LeafIterator != Leaves.end(); ++LeafIterator)
	{
		// Retrieve leaf
		BSPTreeLeaf *pLeaf = *LeafIterator;
		if (!pLeaf) continue;

		// Loop through each of the polygons stored in this leaf
		size_t nPolygonCount = pLeaf->GetPolygonCount();
		for (size_t i = 0; i < nPolygonCount; ++i)
		{
			Polygon * pPoly = pLeaf->GetPolygon(i);

			// Skip if it's invalid
			if (!pPoly || pPoly->m_nVertexCount < 3) continue;

			// Skip if it's not visible
			if (!pPoly->m_bVisible) continue;

			uint16_t materialID = pPoly->m_materialID;

			// Retrieve the leaf bin for this attribute
			LeafBinTexture *pLeafBin = m_LeafBins[materialID];

			// Determine if there is a matching bin already in existence
			if (!pLeafBin)
			{
				// Allocate a new leaf bin
					
				pLeafBin = new LeafBinTexture(this, materialID, m_pSceneMaterial, m_Leaves.size());
				if (!pLeafBin) return false;
     
				// Add to the leaf bin list
				m_LeafBins[materialID] = pLeafBin;

			} // End if no bin existing

			// Change from texId to texArrayId
			uint16_t arrTexID = m_pTextureResources->getTexArrayIndex(pPoly->m_texID);

			leafBinTextureIds[materialID].insert(arrTexID); // pPoly->m_texID

			// Total up vertices for current material
			leafBinVertexSizes[materialID] += pPoly->m_nVertexCount;

			// Count also #indices
			size_t numIndices = (pPoly->m_nVertexCount - 2) * 3;
			leafBinDataIndexSizes[materialID][arrTexID] += numIndices;
			leafBinIndexSizes[materialID] += numIndices;

		} // Next Polygon 

		// Leaf portals info
		if (m_renderPortals) {
			size_t nPortalsCount = pLeaf->m_Portals.size();
			for (size_t i = 0; i < nPortalsCount; ++i)
			{
				BSPTreePortal *pPortal = pLeaf->GetPortal(i);
				// add only front portals
				if (pPortal->mFrontOwner != pLeaf->m_index) continue;
				Polygon *pPortalPoly = pPortal->mPolygon;
				if (!pPortalPoly || pPortalPoly->m_nVertexCount < 3) continue;
				numPortalsVertices += pPortalPoly->m_nVertexCount;
				numPortalsIndices += (pPortalPoly->m_nVertexCount - 2) * 3;
			}
		}

	} // Next Leaf


	// compute index offsets for each (material, texId)

	for (auto leafBinIndexSizesIt = cbegin(leafBinDataIndexSizes);
		leafBinIndexSizesIt != cend(leafBinDataIndexSizes);
		++leafBinIndexSizesIt) {

		// reserve memory for vertex buffer
		size_t vertexNum = leafBinVertexSizes[leafBinIndexSizesIt->first];
		leafBinVertexData[leafBinIndexSizesIt->first] = new byte[vertexNum * m_pSceneMaterial->VertexSize()];
		
		// set also vertex offset to 0
		leafBinVertexDataOffsets[leafBinIndexSizesIt->first] = 0;

		// reserve memory for index  buffer
		size_t numIndices = leafBinIndexSizes[leafBinIndexSizesIt->first];
		leafBinIndices[leafBinIndexSizesIt->first] = new ULONG[numIndices];

		//b32BitIndices[leafBinIndexSizesIt->first] = ((vertexNum - 1) > 0xFFFF) ? true : false;

		// adjust offsets for each texId
		auto &leafBinDataIndexSizes = leafBinIndexSizesIt->second;
		size_t offset = 0;

		for (auto indexSizeIt = cbegin(leafBinDataIndexSizes);
				  indexSizeIt != cend(leafBinDataIndexSizes);
				  ++indexSizeIt)
		{
			leafBinDataIndexOffsets[leafBinIndexSizesIt->first][indexSizeIt->first] = offset;
			offset += indexSizeIt->second;
		}

	}

	// Portals buffers allocations
	if (m_renderPortals) {
		portalsVertices = new byte[numPortalsVertices * m_pBasicMaterial->VertexSize()];
		portalIndices = new ULONG[numPortalsIndices];
	}

	// Now we actually build the renderable data for the traversal ordered leaf list.
	for (auto LeafIterator = Leaves.begin(); LeafIterator != Leaves.end(); ++LeafIterator)
	{
		// Retrieve leaf
		BSPTreeLeaf *pLeaf = *LeafIterator;
		if (!pLeaf) continue;

		// Loop through each of the polygons stored in this leaf
		size_t nPolygonCount = pLeaf->GetPolygonCount();
		for (size_t i = 0; i < nPolygonCount; ++i)
		{
			Polygon * pPoly = pLeaf->GetPolygon(i);

			// Skip if it's invalid
			if (!pPoly || pPoly->m_nVertexCount < 3) continue;

			// Skip if it's not visible
			if (!pPoly->m_bVisible) continue;

			uint16_t materialID = pPoly->m_materialID;

			ULONG *indexData = leafBinIndices[materialID];

			uint16_t arrTexID = m_pTextureResources->getTexArrayIndex(pPoly->m_texID);

			size_t &indexOffset = leafBinDataIndexOffsets[materialID][arrTexID];

			// Retrieve the render data item for this leaf / attribute ID
			LeafBinTexture *pLeafBin = GetLeafBin(materialID);
			BSPTreeLeaf::RenderData *pRenderData = pLeaf->GetRenderData(materialID);

			// If we were unable to find a render data item yet built for this
			// leaf / material combination, add one.
			if (!pRenderData)
			{
				pRenderData = pLeaf->AddRenderData(materialID);
				if (!pRenderData) return false;

				// Store the leaf bin for later use
				pRenderData->pLeafBin = pLeafBin;

			} // End if no render data

			  // Search for a render element associated with the current texture ID
			size_t j;
			BSPTreeLeaf::RenderData::Element *pElement;
			for (j = 0; j < pRenderData->elements.size(); ++j)
			{
				pElement = &pRenderData->elements[j];
				if (pElement->texID == arrTexID) break; // pPoly->m_texID
			} // Next Element

			  // If we reached the end, then we found no element
			if (j == pRenderData->elements.size())
			{
				// Add a render data element for this vertex buffer
				pElement = pLeaf->AddRenderDataElement(materialID);
				pElement->IndexStart = indexOffset;
				pElement->PrimitiveCount = 0;
				pElement->texID = arrTexID; // pPoly->m_texID;
			} // End if no element for this VB

			// current offset in vertex buffer
			size_t &vertexOffset = leafBinVertexDataOffsets[materialID];
			// current vertex buffer
			byte *vertexData = leafBinVertexData[materialID];

			// Store triangle pre-requisites
			size_t FirstVertex = vertexOffset;
			size_t PreviousVertex = 0;

			// For each triangle in the set
			for (j = 0; j < pPoly->m_nVertexCount; ++j)
			{
				pPoly->m_pVertex[j].Normal = pPoly->m_normal;
				pPoly->m_pVertex[j].Tangent = pPoly->m_tangent;
				pPoly->m_pVertex[j].leafIndex = (uint32_t)pLeaf->m_index;
				// Add this vertex to the buffer
				m_pSceneMaterial->WriteVertex(vertexData, vertexOffset, pPoly->m_pVertex[j], m_pTextureResources->getTexIndexInTexArray(pPoly->m_texID));
				++vertexOffset;

				// Enough vertices added to start building triangle data?
				if (j >= 2)
				{
					// Add the index data
					indexData[indexOffset++] = (ULONG)FirstVertex;
					indexData[indexOffset++] = (ULONG)PreviousVertex;
					indexData[indexOffset++] = (ULONG)(vertexOffset - 1);

					// Update leaf element, we've added a primitive
					pElement->PrimitiveCount++;

				} // End if add triangle data

				// Update previous vertex
				PreviousVertex = vertexOffset - 1;

			} // Next Triangle

		} // Next Polygon


		if (m_renderPortals) 
		{
			// Loop through each of the polygons PORTALS of the leaf
			size_t nPortalsCount = pLeaf->m_Portals.size();

			pLeaf->m_portalIndexStart = portalIndicesOffset;
			pLeaf->m_portalPrimitiveCount = 0;

			for (size_t i = 0; i < nPortalsCount; ++i)
			{
				BSPTreePortal *pPortal = pLeaf->GetPortal(i);
				// add only front portals
				if (pPortal->mFrontOwner != pLeaf->m_index) continue;

				Polygon * pPoly = pPortal->mPolygon;

				// Skip if it's invalid
				if (!pPoly || pPoly->m_nVertexCount < 3) continue;

				// current offset in vertex buffer
				size_t &vertexOffset = portalsVerticesOffset;
				// current vertex buffer
				byte *vertexData = portalsVertices;

				// Store portal triangle pre-requisites
				size_t FirstVertex = vertexOffset;
				size_t PreviousVertex = 0;

				// For each portal triangle in the set
				for (size_t j = 0; j < pPoly->m_nVertexCount; ++j)
				{
					m_pBasicMaterial->WriteVertex(vertexData, vertexOffset, pPoly->m_pVertex[j]);
					++vertexOffset;
			
					// Enough vertices added to start building portal triangle data?
					if (j >= 2)
					{
						// Add the index data
						portalIndices[portalIndicesOffset++] = (ULONG)FirstVertex;
						portalIndices[portalIndicesOffset++] = (ULONG)PreviousVertex;
						portalIndices[portalIndicesOffset++] = (ULONG)(vertexOffset - 1);

						// Update leaf counter, we've added a primitive
						pLeaf->m_portalPrimitiveCount++;

					} // End if add triangle data

					  // Update previous vertex
					PreviousVertex = vertexOffset - 1;

				} // Next Portal Triangle

			}
		}

	} // Next Leaf

	// Build d3d buffers
	if (!CommitBuffers(leafBinVertexData,
		leafBinVertexSizes,
		leafBinIndices,
		leafBinIndexSizes,
		leafBinTextureIds)) {
		return false;
	}

	BuildLightsBuffer();

	if (m_renderPortals) {
		m_renderPortals = CommitPortalBuffers(portalsVertices, numPortalsVertices, portalIndices, numPortalsIndices);
	}
		
	// some final steps...
	for (auto LeafIterator = Leaves.begin(); LeafIterator != Leaves.end(); ++LeafIterator)
	{
		// set leafBinData pointers from leaves. Just to avoid search during visibility processing
		auto & LeafRenderData = (*LeafIterator)->m_renderData;

		// Loop through each renderable set in this leaf.
		for (size_t i = 0; i < LeafRenderData.size(); ++i)
		{

			BSPTreeLeaf::RenderData *pData = &LeafRenderData[i];
			LeafBinTexture *pLeafBin = pData->pLeafBin;

			// Loop through each element to render
			for (size_t j = 0; j < pData->elements.size(); ++j)
			{
				BSPTreeLeaf::RenderData::Element *pElement = &pData->elements[j];
				if (pElement->PrimitiveCount == 0) continue;
				
				LeafBinDataTexture *pLeafBinData = pLeafBin->GetBinData(pElement->texID);
				if (!pLeafBinData) return false;
				pElement->pLeafBinData = pLeafBinData;
			} // Next Element

		} // Next RenderData Item
		
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name : CalculatePolyBounds ()
// Desc : Calculates the bounding boxes of each polygon, used for optimization
//        steps later for T-Junction repair and possibly by the application
//-----------------------------------------------------------------------------
void Library::BSPEngine::BSPTree::CalculatePolyBounds()
{
	// Calculate polygon bounds
	for (size_t i = 0; i != m_Polygons.size(); ++i)
	{
		Polygon *pCurrentPoly = m_Polygons[i];
		if (!pCurrentPoly) continue;
		pCurrentPoly->ComputeBounds();
	}
}

void Library::BSPEngine::BSPTree::RepairTJunctions(Polygon * pPoly1, Polygon * pPoly2)
{
	XMFLOAT3 Delta;
	float       Percent;
	ULONG       v1, v2, v1a;
	Vertex     Vert1, Vert2, Vert1a;

	// Validate Parameters
	if (!pPoly1 || !pPoly2) return;

	// For each edge of this face
	for (v1 = 0; v1 < pPoly1->m_nVertexCount; v1++)
	{
		// Retrieve the next edge vertex (wraps to 0)
		v2 = ((v1 + 1) % pPoly1->m_nVertexCount);

		// Store vertices (Required because indices may change)
		Vert1 = pPoly1->m_pVertex[v1];
		Vert2 = pPoly1->m_pVertex[v2];

		// Now loop through each vertex in the test face
		for (v1a = 0; v1a < pPoly2->m_nVertexCount; v1a++)
		{
			// Store test point for easy access
			Vert1a = pPoly2->m_pVertex[v1a];

			// Test if this vertex is close to the test edge
			if (BSPEngine::DistanceToLineSegment((XMFLOAT3&)Vert1a, (XMFLOAT3&)Vert1, (XMFLOAT3&)Vert2) < 0.01f)
			{
				// Insert a new vertex within this edge
				long NewVert = pPoly1->InsertVertex((USHORT)v2);
				if (NewVert < 0) throw std::bad_alloc();

				// Set the vertex pos
				Vertex * pNewVert = &pPoly1->m_pVertex[NewVert];
				pNewVert->x = Vert1a.x;
				pNewVert->y = Vert1a.y; 
				pNewVert->z = Vert1a.z;

				// Calculate the percentage for interpolation calculations
				//Percent = D3DXVec3Length(&(*(D3DXVECTOR3*)pNewVert - (D3DXVECTOR3&)Vert1)) / D3DXVec3Length(&((D3DXVECTOR3&)Vert2 - (D3DXVECTOR3&)Vert1));
				XMFLOAT3 percentVec;
				XMStoreFloat3(&percentVec,
					XMVectorDivide(
						XMVector3Length(XMVectorSubtract(XMLoadFloat3(&(XMFLOAT3&)pNewVert),
							XMLoadFloat3(&(XMFLOAT3&)Vert1))),
						XMVector3Length(XMVectorSubtract(XMLoadFloat3(&(XMFLOAT3&)Vert2),
							XMLoadFloat3(&(XMFLOAT3&)Vert1)))));
				Percent = percentVec.x;

				// Interpolate texture coordinates
				Delta.x = Vert2.tu - Vert1.tu;
				Delta.y = Vert2.tv - Vert1.tv;
				pNewVert->tu = Vert1.tu + (Delta.x * Percent);
				pNewVert->tv = Vert1.tv + (Delta.y * Percent);

				// Interpolate normal
				//Delta = Vert2.Normal - Vert1.Normal;
				XMStoreFloat3(&Delta, XMVectorSubtract(XMLoadFloat3(&Vert2.Normal), XMLoadFloat3(&Vert1.Normal)));
				//pNewVert->Normal = Vert1.Normal + (Delta * Percent);
				XMStoreFloat3(&pNewVert->Normal, 
					XMVectorMultiplyAdd(XMLoadFloat3(&Delta), XMVectorReplicate(Percent), XMLoadFloat3(&Vert1.Normal)));
				//D3DXVec3Normalize(&pNewVert->Normal, &pNewVert->Normal);
				XMStoreFloat3(&pNewVert->Normal, XMVector3Normalize(XMLoadFloat3(&pNewVert->Normal)));
				// Update the edge for which we are testing
				Vert2 = *pNewVert;

			} // End if on edge

		} // Next Vertex v1a

	} // Next Vertex  v1
}

void Library::BSPEngine::BSPTree::UpdateMovingDoors(float timeScale)
{
	size_t i = 0;
	while(i != m_movingDoors.size())
	{
		Door *pDoor = m_movingDoors[i];

		pDoor->Update(timeScale);
		if (!pDoor->IsActive()) {
			m_movingDoors[i] = m_movingDoors.back();
			m_movingDoors.pop_back();
		}
		else {
			++i;
		}
	}
}

bool Library::BSPEngine::BSPTree::CheckDoorOpenCondition(Door * pDoor, bool testIfActive)
{
	if (testIfActive) {
		// check if door is already active
		for (size_t i = 0; i != m_movingDoors.size(); ++i)
		{
			if (m_movingDoors[i]->GetIndex() == pDoor->GetIndex()) {
				return false;
			}
		}
	}

	return pDoor->CheckOpenCondition(m_cachedCameraPos);

}


void Library::BSPEngine::BSPTree::InitD3DStates()
{
	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	D3D11_RASTERIZER_DESC rsDesc;

	/******************************************************/

	if (m_renderLeavesAABB)
	{
		ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dsDesc.DepthEnable = false;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		//dsDesc.DepthFunc = D3D11_COMPARISON_LESS; //D3D11_COMPARISON_GREATER;

		hr = m_pD3DDevice->CreateDepthStencilState(&dsDesc, &m_pAABBRenderDepthStencilState);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateDepthStencilState() failed", hr);
		}
	}
	/******************************************************/


	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	
	hr = m_pD3DDevice->CreateDepthStencilState(&dsDesc, &m_pWorldDepthStencilState);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateDepthStencilState() failed", hr);
	}

	/******************************************************/

	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	//rsDesc.DepthBias = 0;
	//rsDesc.SlopeScaledDepthBias = 0.005f;
	
	hr = m_pD3DDevice->CreateRasterizerState(&rsDesc, &m_pWorldRasterizerState);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateRasterizerState() failed", hr);
	}

	/******************************************************/

	if (m_renderPortals) {
		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_WIREFRAME;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.DepthClipEnable = true;
		rsDesc.AntialiasedLineEnable = true;

		hr = m_pD3DDevice->CreateRasterizerState(&rsDesc, &m_pBasicRasterizeState);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateRasterizerState() failed", hr);
		}
	}

}

void Library::BSPEngine::BSPTree::InitD3DQueryObjects()
{
	return; 
	/*
	D3D11_QUERY_DESC queryDesc;
	ZeroMemory(&queryDesc, sizeof(D3D11_QUERY_DESC));
	queryDesc.Query = D3D11_QUERY_OCCLUSION;
	for (auto leafIt = begin(m_Leaves); leafIt != end(m_Leaves); ++leafIt) {
		BSPTreeLeaf *pLeaf = *leafIt;
		m_pD3DDevice->CreateQuery(&queryDesc, &pLeaf->m_pQuery);
	}
	*/
}

void Library::BSPEngine::BSPTree::InitAABBRenderData()
{
	XMFLOAT3 Vertices[24];

	// Bottom 4 edges
	Vertices[0] = XMFLOAT3(-0.5f, -0.5f, -0.5f);
	Vertices[1]= XMFLOAT3(0.5f, -0.5f, -0.5f);

	Vertices[2] = XMFLOAT3(0.5f, -0.5f, -0.5f);
	Vertices[3] = XMFLOAT3(0.5f, -0.5f, 0.5f);

	Vertices[4] = XMFLOAT3(0.5f, -0.5f, 0.5f);
	Vertices[5] = XMFLOAT3(-0.5f, -0.5f, 0.5f);

	Vertices[6] = XMFLOAT3(-0.5f, -0.5f, 0.5f);
	Vertices[7] = XMFLOAT3(-0.5f, -0.5f, -0.5f);

	// Top 4 edges
	Vertices[8] = XMFLOAT3(-0.5f, 0.5f, -0.5f);
	Vertices[9] = XMFLOAT3(0.5f, 0.5f, -0.5f);

	Vertices[10] = XMFLOAT3(0.5f, 0.5f, -0.5f);
	Vertices[11] = XMFLOAT3(0.5f, 0.5f, 0.5f);

	Vertices[12] = XMFLOAT3(0.5f, 0.5f, 0.5f);
	Vertices[13] = XMFLOAT3(-0.5f, 0.5f, 0.5f);

	Vertices[14] = XMFLOAT3(-0.5f, 0.5f, 0.5f);
	Vertices[15] = XMFLOAT3(-0.5f, 0.5f, -0.5f);


	// 4 Side 'Struts'
	Vertices[16] = XMFLOAT3(-0.5f, -0.5f, -0.5f);
	Vertices[17] = XMFLOAT3(-0.5f, 0.5f, -0.5f);

	Vertices[18] = XMFLOAT3(0.5f, -0.5f, -0.5f);
	Vertices[19] = XMFLOAT3(0.5f, 0.5f, -0.5f);

	Vertices[20] = XMFLOAT3(0.5f, -0.5f, 0.5f);
	Vertices[21] = XMFLOAT3(0.5f, 0.5f, 0.5f);

	Vertices[22] = XMFLOAT3(-0.5f, -0.5f, 0.5f);
	Vertices[23] = XMFLOAT3(-0.5f, 0.5f, 0.5f);


	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = Vertices;
	m_pVBufferAABB = CreateD3DVertexBuffer(m_pD3DDevice, sizeof(Vertices), false, false, &vertexSubResourceData);
	
}

void Library::BSPEngine::BSPTree::InitSkyBox(Camera *pCamera)
{
	//m_pSkyBox = new SkyboxComponent(*m_pApp, Camera, L"Content\\Textures\\snowcube1024.dds", 5000.0f);
	//m_pSkyBox->Initialize();
}

bool Library::BSPEngine::BSPTree::CommitBuffers(std::map<size_t, byte*> & leafBinVertexData,
												std::map<size_t, size_t> &leafBinVertexSizes,
												std::map<size_t, ULONG*> &leafBinIndices,
												std::map<size_t, size_t> &leafBinIndexSizes,
												std::map<size_t, std::set<uint16_t>> &leafBinTextureIds)
{

	for (auto leafBinIt = std::cbegin(m_LeafBins); leafBinIt != std::cend(m_LeafBins); ++leafBinIt) {

		size_t materialID = leafBinIt->first;
		LeafBinTexture *leafBin = leafBinIt->second;

		// create vertex buffer
		size_t numVertices = leafBinVertexSizes[materialID]; // #vertices
		
		size_t numIndices = leafBinIndexSizes[materialID]; // #indices

		size_t numFinalVertices = numVertices; // leafBin->mMaterial->WeldBuffers(leafBinVertexData[materialID], numVertices, leafBinIndices[materialID], numIndices);
		if (numFinalVertices < 3) return false;

		leafBin->CreateVertexBuffer(m_pD3DDevice, leafBinVertexData[materialID], numFinalVertices * leafBin->mMaterial->VertexSize());
		delete[] leafBinVertexData[materialID];
		leafBinVertexData[materialID] = nullptr;

		// create index buffer
		bool use32BitIndices = ((numFinalVertices - 1) > 0xFFFF) ? true : false;
		
		leafBin->CreateIndexBuffer(m_pD3DDevice, leafBinIndices[materialID], numIndices, use32BitIndices);

		delete[] leafBinIndices[materialID];
		leafBinIndices[materialID] = nullptr;

		const auto& leafBinTexIds = leafBinTextureIds[materialID];
		// used for linked list of LeafBinDataTexture
		LeafBinDataTexture **leafBinDataPtrRef = &leafBin->m_pLeafBinDataTextureList;

		for (auto texIdsIt = std::cbegin(leafBinTexIds); texIdsIt != std::cend(leafBinTexIds); ++texIdsIt)
		{		
			uint16_t texArrID = *texIdsIt;

			// Allocate a new LeafBinData structure
			LeafBinDataTexture * pData = new LeafBinDataTexture(texArrID, m_pTextureResources->getTextureArrayDiffuseView(texArrID), m_pTextureResources->getTextureArrayNormalView(texArrID));  //getTextureItem(texID)->mTextureView);
			if (!pData) return false;

			// Add it to the leaf bin, so that the data is released if something goes wrong
			if (!leafBin->AddLeafBinData(pData)) { delete pData; return false; }

			// insert new LeafBinDataTexture at the end of the linked list
			*leafBinDataPtrRef = pData;
			leafBinDataPtrRef = &pData->m_next;
		}

	}

	// Success!
	return true;
}

bool Library::BSPEngine::BSPTree::BuildLightsBuffer()
{

	size_t numLeaves = m_Leaves.size();

	m_leafDataVec.clear();
	m_leafDataVec.resize(numLeaves);

	memset(&m_leafDataVec[0], 0, sizeof(LeafDataShader) * m_leafDataVec.size());

	for (size_t iLeaf = 0; iLeaf != numLeaves; ++iLeaf)
	{
		BSPTreeLeaf *pLeaf = m_Leaves[iLeaf];

		m_leafDataVec[iLeaf].ambient = XMFLOAT3(0.f, 0.f, 0.f);
		m_leafDataVec[iLeaf].activeLightsMask = pLeaf->m_activeLightsMask;
		
		for (size_t iLight = 0; iLight != MAX_LIGHTS_PER_LEAF; ++iLight)
		{
			Light *pLight = pLeaf->m_lights[iLight];
			if (pLight) {
				m_leafDataVec[iLeaf].lights[iLight].position = XMFLOAT3(pLight->origin[0], pLight->origin[1], pLight->origin[2]);
				m_leafDataVec[iLeaf].lights[iLight].intensity = pLight->intensity;
				m_leafDataVec[iLeaf].lights[iLight].color = pLight->color;	
				m_leafDataVec[iLeaf].lights[iLight].lightIndex = pLight->index;
				m_leafDataVec[iLeaf].lights[iLight].radius = pLight->radius;
			}
		}
	}

	D3D11_SUBRESOURCE_DATA bufferInitData;
	ZeroMemory((&bufferInitData), sizeof(bufferInitData));
	bufferInitData.pSysMem = m_leafDataVec.data();

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (UINT)(numLeaves * sizeof(LeafDataShader));
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(LeafDataShader);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	// Create the buffer with the specified configuration
	ID3D11Buffer* pBuffer = 0;
	HRESULT hr = m_pD3DDevice->CreateBuffer(&desc, &bufferInitData, &pBuffer);

	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateBuffer() failed", hr);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementWidth = (UINT)numLeaves;

	ID3D11ShaderResourceView *pSRV;
	hr = m_pD3DDevice->CreateShaderResourceView(pBuffer, &srvDesc, &pSRV);

	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateShaderResourceView() failed", hr);
	}

	m_pLeafDataBuffer = pBuffer;
	m_pLeafDataBufferSRV = pSRV;

	return true;
}

bool Library::BSPEngine::BSPTree::CommitPortalBuffers(byte * &vertices, size_t numVertices, ULONG * &indices, size_t numIndices)
{ 
	m_LeafBinPortals = new LeafBinPortal(this, m_pBasicMaterial, m_Leaves.size());

	// create vertex buffer
	size_t numFinalVertices = m_pBasicMaterial->WeldBuffers(vertices, numVertices, indices, numIndices);
	if (numFinalVertices < 3) return false;

	m_LeafBinPortals->CreateVertexBuffer(m_pD3DDevice, vertices, numFinalVertices * m_pBasicMaterial->VertexSize());

	delete[] vertices;
	vertices = nullptr;

	// create index buffer
	bool use32BitIndices = ((numFinalVertices - 1) > 0xFFFF) ? true : false;
	
	m_LeafBinPortals->CreateIndexBuffer(m_pD3DDevice, indices, numIndices, use32BitIndices);

	delete[] indices;
	indices = nullptr;

	return true;
}


//-----------------------------------------------------------------------------
// Name : PostBuild ()
// Desc : Function called by derived tree classes, once building has been 
//        completed.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::BSPTree::PostBuild()
{
	// Calculate the polygon bounding boxes
	CalculatePolyBounds();

	// Build the render data
	if (!BuildRenderData()) {
		return false;
	}

	for (auto it = begin(m_doors); it != end(m_doors); ++it) {
		if (!(*it)->BuildRenderData(this, m_pDoorMaterial)) {
			return false;
		}
	}

	m_movingDoors.clear();

	return true;
}

bool Library::BSPEngine::BSPTree::BuildTree(bspFileNode * pFileNode, BSPTreeNode * pNode)
{
	XMFLOAT3 vecMin, vecMax;

	// First time in?
	if (!pNode)
	{
		// Allocate a root node
		if (!(pNode = new BSPTreeNode)) return false;
		m_pRootNode = pNode;

	} // End if no node specified

	  // Build node data from that loaded from file
	pNode->Plane = m_pFilePlanes[pFileNode->PlaneIndex];
	pNode->BoundsMin = pFileNode->BoundsMin;
	pNode->BoundsMax = pFileNode->BoundsMax;

	// Allocate new node in front
	pNode->Front = new BSPTreeNode;
	if (!pNode->Front) return false;
	pNode->Front->Parent = pNode;
	
	// Node or leaf in front?
	if (pFileNode->FrontIndex >= 0)
	{
		// Build this new front node
		if (!BuildTree(&m_pFileNodes[pFileNode->FrontIndex], pNode->Front)) return false;

	} // End if node in front
	else
	{
		// Build a leaf
		BSPTreeLeaf * pLeaf = (BSPTreeLeaf*)m_Leaves[abs(pFileNode->FrontIndex + 1)];
		if (!pLeaf) return false;

		// Store pointer to leaf in the node
		pNode->Front->Leaf = pLeaf;
		// and pointer to node in the leaf
		pLeaf->m_pParentNode = pNode->Front;

		// Store the leaf's bounding box in the node
		pLeaf->GetBoundingBox(vecMin, vecMax);
		pNode->Front->BoundsMin = vecMin;
		pNode->Front->BoundsMax = vecMax;

	} // End if leaf in front

	  // Allocate new node behind
	pNode->Back = new BSPTreeNode;
	if (!pNode->Back) return false;
	pNode->Back->Parent = pNode;

	// Node or leaf behind?
	if (pFileNode->BackIndex >= 0)
	{
		// Build this new back node
		if (!BuildTree(&m_pFileNodes[pFileNode->BackIndex], pNode->Back)) return false;

	} // End if node in front
	else
	{
		// Solid leaf?
		if (pFileNode->BackIndex == BSP_SOLID_LEAF)
		{
			delete pNode->Back;
			pNode->Back = NULL;

		} // End if solid leaf
		else
		{
			// Retrieve the leaf specified
			BSPTreeLeaf * pLeaf = (BSPTreeLeaf*)m_Leaves[abs(pFileNode->BackIndex + 1)];
			if (!pLeaf) return false;

			// Store pointer to leaf in the node
			pNode->Back->Leaf = pLeaf;
			// and pointer to node in the leaf
			pLeaf->m_pParentNode = pNode->Back;

			// Store the leaf's bounding box in the node
			pLeaf->GetBoundingBox(vecMin, vecMax);
			pNode->Back->BoundsMin = vecMin;
			pNode->Back->BoundsMax = vecMax;

		} // End if empty back leaf

	} // End if leaf behind

	  // Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CollectLeavesAABBRecurse () (Private, Recursive)
// Desc : The actual recursive function which traverses the KD tree nodes in
//        order to build a list of intersecting leaves.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::BSPTree::CollectAABBRecurse(BSPTreeNode * pNode, LeafVector & List, const XMFLOAT3 & Min, const XMFLOAT3 & Max, bool bAutoCollect)
{
	bool bResult = false;

	// Validate parameters
	if (!pNode) return false;

	// Does the specified box intersect this node?
	if (!bAutoCollect && !Collision::AABBIntersectAABB(bAutoCollect, Min, Max, pNode->BoundsMin, pNode->BoundsMax)) return false;

	// Is there a leaf here, add it to the list
	if (pNode->Leaf) { List.push_back(pNode->Leaf); return true; }

	// Traverse down to children
	if (CollectAABBRecurse(pNode->Front, List, Min, Max, bAutoCollect)) bResult = true;
	if (CollectAABBRecurse(pNode->Back, List, Min, Max, bAutoCollect)) bResult = true;

	// Return the 'was anything added' result.
	return bResult;
}

//-----------------------------------------------------------------------------
// Name : CollectLeavesRayRecurse () (Private, Recursive)
// Desc : The actual recursive function which traverses the KD tree nodes in
//        order to build a list of intersecting leaves.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::BSPTree::CollectRayRecurse(BSPTreeNode * pNode, LeafVector & List, const XMFLOAT3 & RayOrigin, const XMFLOAT3 & Velocity)
{
	Collision::CLASSIFYTYPE PointA, PointB;
	XMFLOAT3 RayEnd, Intersection;
	bool        bResult;
	XMFLOAT4   Plane;
	float       t;


	// No operation if the node was null
	if (!pNode) return false;

	// If this node stores a leaf, just add it to the list and return
	if (pNode->Leaf)
	{
		// Add the leaf to the list
		List.push_back(pNode->Leaf);

		// We collected a leaf
		return true;

	} // End if stores a leaf

	  // Calculate the end point of the ray
	//RayEnd = RayOrigin + Velocity;
	XMStoreFloat3(&RayEnd, XMVectorAdd(XMLoadFloat3(&RayOrigin), XMLoadFloat3(&Velocity)));


	// Retrieve the plane, and classify the ray points against it
	Plane = pNode->Plane;
	PointA = Collision::PointClassifyPlane(RayOrigin, (XMFLOAT3&)Plane, Plane.w);
	PointB = Collision::PointClassifyPlane(RayEnd, (XMFLOAT3&)Plane, Plane.w);

	// Test for the combination of ray point positions
	if (PointA == Collision::CLASSIFY_ONPLANE && PointB == Collision::CLASSIFY_ONPLANE)
	{
		// Traverse down the front and back
		bResult = false;
		if (CollectRayRecurse(pNode->Front, List, RayOrigin, Velocity)) bResult = true;
		if (CollectRayRecurse(pNode->Back, List, RayOrigin, Velocity)) bResult = true;
		return bResult;

	} // End If both points on plane
	else if (PointA == Collision::CLASSIFY_INFRONT && PointB == Collision::CLASSIFY_BEHIND)
	{
		// The ray is spanning the plane, with the origin in front. Get the intersection point
		Collision::RayIntersectPlane(RayOrigin, Velocity, (XMFLOAT3&)Plane, Plane.w, t, true);
		//Intersection = RayOrigin + (Velocity * t);
		XMStoreFloat3(&Intersection, XMVectorMultiplyAdd(XMLoadFloat3(&Velocity), XMVectorReplicate(t), XMLoadFloat3(&RayOrigin)));

		// Traverse down both sides passing the relevant segments of the ray
		bResult = false;

		XMFLOAT3 tmp;
		XMStoreFloat3(&tmp, XMVectorSubtract(XMLoadFloat3(&Intersection), XMLoadFloat3(&RayOrigin)));
		if (CollectRayRecurse(pNode->Front, List, RayOrigin, tmp)) bResult = true;
		XMStoreFloat3(&tmp, XMVectorSubtract(XMLoadFloat3(&RayEnd), XMLoadFloat3(&Intersection)));
		if (CollectRayRecurse(pNode->Back, List, Intersection, tmp)) bResult = true;
		return bResult;

	} // End If Spanning with origin in front
	else if (PointA == Collision::CLASSIFY_BEHIND && PointB == Collision::CLASSIFY_INFRONT)
	{
		// The ray is spanning the plane, with the origin in front. Get the intersection point
		Collision::RayIntersectPlane(RayOrigin, Velocity, (XMFLOAT3&)Plane, Plane.w, t, true);
		//Intersection = RayOrigin + (Velocity * t);
		XMStoreFloat3(&Intersection, XMVectorMultiplyAdd(XMLoadFloat3(&Velocity), XMVectorReplicate(t), XMLoadFloat3(&RayOrigin)));


		// Traverse down both sides passing the relevant segments of the ray
		bResult = false;

		XMFLOAT3 tmp;
		XMStoreFloat3(&tmp, XMVectorSubtract(XMLoadFloat3(&RayEnd), XMLoadFloat3(&Intersection)));
		if (CollectRayRecurse(pNode->Front, List, Intersection, tmp)) bResult = true;
		XMStoreFloat3(&tmp, XMVectorSubtract(XMLoadFloat3(&Intersection), XMLoadFloat3(&RayOrigin)));
		if (CollectRayRecurse(pNode->Back, List, RayOrigin, tmp)) bResult = true;
		return bResult;

	} // End If Spanning with origin in front
	else if (PointA == Collision::CLASSIFY_INFRONT || PointB == Collision::CLASSIFY_INFRONT)
	{
		// Either of the points are in front (but not spanning), just pass down the front
		return CollectRayRecurse(pNode->Front, List, RayOrigin, Velocity);

	} // End if either point in front
	else
	{
		// Either of the points are behind (but not spanning), just pass down the back
		return CollectRayRecurse(pNode->Back, List, RayOrigin, Velocity);

	} // End if either point behind
}

void Library::BSPEngine::BSPTree::SortLeavesFrontToBackRecurse(BSPTreeNode * pNode)
{
	if (!pNode || pNode->m_nVisCounter != m_nVisCounter) 
	{
		return;
	}

	// Return the leaf if we have found a leaf node
	if (pNode->Leaf) 
	{
		AddVisibleLeaf(pNode->Leaf);
		return;
	}

	// Classify the point against this node plane
	Collision::CLASSIFYTYPE Location = Collision::PointClassifyPlane(m_cachedCameraPos, (XMFLOAT3&)pNode->Plane, pNode->Plane.w);

	BSPTreeNode *pNodeNear, *pNodeFar;

	if (Location == Collision::CLASSIFY_BEHIND) 
	{
		pNodeNear = pNode->Back;
		pNodeFar = pNode->Front;
	}
	else {
		pNodeNear = pNode->Front;
		pNodeFar = pNode->Back;
	}

	SortLeavesFrontToBackRecurse(pNodeNear);
	SortLeavesFrontToBackRecurse(pNodeFar);

}

void Library::BSPEngine::BSPTree::ReleaseFileData()
{
	// Destroy arrays
	if (m_pFileNodes) delete[]m_pFileNodes;
	if (m_pFilePlanes) delete[]m_pFilePlanes;

	// Clear Variables
	m_pFileNodes = NULL;
	m_pFilePlanes = NULL;
	m_nFileNodeCount = 0;
	m_nFilePlaneCount = 0;
}

//-----------------------------------------------------------------------------
// Name : FindLeaf ()
// Desc : Determine the leaf into which the specified point falls.
//-----------------------------------------------------------------------------
BSPTreeLeaf * Library::BSPEngine::BSPTree::FindLeaf(const XMFLOAT3 & Position)
{
	Collision::CLASSIFYTYPE Location;
	BSPTreeNode * pNode = m_pRootNode;

	// Since a point can only exist on one side of a plane (not both)
	// there is no need for us to implement a recursive process. Just
	// keep searching until we pop out in the correct leaf.
	for (;;)
	{
		// No node?
		if (!pNode) break;

		// Return the leaf if we have found a leaf node
		if (pNode->Leaf) return pNode->Leaf;

		// Classify the point against this node plane
		Location = Collision::PointClassifyPlane(Position, (XMFLOAT3&)pNode->Plane, pNode->Plane.w);

		// Select the correct node
		switch (Location)
		{
		case Collision::CLASSIFY_ONPLANE:
		case Collision::CLASSIFY_INFRONT:

			// Select front node
			pNode = pNode->Front;
			break;

		case Collision::CLASSIFY_BEHIND:

			// Select back node
			pNode = pNode->Back;
			break;

		} // End Switch

	} // Next Iteration

	  // No leaf found
	return nullptr;
}

/*
void Library::BSPEngine::BSPTree::ResetRenderBinData()
{
	// Iterate through the leaf bins and destroy them
	for (auto BinIterator = begin(m_LeafBins); BinIterator != end(m_LeafBins); ++BinIterator)
	{
		LeafBinTexture * pBin = BinIterator->second;
		if (!pBin) continue;

		pBin->ResetBinData();

	} // Next Leaf Bin

}
*/

Library::BSPEngine::LeafBinTexture::LeafBinTexture(BSPTree *pBspTree, size_t materialID, SceneMaterial *material, size_t nLeafCount)
	: LeafBin(pBspTree, materialID, nLeafCount)
{
	mMaterial = material;
	m_pLeafBinDataTextureList = nullptr;
	m_pLeafBinDataTexListPtr = &m_pLeafBinDataTextureList;
}

Library::BSPEngine::LeafBinTexture::~LeafBinTexture()
{
	DeleteObject(mMaterial);

	// Loop through each bin data item
	for (auto it = begin(m_LeafBinData); it != end(m_LeafBinData); ++it)
	{
		LeafBinData *item = it->second;
		delete item;
		it->second = nullptr;
	} // Next Data Item

	m_pLeafBinDataTextureList = nullptr;
	m_pLeafBinDataTexListPtr = nullptr;
}

void Library::BSPEngine::LeafBinTexture::ResetBinData()
{
	// For each leafBinData
	for (auto it = begin(m_LeafBinData); it != end(m_LeafBinData); ++it)
	{
		LeafBinDataTexture * pData = it->second;
		//if (!pData) continue;

		// Reset the batch count
		pData->clearBatches();

	} // Next Buffer

	// reset ppointer to list to concatenate leaf bins texture for current frame
	m_pLeafBinDataTexListPtr = &m_pLeafBinDataTextureList;
}

void Library::BSPEngine::LeafBinTexture::CreateVertexBuffer(ID3D11Device *D3DDevice, const byte *pMem, size_t numBytes)
{
	mMaterial->CreateVertexBuffer(D3DDevice, pMem, numBytes, &m_pVertexBuffer);
}

LeafBinDataTexture * Library::BSPEngine::LeafBinTexture::GetBinData(uint16_t texID) const
{
	auto it = m_LeafBinData.find(texID);
	// Retrieve the item
	if (it != std::end(m_LeafBinData)) {
		return it->second;
	}
	return nullptr;
}

void Library::BSPEngine::LeafBinTexture::AddVisibleDataTexture(LeafBinDataTexture * pData, size_t IndexStart, size_t PrimitiveCount, size_t visCounter)
{
	// check to see if the leaf bin data must be added to the tex bin data list
	if (pData->m_nVisCounter != visCounter) {
		pData->m_nVisCounter = visCounter;
		pData->clearBatches();
		*m_pLeafBinDataTexListPtr = pData;
		m_pLeafBinDataTexListPtr = &pData->m_next;
	}

	LeafBin::AddVisibleData(pData, IndexStart, PrimitiveCount);
}

//-----------------------------------------------------------------------------
// Name : AddLeafBinData ()
// Desc : Append a new CLeafBinData object to the end of our internal list.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::LeafBinTexture::AddLeafBinData(LeafBinData * pData)
{
	LeafBinDataTexture *pTexData = dynamic_cast<LeafBinDataTexture *>(pData);
	if (!pTexData) return false;

	m_LeafBinData[pTexData->getTexID()] = pTexData;

	// Allocate enough space in the render queue. Remember that the
	// worst case is that our queue will contain batches where one leaf is visible
	// and the next is invisible, repeating over the entire set. This means that
	// the highest number of batches we need to allocate is equal to the leaf count / 2
	// Note: We add '1' to ensure that we don't have any rounding issues.
	pTexData->reserveBatches((m_nLeafCount + 1) / 2);
	// Success!
	return true;
}


Library::BSPEngine::LeafBinDataTexture::LeafBinDataTexture(uint16_t texID, ID3D11ShaderResourceView * TextureView, ID3D11ShaderResourceView * TextureNormalView)
{
	mTexID = texID;
	mTextureView = TextureView;
	mTextureNormalView = TextureNormalView;
	m_nVisCounter = 0;
	m_next = nullptr;
}

Library::BSPEngine::LeafBinDataTexture::~LeafBinDataTexture()
{
	mTextureView = nullptr;
	mTextureNormalView = nullptr;
	m_next = nullptr;
}


//-----------------------------------------------------------------------------
// Name : Render ()
// Desc : Render the contents of the dynamic index buffer.
//-----------------------------------------------------------------------------
void Library::BSPEngine::LeafBinTexture::Render(ID3D11DeviceContext* pDeviceContext, CXMMATRIX wvpMat)
{
	mMaterial->Apply(pDeviceContext);

	// set vertex buffer
	UINT stride = (UINT)mMaterial->VertexSize();
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// set index buffer
	DXGI_FORMAT indexFormat = m_b32BitIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, indexFormat, 0);

	// set material wvp variable
	mMaterial->SetCBufferVS(pDeviceContext, wvpMat);
	mMaterial->SetCBufferPS(pDeviceContext, m_pBspTree->m_cachedCameraPos, m_pBspTree->m_clusterMapSize);

	// set lightmap & cluster maps
	mMaterial->SetLightmapClusterMapsTextureArrayView(pDeviceContext, m_pBspTree->m_pTextureResources->getLightMapsArrayView(), m_pBspTree->m_pTextureResources->getClusterMapsArrayView());
	mMaterial->SetLeafDataView(pDeviceContext, m_pBspTree->m_pLeafDataBufferSRV);

	// end current leaf bin texture list
	*m_pLeafBinDataTexListPtr = nullptr;

	// Loop through each leaf bin texture

	for (LeafBinDataTexture *TextureArrBinDataPtr = m_pLeafBinDataTextureList;
		TextureArrBinDataPtr;
		TextureArrBinDataPtr = TextureArrBinDataPtr->m_next)
	{
		LeafBinDataTexture * pData = TextureArrBinDataPtr;

		// Commit any last piece of data to the render queue
		//AddVisibleData(pData, 0, 0);
		pData->coalesceBatches();

		// set current diffuse&normal texture maps
		mMaterial->SetDiffuseAndNormalTextureArrayViews(pDeviceContext, pData->getTextureView(), pData->getTextureNormalView());

		// Render leaves batches finally
		for (size_t j = 0, numBatches = pData->getRenderBatchCount();
			j != numBatches;
			++j)
		{
			LeafBinData::RenderBatch & Batch = pData->getRenderBatch(j);

			// Render any data
			pDeviceContext->DrawIndexed((UINT)Batch.PrimitiveCount * 3, (UINT)Batch.IndexStart, 0);

		}  // Next element

		// reset leaf bin data vis counter in case of rendering each leaf per se
		pData->m_nVisCounter = 0;
	} // Next Vertex Buffer

	// reset leaf bin texture ppointer
	m_pLeafBinDataTexListPtr = &m_pLeafBinDataTextureList;
	*m_pLeafBinDataTexListPtr = nullptr;
}

Library::BSPEngine::LeafBinPortal::LeafBinPortal(BSPTree *pBspTree, BasicMaterial * material, size_t nLeafCount)
	: LeafBin(pBspTree, 0, nLeafCount)
{
	mMaterial = material;
}

Library::BSPEngine::LeafBinPortal::~LeafBinPortal()
{
	DeleteObject(mMaterial);
}

void Library::BSPEngine::LeafBinPortal::ResetBinData()
{
	m_LeafBinData.clearBatches();
}

void Library::BSPEngine::LeafBinPortal::CreateVertexBuffer(ID3D11Device * D3DDevice, const byte * pMem, size_t numBytes)
{
	mMaterial->CreateVertexBuffer(D3DDevice, pMem, numBytes, &m_pVertexBuffer);
}

void Library::BSPEngine::LeafBinPortal::Render(ID3D11DeviceContext * pDeviceContext, CXMMATRIX tMat, CXMVECTOR color)
{
	mMaterial->Apply(pDeviceContext);

	// set vertex buffer
	UINT stride = (UINT)mMaterial->VertexSize();
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// set index buffer
	DXGI_FORMAT indexFormat = m_b32BitIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, indexFormat, 0);

	// update material wvp variable
	mMaterial->Update(pDeviceContext, tMat, color);

	m_LeafBinData.coalesceBatches();

	// Render the leaves
	for (size_t j = 0; j < m_LeafBinData.getRenderBatchCount(); ++j)
	{
		LeafBinData::RenderBatch & Batch = m_LeafBinData.getRenderBatch(j);
		
		// Render any data
		pDeviceContext->DrawIndexed((UINT)Batch.PrimitiveCount * 3, (UINT)Batch.IndexStart, 0);
	}
}

bool Library::BSPEngine::LeafBinPortal::AddLeafBinData(LeafBinData * pData)
{
	return false;
}

void Library::BSPEngine::LeafBinPortal::AddVisibleData(size_t IndexStart, size_t PrimitiveCount)
{
	m_LeafBinData.addBatch(IndexStart, PrimitiveCount);
}

Library::BSPEngine::BSPTreePortal::BSPTreePortal()
{
	mPolygon = nullptr;
}

Library::BSPEngine::BSPTreePortal::~BSPTreePortal()
{
	delete mPolygon;
}

Library::BSPEngine::LeafBinEntity::LeafBinEntity(BSPTree * pBspTree, size_t materialID, DoorMaterial * material, size_t nSize)
	: LeafBin(pBspTree, materialID, nSize)
{
	mMaterial = material;
}

Library::BSPEngine::LeafBinEntity::~LeafBinEntity()
{
	DeleteObject(mMaterial);

	// Loop through each bin data item
	for (auto it = begin(m_LeafBinData); it != end(m_LeafBinData); ++it)
	{
		LeafBinData *item = it->second;
		delete item;
		it->second = nullptr;
	} // Next Data Item

}

void Library::BSPEngine::LeafBinEntity::ResetBinData()
{
	// For each leafBinData
	for (auto it = begin(m_LeafBinData); it != end(m_LeafBinData); ++it)
	{
		LeafBinDataTexture * pData = it->second;
		//if (!pData) continue;

		// Reset the batch count
		pData->clearBatches();

	} // Next Buffer

}

void Library::BSPEngine::LeafBinEntity::CreateVertexBuffer(ID3D11Device * D3DDevice, const byte * pMem, size_t numBytes)
{
	mMaterial->CreateVertexBuffer(D3DDevice, pMem, numBytes, &m_pVertexBuffer);
}

bool Library::BSPEngine::LeafBinEntity::AddLeafBinData(LeafBinData * pData)
{
	LeafBinDataTexture *pTexData = dynamic_cast<LeafBinDataTexture *>(pData);
	if (!pTexData) return false;

	m_LeafBinData[pTexData->getTexID()] = pTexData;

	// Allocate enough space in the render queue. Remember that the
	// worst case is that our queue will contain batches where one leaf is visible
	// and the next is invisible, repeating over the entire set. This means that
	// the highest number of batches we need to allocate is equal to the leaf count / 2
	// Note: We add '1' to ensure that we don't have any rounding issues.
	pTexData->reserveBatches((m_nLeafCount + 1) / 2);
	// Success!
	return true;
}

void Library::BSPEngine::LeafBinEntity::Render(ID3D11DeviceContext * pDeviceContext, CXMMATRIX wvpMat)
{
	mMaterial->Apply(pDeviceContext);

	// set vertex buffer
	UINT stride = (UINT)mMaterial->VertexSize();
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// set index buffer
	DXGI_FORMAT indexFormat = m_b32BitIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, indexFormat, 0);

	// set material wvp variable
	mMaterial->SetCBufferVS(pDeviceContext, wvpMat);
	mMaterial->SetCBufferPS(pDeviceContext, m_pBspTree->m_cachedCameraPos, m_pBspTree->m_clusterMapSize);

	// set lightmap & cluster maps

	// render
	for (auto it = std::begin(m_LeafBinData); it != std::end(m_LeafBinData); ++it) 
	{
		LeafBinDataTexture * pData = it->second;

		pData->coalesceBatches();

		// set current diffuse & normal texture maps
		mMaterial->SetDiffuseAndNormalTextureArrayViews(pDeviceContext, pData->getTextureView(), pData->getTextureNormalView());
		
		// Render leaves batches finally
		for (size_t j = 0, numBatches = pData->getRenderBatchCount();
			j != numBatches;
			++j)
		{
			LeafBinData::RenderBatch & Batch = pData->getRenderBatch(j);
			
			// Render any data
			pDeviceContext->DrawIndexed((UINT)Batch.PrimitiveCount * 3, (UINT)Batch.IndexStart, 0);

		}  // Next element

		pData->clearBatches();
	}
}

LeafBinDataTexture * Library::BSPEngine::LeafBinEntity::GetBinData(uint16_t texID) const
{
	auto it = m_LeafBinData.find(texID);
	// Retrieve the item
	if (it != std::end(m_LeafBinData)) {
		return it->second;
	}
	return nullptr;
}

Library::BSPEngine::BrushEntity::BrushEntity()
{
	XMStoreFloat4x4(&m_model,
		XMMatrixIdentity());
	m_pBspTree = nullptr;
	m_LastFrustumPlane = -1;
}

Library::BSPEngine::BrushEntity::~BrushEntity()
{
	// ...
}

void Library::BSPEngine::BrushEntity::Render(ID3D11DeviceContext * pDeviceContext, CXMMATRIX vpMat)
{
	CXMMATRIX mvpMat =	XMMatrixMultiply(XMLoadFloat4x4(&m_model), vpMat);

	// Loop through each renderable set in this door.
	for (size_t i = 0; i < m_renderData.size(); ++i)
	{
		RenderData *pData = &m_renderData[i];
		LeafBinEntity *pLeafBin = pData->pLeafBin;

		// Loop through each element texture to render
		for (size_t j = 0; j < pData->elements.size(); ++j)
		{
			RenderData::Element *pElement = &pData->elements[j];
			if (pElement->PrimitiveCount == 0) continue;

			LeafBinDataTexture * pBinData = pLeafBin->m_LeafBinData[pElement->texID];//pLeafBin->GetBinData(pElement->texID);
																					 //if (!pData) continue;																				// Add this to the leaf bin
			pLeafBin->AddVisibleData(pBinData, pElement->IndexStart, pElement->PrimitiveCount);

		} // Next Element

		  // then render
		pLeafBin->Render(pDeviceContext, mvpMat);


	} // Next RenderData Item

}

bool Library::BSPEngine::BrushEntity::BuildRenderData(BSPTree * pBspTree, DoorMaterial * pDoorMaterial)
{
	m_pBspTree = pBspTree;

	std::map<size_t, size_t> leafBinVertexSizes; // num vertices for each material/shader
	std::map<size_t, byte *> leafBinVertexData; // vertex buffer for each material/shader
	std::map<size_t, size_t> leafBinVertexDataOffsets; // vertex byte offsets for each material/shader

	std::map<size_t, ULONG *> leafBinIndices; // indices buffer for each material/shader
	std::map<size_t, size_t> leafBinIndexSizes; // num indices for each material/shader
	std::map<size_t, std::set<uint16_t>> leafBinTextureIds; // texture array ids for each leafBin

	std::map<size_t, std::map<uint16_t, size_t>> leafBinDataIndexSizes; // num indices for each (material/shader, texture array)
	std::map<size_t, std::map<uint16_t, size_t>> leafBinDataIndexOffsets; // indices offsets for each (material/shader, texture array)

																		  // Iterate through all of the polygons												  // Iterate through all of the polygons

	size_t nPolygonCount = m_pPolygons.size();

	for (size_t i = 0; i < nPolygonCount; ++i) {

		Polygon * pPoly = m_pPolygons[i];

		// Skip if it's invalid
		if (!pPoly || pPoly->m_nVertexCount < 3) continue;

		/******************************************************/
		// compute poly bounds
		XMFLOAT3 & Min = pPoly->m_vecBoundsMin;
		XMFLOAT3 & Max = pPoly->m_vecBoundsMax;

		// Reset bounding box
		Min = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
		Max = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		// Build polygon bounds
		for (int i = 0; i < pPoly->m_nVertexCount; ++i)
		{
			Vertex & vtx = pPoly->m_pVertex[i];
			if (vtx.x < Min.x) Min.x = vtx.x;
			if (vtx.y < Min.y) Min.y = vtx.y;
			if (vtx.z < Min.z) Min.z = vtx.z;
			if (vtx.x > Max.x) Max.x = vtx.x;
			if (vtx.y > Max.y) Max.y = vtx.y;
			if (vtx.z > Max.z) Max.z = vtx.z;

		} // Next Vertex

		// Increase bounds slightly to relieve this operation during intersection testing
		Min.x -= 0.1f; Min.y -= 0.1f; Min.z -= 0.1f;
		Max.x += 0.1f; Max.y += 0.1f; Max.z += 0.1f;
		/******************************************************/

		// Skip if it's not visible
		if (!pPoly->m_bVisible) continue;

		uint16_t materialID = pPoly->m_materialID;

		// Retrieve the leaf bin for this attribute
		LeafBinEntity *pLeafBin = m_leafBins[materialID];

		// Determine if there is a matching bin already in existence
		if (!pLeafBin)
		{
			// Allocate a new leaf bin

			pLeafBin = new LeafBinEntity(m_pBspTree, materialID, pDoorMaterial, 1);
			if (!pLeafBin) return false;

			// Add to the leaf bin list
			m_leafBins[materialID] = pLeafBin;

		} // End if no bin existing

		  // Change from texId to texArrayId
		uint16_t arrTexID = m_pBspTree->m_pTextureResources->getTexArrayIndex(pPoly->m_texID);

		leafBinTextureIds[materialID].insert(arrTexID); // pPoly->m_texID

														// Total up vertices for current material
		leafBinVertexSizes[materialID] += pPoly->m_nVertexCount;

		// Count also #indices
		size_t numIndices = (pPoly->m_nVertexCount - 2) * 3;
		leafBinDataIndexSizes[materialID][arrTexID] += numIndices;
		leafBinIndexSizes[materialID] += numIndices;

	} // Next Polygon 

	  // compute index offsets for each (material, texId)

	for (auto leafBinIndexSizesIt = cbegin(leafBinDataIndexSizes);
		leafBinIndexSizesIt != cend(leafBinDataIndexSizes);
		++leafBinIndexSizesIt) {

		// reserve memory for vertex buffer
		size_t vertexNum = leafBinVertexSizes[leafBinIndexSizesIt->first];
		leafBinVertexData[leafBinIndexSizesIt->first] = new byte[vertexNum * pDoorMaterial->VertexSize()];

		// set also vertex offset to 0
		leafBinVertexDataOffsets[leafBinIndexSizesIt->first] = 0;

		// reserve memory for index  buffer
		size_t numIndices = leafBinIndexSizes[leafBinIndexSizesIt->first];
		leafBinIndices[leafBinIndexSizesIt->first] = new ULONG[numIndices];

		//b32BitIndices[leafBinIndexSizesIt->first] = ((vertexNum - 1) > 0xFFFF) ? true : false;

		// adjust offsets for each texId
		auto &leafBinDataIndexSizes = leafBinIndexSizesIt->second;
		size_t offset = 0;

		for (auto indexSizeIt = cbegin(leafBinDataIndexSizes);
			indexSizeIt != cend(leafBinDataIndexSizes);
			++indexSizeIt)
		{
			leafBinDataIndexOffsets[leafBinIndexSizesIt->first][indexSizeIt->first] = offset;
			offset += indexSizeIt->second;
		}

	}

	// Loop through each of the polygons of the door
	for (size_t i = 0; i < nPolygonCount; ++i)
	{
		Polygon * pPoly = m_pPolygons[i];

		// Skip if it's invalid
		if (!pPoly || pPoly->m_nVertexCount < 3) continue;

		// Skip if it's not visible
		if (!pPoly->m_bVisible) continue;

		uint16_t materialID = pPoly->m_materialID;

		ULONG *indexData = leafBinIndices[materialID];

		uint16_t arrTexID = m_pBspTree->m_pTextureResources->getTexArrayIndex(pPoly->m_texID);

		size_t &indexOffset = leafBinDataIndexOffsets[materialID][arrTexID];


		LeafBinEntity *pLeafBin = m_leafBins.find(materialID)->second;
		RenderData *pRenderData = nullptr;

		// Search for the correct render data item
		for (size_t i = 0; i < m_renderData.size(); ++i)
		{
			if (m_renderData[i].materialID == materialID) {
				pRenderData = &m_renderData[i];
				break;
			}
		} // Next RenderData Item

		  // If we were unable to find a render data item yet built for this
		  // leaf / material combination, add one.
		if (!pRenderData)
		{
			// add render data
			m_renderData.push_back(RenderData());

			// Get the element we just created
			RenderData *pData = &m_renderData[m_renderData.size() - 1];

			// Clear the new entry
			ZeroMemory(pData, sizeof(RenderData));

			// Store the attribute ID, it's used to look up the render data later
			pData->materialID = materialID;

			pRenderData = pData;
			if (!pRenderData) return false;

			// Store the leaf bin for later use
			pRenderData->pLeafBin = pLeafBin;

		} // End if no render data

		  // Search for a render element associated with the current texture ID
		size_t j;
		RenderData::Element *pElement;
		for (j = 0; j < pRenderData->elements.size(); ++j)
		{
			pElement = &pRenderData->elements[j];
			if (pElement->texID == arrTexID) break; // pPoly->m_texID
		} // Next Element

		  // If we reached the end, then we found no element
		if (j == pRenderData->elements.size())
		{
			// Add a render data element for this vertex buffer
			pRenderData->elements.push_back(RenderData::Element());
			// Get a pointer to the new element
			pElement = &pRenderData->elements[pRenderData->elements.size() - 1];

			// Clear the new entry
			ZeroMemory(pElement, sizeof(RenderData::Element));

			pElement->IndexStart = indexOffset;
			pElement->PrimitiveCount = 0;
			pElement->texID = arrTexID; // pPoly->m_texID;
		} // End if no element for this VB

		  // current offset in vertex buffer
		size_t &vertexOffset = leafBinVertexDataOffsets[materialID];
		// current vertex buffer
		byte *vertexData = leafBinVertexData[materialID];

		// Store triangle pre-requisites
		size_t FirstVertex = vertexOffset;
		size_t PreviousVertex = 0;

		// For each triangle in the set
		for (j = 0; j < pPoly->m_nVertexCount; ++j)
		{
			pPoly->m_pVertex[j].Normal = pPoly->m_normal;
			pPoly->m_pVertex[j].Tangent = pPoly->m_tangent;

			// Add this vertex to the buffer
			pDoorMaterial->WriteVertex(vertexData, vertexOffset, pPoly->m_pVertex[j], m_pBspTree->m_pTextureResources->getTexIndexInTexArray(pPoly->m_texID));
			++vertexOffset;

			// Enough vertices added to start building triangle data?
			if (j >= 2)
			{
				// Add the index data
				indexData[indexOffset++] = (ULONG)FirstVertex;
				indexData[indexOffset++] = (ULONG)PreviousVertex;
				indexData[indexOffset++] = (ULONG)(vertexOffset - 1);

				// Update leaf element, we've added a primitive
				pElement->PrimitiveCount++;

			} // End if add triangle data

			  // Update previous vertex
			PreviousVertex = vertexOffset - 1;

		} // Next Triangle

	}


	for (auto leafBinIt = std::cbegin(m_leafBins); leafBinIt != std::cend(m_leafBins); ++leafBinIt) {

		size_t materialID = leafBinIt->first;
		LeafBinEntity *leafBin = leafBinIt->second;

		// create vertex buffer
		size_t numVertices = leafBinVertexSizes[materialID]; // #vertices

		size_t numIndices = leafBinIndexSizes[materialID]; // #indices

		size_t numFinalVertices = numVertices;// leafBin->mMaterial->WeldBuffers(leafBinVertexData[materialID], numVertices, leafBinIndices[materialID], numIndices);
		if (numFinalVertices < 3) return false;

		leafBin->CreateVertexBuffer(m_pBspTree->m_pD3DDevice, leafBinVertexData[materialID], numFinalVertices * leafBin->mMaterial->VertexSize());
		delete[] leafBinVertexData[materialID];
		leafBinVertexData[materialID] = nullptr;

		// create index buffer
		bool use32BitIndices = ((numFinalVertices - 1) > 0xFFFF) ? true : false;

		leafBin->CreateIndexBuffer(m_pBspTree->m_pD3DDevice, leafBinIndices[materialID], numIndices, use32BitIndices);

		delete[] leafBinIndices[materialID];
		leafBinIndices[materialID] = nullptr;

		const auto& leafBinTexIds = leafBinTextureIds[materialID];

		for (auto texIdsIt = std::cbegin(leafBinTexIds); texIdsIt != std::cend(leafBinTexIds); ++texIdsIt)
		{
			uint16_t texArrID = *texIdsIt;

			// Allocate a new LeafBinData structure
			LeafBinDataTexture * pData = new LeafBinDataTexture(texArrID, m_pBspTree->m_pTextureResources->getTextureArrayDiffuseView(texArrID), m_pBspTree->m_pTextureResources->getTextureArrayNormalView(texArrID));  //getTextureItem(texID)->mTextureView);
			if (!pData) return false;

			// Add it to the leaf bin, so that the data is released if something goes wrong
			if (!leafBin->AddLeafBinData(pData)) { delete pData; return false; }

		}


	}

	m_isMarked = false;

	return true;
}

void Library::BSPEngine::BrushEntity::GetBoundingBox(XMFLOAT3 & Min, XMFLOAT3 & Max) const
{
	// Retrieve the bounding boxes
	Min = m_boundsMin;
	Max = m_boundsMax;
}
