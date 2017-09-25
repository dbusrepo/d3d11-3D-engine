#include "CLevelFile.h"

#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"
#include <cstdint>
#include "CBSPTree.h"
#include "..\\Support Source\\CPlane.h"

CLevelFile::CLevelFile()
{
	
}

CLevelFile::~CLevelFile()
{
}

void CLevelFile::Load(LPCTSTR FileName)
{
	m_vpMeshList.clear();
	m_mapEntities = 0;
	m_mapTextures = 0;

	if (!m_mapParser.Load(FileName, &m_mapEntities, &m_mapTextures)) {
		throw BCERR_FILENOTFOUND;
	}

	SearchNoDrawTexture();
	GenerateMeshes();
	LoadPlayerInfo();
	LoadLights();
}

void CLevelFile::Save(LPCTSTR FileName, CBSPTree * pTree)
{
	// void CFileIWF::Save( LPCTSTR FileName, CBSPTree * pTree )
	FILE *outFile;

	if ((outFile = fopen(FileName, "wb")) == 0) {
		throw BCERR_FILENOTOPEN;
		//printf("%s file creation failed...\n", FileName);
		//printf("Exiting...\n");
		//exit(EXIT_FAILURE);
	}

	// textures
	SaveTextures(outFile);

	// materials
	// shaders

	// meshes
	SaveMeshes(outFile);

	// bsp tree
	SaveTree(outFile, pTree);
	
	// door meshes
	SaveDoorMeshes(outFile);

	// entities
	SavePlayerInfo(outFile);

	// lights
	SaveLights(outFile);

	fclose(outFile);

	//SaveLightMapGenInfo(); //not used anymore

}

void CLevelFile::SaveMeshes(FILE * file)
{
	// (m_vpMeshList[0] is always the BSP tree mesh if tree data is available)

	uint32_t numMeshes = 0;
	uint32_t totalVertexCount = 0;

	for (size_t i = 0; i < m_vpMeshList.size(); i++)
	{
		if (m_vpMeshList[i] && m_vpMeshList[i]->FaceCount > 0)
		{
			++numMeshes;
		}
	}

	fwrite(&numMeshes, sizeof(uint32_t), 1, file);

	for (size_t i = 0; i < m_vpMeshList.size(); i++)
	{
		if (m_vpMeshList[i] && m_vpMeshList[i]->FaceCount > 0)
		{
			CMesh *mesh = m_vpMeshList[i];

			// write mesh name
			const char *meshName = mesh->Name;
			uint8_t charCount = (uint8_t)strlen(meshName);
			fwrite(&charCount, sizeof(uint8_t), 1, file);
			fwrite(meshName, 1, charCount, file);

			// aabb
			fwrite(&mesh->Bounds, sizeof(CBounds3), 1, file);

			// Write number of surfaces
			uint32_t faceCount = mesh->FaceCount;
			fwrite(&faceCount, sizeof(uint32_t), 1, file);

			// Loop through each surface
			for (uint32_t i = 0; i < faceCount; i++)
			{
				CFace * pFace = mesh->Faces[i];
				uint16_t texId = pFace->TextureIndex;
				fwrite(&pFace->Normal, sizeof(CVector3), 1, file);
				fwrite(&texId, sizeof(uint16_t), 1, file);
				
				totalVertexCount += pFace->VertexCount;
			}

			fwrite(&totalVertexCount, sizeof(uint32_t), 1, file);

			for (uint32_t i = 0; i < faceCount; i++)
			{
				CFace * pFace = mesh->Faces[i];
				uint16_t faceVertexCount = (uint16_t)pFace->VertexCount;
				fwrite(&faceVertexCount, sizeof(uint16_t), 1, file);

				// Write Vertex Information
				for (uint16_t j = 0; j < faceVertexCount; j++)
				{
					CVertex *vtx = &pFace->Vertices[j];
					fwrite(&vtx->x, sizeof(float), 1, file);
					fwrite(&vtx->y, sizeof(float), 1, file);
					fwrite(&vtx->z, sizeof(float), 1, file);
					fwrite(&vtx->Normal, sizeof(CVector3), 1, file);
					fwrite(&vtx->tu, sizeof(float), 1, file);
					fwrite(&vtx->tv, sizeof(float), 1, file);
					fwrite(&vtx->lu, sizeof(float), 1, file);
					fwrite(&vtx->lv, sizeof(float), 1, file);
				}

			}

		}
	}
}

void CLevelFile::SaveDoorMeshes(FILE * file)
{

	uint32_t numMeshes = m_vpDoorList.size();
	uint32_t totalVertexCount = 0;

	fwrite(&numMeshes, sizeof(uint32_t), 1, file);

	for (size_t i = 0; i < numMeshes; i++)
	{
			CMesh *mesh = m_vpDoorList[i]->mesh;

			// write mesh name
			const char *meshName = mesh->Name;
			uint8_t charCount = (uint8_t)strlen(meshName);
			fwrite(&charCount, sizeof(uint8_t), 1, file);
			fwrite(meshName, 1, charCount, file);

			// aabb
			fwrite(&mesh->Bounds, sizeof(CBounds3), 1, file);

			// Write number of surfaces
			uint32_t faceCount = mesh->FaceCount;
			fwrite(&faceCount, sizeof(uint32_t), 1, file);

			// Loop through each surface
			for (uint32_t i = 0; i < faceCount; i++)
			{
				CFace * pFace = mesh->Faces[i];
				uint16_t texId = pFace->TextureIndex;
				fwrite(&pFace->Normal, sizeof(CVector3), 1, file);
				fwrite(&texId, sizeof(uint16_t), 1, file);

				totalVertexCount += pFace->VertexCount;
			}

			fwrite(&totalVertexCount, sizeof(uint32_t), 1, file);

			for (uint32_t i = 0; i < faceCount; i++)
			{
				CFace * pFace = mesh->Faces[i];
				uint16_t faceVertexCount = (uint16_t)pFace->VertexCount;
				fwrite(&faceVertexCount, sizeof(uint16_t), 1, file);

				// Write Vertex Information
				for (uint16_t j = 0; j < faceVertexCount; j++)
				{
					CVertex *vtx = &pFace->Vertices[j];
					fwrite(&vtx->x, sizeof(float), 1, file);
					fwrite(&vtx->y, sizeof(float), 1, file);
					fwrite(&vtx->z, sizeof(float), 1, file);
					fwrite(&vtx->Normal, sizeof(CVector3), 1, file);
					fwrite(&vtx->tu, sizeof(float), 1, file);
					fwrite(&vtx->tv, sizeof(float), 1, file);
					fwrite(&vtx->lu, sizeof(float), 1, file);
					fwrite(&vtx->lv, sizeof(float), 1, file);
				}

			}

	}
}

void CLevelFile::SaveTextures(FILE *file) {

	// save textures names
	Texture			*pTex = m_mapTextures;
	uint16_t numTextures = 0;

	while (pTex != NULL) {
		if (!pTex->isNullTex) {
			numTextures++;
		}
		pTex = pTex->GetNext();
	}

	fwrite(&numTextures, sizeof(uint16_t), 1, file);
	
	for (pTex = m_mapTextures; pTex != 0; pTex = pTex->GetNext()) {
		const char *texName = pTex->name;
		if (pTex->isNullTex) continue;
		uint8_t charCount = (uint8_t)strlen(texName);
		fwrite(&charCount, sizeof(uint8_t), 1, file);
		fwrite(texName, 1, charCount, file);
		uint16_t texID = pTex->uiID;
		fwrite(&texID, sizeof(uint16_t), 1, file);
	}


}

void CLevelFile::SaveTree(FILE * file, CBSPTree * pTree)
{
	// write planes
	uint32_t numPlanes = pTree->GetPlaneCount();
	fwrite(&numPlanes, sizeof(uint32_t), 1, file);

	for (uint32_t i = 0; i != numPlanes; ++i) {
		CPlane3 *plane = pTree->GetPlane(i);
		fwrite(&plane->Normal, sizeof(CVector3), 1, file);
		fwrite(&plane->Distance, sizeof(float), 1, file);
	}
	
	// write nodes
	uint32_t numNodes = pTree->GetNodeCount();
	fwrite(&numNodes, sizeof(uint32_t), 1, file);

	for (uint32_t i = 0; i != numNodes; ++i) {
		CBSPNode *node = pTree->GetNode(i);
		fwrite(&node->Plane, sizeof(uint32_t), 1, file);
		fwrite(&node->Bounds, sizeof(CBounds3), 1, file);
		fwrite(&node->Front, sizeof(uint32_t), 1, file);
		fwrite(&node->Back, sizeof(uint32_t), 1, file);
	}

	// write portals
	uint32_t numPortals = pTree->GetPortalCount();
	fwrite(&numPortals, sizeof(uint32_t), 1, file);

	for (uint32_t i = 0; i != numPortals; ++i) {
		CBSPPortal * pPortal = pTree->GetPortal(i);
		fwrite(&pPortal->OwnerNode, sizeof(uint32_t), 1, file);
		fwrite(&pPortal->LeafOwner[FRONT_OWNER], sizeof(uint32_t), 1, file);
		fwrite(&pPortal->LeafOwner[BACK_OWNER], sizeof(uint32_t), 1, file);
		fwrite(&pPortal->VertexCount, sizeof(uint32_t), 1, file);

		// Write Vertices
		for (uint32_t k = 0; k < pPortal->VertexCount; k++)
		{
			fwrite(&pPortal->Vertices[k], sizeof(CVector3), 1, file);
		}
	}

	// write leaves
	uint32_t numLeaves = pTree->GetLeafCount();
	fwrite(&numLeaves, sizeof(uint32_t), 1, file);

	for (uint32_t i = 0; i != numLeaves; ++i) {
		CBSPLeaf * pLeaf = pTree->GetLeaf(i);
		uint32_t faceCount = pLeaf->FaceIndices.size();
		uint32_t portalCount = pLeaf->PortalIndices.size();

		fwrite(&pLeaf->Bounds, sizeof(CBounds3), 1, file);
		fwrite(&pLeaf->PVSIndex, sizeof(uint32_t), 1, file);
		fwrite(&faceCount, sizeof(uint32_t), 1, file);
		fwrite(&portalCount, sizeof(uint32_t), 1, file);

		// Write linkage details
		for (uint32_t j = 0; j < faceCount; j++) fwrite(&pLeaf->FaceIndices[j], sizeof(uint32_t), 1, file);
		for (uint32_t j = 0; j < portalCount; j++) fwrite(&pLeaf->PortalIndices[j], sizeof(uint32_t), 1, file);
	}

	// write pvs
	fwrite(&pTree->m_lPVSDataSize, sizeof(uint32_t), 1, file);
	fwrite(&pTree->m_bPVSCompressed, sizeof(bool), 1, file);
	fwrite(pTree->m_pPVSData, pTree->m_lPVSDataSize, 1, file);

}

void CLevelFile::SavePlayerInfo(FILE *file)
{
	fwrite(&m_infoPlayerStart.isValid, sizeof(bool), 1, file);
	fwrite(&m_infoPlayerStart.origin, sizeof(float), 3, file);
	fwrite(&m_infoPlayerStart.angles, sizeof(float), 3, file);
}

void CLevelFile::SaveLights(FILE * file)
{
	fwrite(&m_useLightmaps, sizeof(bool), 1, file);

	if (!m_useLightmaps) return;

	uint16_t numLights = (uint16_t)m_lightsVec.size();

	uint16_t numValidLights = 0;
	for (uint16_t i = 0; i != numLights; ++i)
	{
		if (m_lightsVec[i].isValid) {
			++numValidLights;
		}

	}
		
	fwrite(&numValidLights, sizeof(uint16_t), 1, file);

	for (uint16_t i = 0; i != numLights; ++i) 
	{
		if (m_lightsVec[i].isValid) {
			fwrite(&m_lightsVec[i].origin, sizeof(float), 3, file);
			fwrite(&m_lightsVec[i].radius, sizeof(float), 1, file);
			uint16_t numLeaves = (uint16_t)m_lightsVec[i].leaves.size();
			fwrite(&numLeaves, sizeof(uint16_t), 1, file);
			for (uint16_t j = 0; j != numLeaves; ++j)
			{
				uint16_t leafIndex = m_lightsVec[i].leaves[j].leafIdx;
				fwrite(&leafIndex, sizeof(uint16_t), 1, file);
				uint8_t lightClusterIndex = m_lightsVec[i].leaves[j].lightClusterIndex;
				fwrite(&lightClusterIndex, sizeof(uint8_t), 1, file);
			}
		}
	}

}

/* // not used anymore
void CLevelFile::SaveLightMapGenInfo()
{
	FILE *outFile;

	// Save data for lightmap generation
	if ((outFile = fopen("LightMapGenInfo.dat", "wb")) == 0) {
		throw BCERR_FILENOTOPEN;
	}

	SaveLights(outFile);

	// save mesh 0 (bsp tree mesh)

	std::vector<uint32_t> polygonsFirstVertexVec;
	std::vector<float> polygonsNormalVec;

	std::vector<float> vertexPosVec;
	std::vector<float> vertexUvVec;
	std::vector<uint32_t> indexVec;

	CMesh *mesh = m_vpMeshList[0];

	uint32_t numVertexWritten = 0;
	uint32_t faceCount = mesh->FaceCount;
	for (uint32_t i = 0; i < faceCount; i++)
	{
		CFace * pFace = mesh->Faces[i];
		uint16_t faceVertexCount = (uint16_t)pFace->VertexCount;
		
		if (faceVertexCount < 3) continue;
		
		polygonsFirstVertexVec.push_back(numVertexWritten);

		polygonsNormalVec.push_back(pFace->Normal.x);
		polygonsNormalVec.push_back(pFace->Normal.y);
		polygonsNormalVec.push_back(pFace->Normal.z);

		uint32_t FirstVertexIdx = numVertexWritten;
		uint32_t PreviousVertexIdx = 0;
		
		for (uint16_t j = 0; j < faceVertexCount; j++)
		{
			CVertex *vertex = &pFace->Vertices[j];
			float x = vertex->x;
			float y = vertex->y;
			float z = vertex->z;
			float u = vertex->tu;
			float v = vertex->tv;
			
			vertexPosVec.push_back(x);
			vertexPosVec.push_back(y);
			vertexPosVec.push_back(z);

			vertexUvVec.push_back(u);
			vertexUvVec.push_back(v);

			++numVertexWritten;

			// Enough vertices added to start building triangle data?
			if (j >= 2)
			{
				// Add the index data
				indexVec.push_back(FirstVertexIdx);
				indexVec.push_back(PreviousVertexIdx);
				indexVec.push_back(numVertexWritten-1);
			} // End if add triangle data

			  // Update previous vertex
			PreviousVertexIdx = numVertexWritten - 1;
		}
	}
	
	uint32_t numPolygons = polygonsFirstVertexVec.size();
	fwrite(&numPolygons, sizeof(uint32_t), 1, outFile);
	fwrite(&polygonsFirstVertexVec[0], sizeof(uint32_t), numPolygons, outFile);

	uint32_t numFloats = polygonsNormalVec.size();
	// write polygons normals
	fwrite(&numFloats, sizeof(uint32_t), 1, outFile);
	fwrite(&polygonsNormalVec[0], sizeof(float), numFloats, outFile);


	// write vertex positions
	numFloats = vertexPosVec.size();
	fwrite(&numFloats, sizeof(uint32_t), 1, outFile);
	fwrite(&vertexPosVec[0], sizeof(float), numFloats, outFile);
	

	numFloats = vertexUvVec.size();
	fwrite(&numFloats, sizeof(uint32_t), 1, outFile);
	fwrite(&vertexUvVec[0], sizeof(float), numFloats, outFile);

	uint32_t numIndices = indexVec.size();
	fwrite(&numIndices, sizeof(uint32_t), 1, outFile);
	fwrite(&indexVec[0], sizeof(uint32_t), numIndices, outFile);

	fclose(outFile);

}
*/

void CLevelFile::ClearObjects()
{
	m_vpMeshList.clear();
	m_mapEntities = 0;
	m_mapTextures = 0;
}

void CLevelFile::LoadPlayerInfo()
{
	for (Entity *pEnt = m_mapEntities; pEnt; pEnt = pEnt->GetNext()) {
		if (strcmp(PLAYER, pEnt->GetEntityClassName()) == 0)
		{
			m_infoPlayerStart.isValid = true;
			for (Property *pProp = pEnt->GetProperties(); pProp; pProp = pProp->GetNext()) {
				if (strcmp("origin", pProp->GetName()) == 0)
				{
					const char *originStr = pProp->GetValue();

					float x, y, z;
					sscanf(originStr, "%f %f %f", &x, &z, &y); // swap y z
					
					m_infoPlayerStart.origin[0] = x;
					m_infoPlayerStart.origin[1] = y;
					m_infoPlayerStart.origin[2] = z;
				}
				else if (strcmp("angles", pProp->GetName()) == 0)
				{
					const char *anglesStr = pProp->GetValue();

					float pitch, yaw, roll;
					sscanf(anglesStr, "%f %f %f", &pitch, &yaw, &roll);

					m_infoPlayerStart.angles[0] = pitch;
					m_infoPlayerStart.angles[1] = yaw;
					m_infoPlayerStart.angles[2] = roll;
				}
			}

			return;
		}
	}
}

void CLevelFile::LoadLights()
{
	for (Entity *pEnt = m_mapEntities; pEnt; pEnt = pEnt->GetNext()) {
		if (strcmp(LIGHT, pEnt->GetEntityClassName()) == 0)
		{
			Light light;

			for (Property *pProp = pEnt->GetProperties(); pProp; pProp = pProp->GetNext()) {
				if (strcmp("origin", pProp->GetName()) == 0)
				{
					const char *originStr = pProp->GetValue();

					float x, y, z;
					sscanf(originStr, "%f %f %f", &x, &z, &y); // swap y z

					light.origin[0] = x;
					light.origin[1] = y;
					light.origin[2] = z;
				} 
				else if (strcmp("radius", pProp->GetName()) == 0) 
				{
					const char *radiusStr = pProp->GetValue();
					float radius;
					sscanf(radiusStr, "%f", &radius);
					light.radius = radius;
				}
			}

			m_lightsVec.push_back(light);
		}
	}
}

void CLevelFile::GenerateMeshes()
{
	// World mesh

	//
	// Find "worldspawn" entity
	//
	Entity			*pEnt = m_mapEntities;
	bool			 bFound = false;

	while ((pEnt != NULL) && (!bFound)) {
		if (strcmp(WORLDSPAWN, pEnt->GetEntityClassName()) == 0) {
			bFound = true;
		}
		else {
			pEnt = pEnt->GetNext();
		}
	}

	if (!bFound)
	{
		//cout << "Unable to find worldspawn entity in map!" << endl;
		throw BCERR_LOADFAILURE;
	}

	unsigned long numPolygons = 0;

	for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
		if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {
			++numPolygons;
		}
	}

	CMesh *mesh = new CMesh;
	if (!mesh) throw BCERR_OUTOFMEMORY;
	m_vpMeshList.push_back(mesh);
	mesh->Name = _strdup("world");
	mesh->AddFaces(numPolygons);
	mesh->Flags = MESH_WORLD;
	unsigned long faceIdx = 0;

	Poly *poly = pEnt->GetPolys();
	for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
		if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {


			GenerateFace(mesh->Faces[faceIdx++], poly, FACE_WORLD);

			/* to generate 1 mesh for each world polygon
			CMesh *mesh = new CMesh;
			if (!mesh) throw BCERR_OUTOFMEMORY;
			m_vpMeshList.push_back(mesh);
			mesh->Name = _strdup("world");
			mesh->AddFaces(1);
			GenerateFace(mesh->Faces[0], poly);
			mesh->Matrix.Identity();
			mesh->CalculateBoundingBox();
			//mesh->Flags = ...
			*/
		}
	}

	mesh->Matrix.Identity();
	mesh->CalculateBoundingBox();

	// Entities meshes

	// func_wall entities
	pEnt = m_mapEntities;

	for (pEnt = m_mapEntities; pEnt != NULL; pEnt = pEnt->GetNext()) {
		if (strcmp(FUNC_WALL, pEnt->GetEntityClassName()) == 0) {
			
			numPolygons = 0;

			for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
				if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {
					++numPolygons;
				}
			}

			CMesh *mesh = new CMesh;
			if (!mesh) throw BCERR_OUTOFMEMORY;
			m_vpMeshList.push_back(mesh);
			mesh->Name = _strdup(FUNC_WALL);
			mesh->AddFaces(numPolygons);
			mesh->Flags = MESH_WORLD;
			unsigned long faceIdx = 0;

			Poly *poly = pEnt->GetPolys();
			for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
				if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {
					GenerateFace(mesh->Faces[faceIdx++], poly, FACE_DETAIL);
				}
			}

			mesh->Matrix.Identity();
			mesh->CalculateBoundingBox();
		
		}
		else if (strcmp(FUNC_DOOR, pEnt->GetEntityClassName()) == 0) {

			uint32_t numPolygons = 0;

			for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
				if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {
					++numPolygons;
				}
			}

			Door *door = new Door;
			CMesh *mesh = new CMesh;
			if (!mesh) throw BCERR_OUTOFMEMORY;		
			mesh->Name = _strdup(FUNC_DOOR);
			mesh->AddFaces(numPolygons);
			mesh->Flags = MESH_DOOR;

			door->mesh = mesh;
			m_vpDoorList.push_back(door);


			unsigned long faceIdx = 0;

			Poly *poly = pEnt->GetPolys();
			for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
				if (!mNoDrawTexPresent || poly->TextureID != m_noDrawTexID) {
					GenerateFace(mesh->Faces[faceIdx++], poly, FACE_DOOR);
				}
			}

			mesh->Matrix.Identity();
			mesh->CalculateBoundingBox();

			// properties
			/*
			for (Property *pProp = pEnt->GetProperties(); pProp; pProp = pProp->GetNext()) {
			if (strcmp("origin", pProp->GetName()) == 0)
			{
			const char *originStr = pProp->GetValue();

			float x, y, z;
			//sscanf(originStr, "%f %f %f", &x, &z, &y); // swap y z

			//m_infoPlayerStart.origin[0] = x;
			//m_infoPlayerStart.origin[1] = y;
			//m_infoPlayerStart.origin[2] = z;
			}
			else if (strcmp("angles", pProp->GetName()) == 0)
			{
			const char *anglesStr = pProp->GetValue();

			float pitch, yaw, roll;
			sscanf(anglesStr, "%f %f %f", &pitch, &yaw, &roll);

			//m_infoPlayerStart.angles[0] = pitch;
			//m_infoPlayerStart.angles[1] = yaw;
			//m_infoPlayerStart.angles[2] = roll;
			}
			}
			*/
		}
	}

}


void CLevelFile::GenerateFace(CFace *face, const Poly *poly, UINT flags)
{
	CVector3 normal((float)poly->plane.n.x,
					(float)poly->plane.n.y,
					(float)poly->plane.n.z);

	face->Normal = normal;
	face->TextureIndex = (short)poly->TextureID;
	face->Flags = flags;

	face->AddVertices(poly->m_iNumberOfVertices);
	for (int i = 0; i != poly->m_iNumberOfVertices; ++i) {
		Vertex *v = &poly->verts[i];
		face->Vertices[i].x = (float)v->p.x;
		face->Vertices[i].y = (float)v->p.y;
		face->Vertices[i].z = (float)v->p.z;
		face->Vertices[i].Normal = normal;
		face->Vertices[i].tu = (float)v->tex[0];
		face->Vertices[i].tv = (float)v->tex[1];
	}
	
}


void CLevelFile::SearchNoDrawTexture()
{
	// search for no-draw texture ID
	Texture *tmp = m_mapTextures;
	mNoDrawTexPresent = false;

	while (tmp != NULL) {
		if (strcmp(tmp->name, LEVEL_NO_DRAW_TEX_NAME) == 0) {
			m_noDrawTexID = tmp->uiID;
			mNoDrawTexPresent = true;
			tmp->isNullTex = true;
			break;
		}
		tmp = tmp->GetNext();
	}
}



/*
void CLevelFile::GenerateMeshes()
{
// world mesh

//
// Find "worldspawn" entity
//
Entity			*pEnt = m_mapEntities;
bool			 bFound = false;

while ((pEnt != NULL) && (!bFound)) {
if (strcmp("worldspawn", pEnt->GetEntityClassName()) == 0) {
bFound = true;
}
else {
pEnt = pEnt->GetNext();
}
}

if (!bFound)
{
//cout << "Unable to find worldspawn entity in map!" << endl;
throw BCERR_LOADFAILURE;
}

// Allocate a new brush object
CMesh *mesh = new CMesh;
if (!mesh) throw BCERR_OUTOFMEMORY;

m_vpMeshList.push_back(mesh);

unsigned long numPolygons = 0;

for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
if (m_noDrawTexID < 0 || poly->TextureID != m_noDrawTexID) {
++numPolygons;
}
}

mesh->Name = _strdup("world");
mesh->AddFaces(numPolygons);

unsigned long faceIdx = 0;
Poly *poly = pEnt->GetPolys();
for (Poly *poly = pEnt->GetPolys(); poly != NULL; poly = poly->m_pNext) {
if (m_noDrawTexID < 0 || poly->TextureID != m_noDrawTexID) {
GenerateFace(mesh->Faces[faceIdx++], poly);
}
}

mesh->Matrix.Identity();
mesh->CalculateBoundingBox();
//mesh->Flags = ...

// entities meshes

}
*/
