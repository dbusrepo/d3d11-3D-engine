#include "D3DApp.h"
#include "D3DAppException.h"
#include "Utility.h"
#include "BSPTree.h"
#include "Scene.h"
#include "Polygon.h"
#include "SceneMaterial.h"
#include "MatrixHelper.h"
#include "TextureResources.h"
#include "Collision.h"
#include "Door.h"
#include "Player.h"
#include "Camera.h"

#define TEXTURES_FOLDER_PATH L"Content\\Textures\\"
#define LIGHTMAPS_CLUSTERMAPS_FOLDER_PATH L"G:\\Documenti\\GoogleDrive\\D3DGraphicsProgramming\\D3D11Projects\\D3DEngine\\source\\BSP_PVS_Compiler\\Content\\LightClusterMaps"


Library::BSPEngine::Scene::Scene(D3DApp *pApp)
{
	m_pApp = pApp;
	m_pSpatialTree = nullptr;
	m_pTextureResouces = nullptr;
	m_pCollision = nullptr;
	m_pPlayer = nullptr;
}


Library::BSPEngine::Scene::~Scene()
{
	Release();
}

void Library::BSPEngine::Scene::Initialize()
{
	m_pCollision = new Collision;

	if (!LoadSceneFromFile("G:\\Documenti\\GoogleDrive\\D3DGraphicsProgramming\\D3D11Projects\\D3DEngine\\source\\BSP_PVS_Compiler\\Content\\Bsp\\test.bsp"))
	{
		throw D3DAppException("LoadSceneFromFile() failed");
	}

	SetupPlayer();

	m_pSpatialTree->Initialize(m_pPlayer->GetCamera());
		
	m_pCollision->SetSpatialTree(m_pSpatialTree);

	// Optimize our collision database
	//m_Collision.Optimize();
}


void Library::BSPEngine::Scene::Draw(const Timer & timer)
{
	m_pPlayer->GetCamera()->Update();
	m_pSpatialTree->Render(timer);
}

bool Library::BSPEngine::Scene::Update(float TimeScale)
{
	m_pSpatialTree->UpdateMovingDoors(TimeScale);

	return false;
}

bool Library::BSPEngine::Scene::UpdatePlayer(Player * pPlayer, float TimeScale, bool onlyTest)
{
	// Validate Parameters
	if (!pPlayer) return false;

	// Retrieve values
	VOLUME_INFO Volume = pPlayer->GetVolumeInfo();
	XMFLOAT3 Position = pPlayer->GetPosition();
	XMFLOAT3 Velocity = pPlayer->GetVelocity();
	//XMFLOAT3 AddedMomentum;

	XMFLOAT3 CollExtentsMin, CollExtentsMax, NewPos, NewVelocity;
	XMFLOAT3 Radius;
	//Radius = (Volume.Max - Volume.Min) * 0.5f;
	XMStoreFloat3(&Radius,
		XMVectorMultiply(
			XMVectorSubtract(XMLoadFloat3(&Volume.Max), XMLoadFloat3(&Volume.Min)),
			XMVectorReplicate(0.5f)));

	// Velocity * TimeScale
	XMFLOAT3 velTimeScaled;
	XMStoreFloat3(&velTimeScaled,
		XMVectorMultiply(
			XMLoadFloat3(&Velocity),
			XMVectorReplicate(TimeScale)));

	// Test for collision against the scene
	if (m_pCollision->CollideEllipsoid(Position, Radius, velTimeScaled, NewPos, NewVelocity, CollExtentsMin, CollExtentsMax))
	{
		// If the lowest intersection point was somewhere in the 
		// bottom eigth (+tolerance) of the ellipsoid
		if (CollExtentsMin.y < -(Radius.y - (Radius.y / 2.125f)))
		{
			// Notify to the player that we're in contact with a "floor"
			pPlayer->SetOnFloor();

		} // End if hit "feet"

		//if (CollExtentsMax.y > (Radius.y - (Radius.y / 2.125f))) {
		//	printf("ceilcollision\n");
		//}

		if (onlyTest) return true;

		  // Scale new frame based velocity into per-secondceilcollision
		//NewVelocity /= TimeScale;
		XMStoreFloat3(&NewVelocity,
			XMVectorDivide(
				XMLoadFloat3(&NewVelocity),
				XMVectorReplicate(TimeScale)));

		// Store our new vertical velocity. It's important to ignore
		// the x / z velocity changes to allow us to 'bump' over objects
		Velocity.y = NewVelocity.y;

		// Truncate the impact extents so the interpolation below begins
		// and ends at the above the player's "feet"
		Radius.y -= (Radius.y / 1.3f); //2.125f); //1.7f); 
		CollExtentsMax.y = max(-Radius.y, CollExtentsMax.y);
		CollExtentsMax.y = min(Radius.y, CollExtentsMax.y);

		// Linearly interpolate (based on the height of the maximum intersection
		// point) the diminishing x/z velocity values returned from the collision function
		float fScale = 1 - (fabsf(CollExtentsMax.y) / Radius.y);
		Velocity.x = Velocity.x + (((NewVelocity.x) - Velocity.x) * fScale);
		Velocity.z = Velocity.z + (((NewVelocity.z) - Velocity.z) * fScale);
		 
		// Update our position and velocity to the new one
		pPlayer->SetPosition(NewPos);
		pPlayer->SetVelocity(Velocity);

		// Hit has been registered
		return true;

	} // End if collision

	// No hit registered
	return false;
}

void Library::BSPEngine::Scene::Release()
{
	m_pApp = nullptr;
	DeleteObject(m_pPlayer);
	DeleteObject(m_pSpatialTree);
	DeleteObject(m_pTextureResouces);
	DeleteObject(m_pCollision);
}

bool Library::BSPEngine::Scene::LoadTextures(FILE *file)
{

	m_pTextureResouces = new TextureResources;

	std::wstring path = TEXTURES_FOLDER_PATH;

	uint16_t numTextures;
	fread(&numTextures, sizeof(uint16_t), 1, file);
	//TEXTURES_FOLDER_PATH

	for (uint16_t i = 0; i < numTextures; i++)
	{
		uint8_t charCount;
		fread(&charCount, sizeof(uint8_t), 1, file);

		char texName[300];
		fread(texName, 1, charCount, file);
		texName[charCount] = 0;

		uint16_t texID;
		fread(&texID, 1, sizeof(uint16_t), file);

		// Now build the full path
		std::wstring textureFileName = TextHelper::s2ws(texName);
		//std::wstring textureFilePath = path + textureFileName + TextHelper::s2ws(".jpg");
		//m_pTextureResouces->LoadTexture(m_pApp->Direct3DDevice(), texName, textureFilePath, texID++);

		std::wstring diffuseMapPath = path + textureFileName + TextHelper::s2ws(".dds");
		m_pTextureResouces->addTextureDiffuseMap(diffuseMapPath, texID);

		std::wstring normalMapPath = path + textureFileName + TextHelper::s2ws("Normal.dds");
		m_pTextureResouces->addTextureNormalMap(normalMapPath, texID);
	} // Next Texture

	//m_pTextureResouces->addTexture(L"Content\\Textures\\0.dds", 0);
	//m_pTextureResouces->addTexture(L"Content\\Textures\\1.dds", 1);
	//m_pTextureResouces->addTexture(L"Content\\Textures\\2.dds", 2);
	m_pTextureResouces->createTextureArrayDiffuseMaps(m_pApp->Direct3DDevice(), m_pApp->Direct3DDeviceContext());
	m_pTextureResouces->createTextureArrayNormalMaps(m_pApp->Direct3DDevice(), m_pApp->Direct3DDeviceContext());

	return true;
}

bool Library::BSPEngine::Scene::LoadMeshes(FILE *file)
{
	char meshName[300];
	uint32_t numMeshes = 0;
	uint32_t totalVertexCount = 0;

	fread(&numMeshes, sizeof(uint32_t), 1, file);

	for (uint32_t meshIdx = 0; meshIdx != numMeshes; ++meshIdx) {


		uint8_t charCount;

		fread(&charCount, sizeof(uint8_t), 1, file);
		fread(meshName, 1, charCount, file);
		meshName[charCount] = 0;

		float bbounds[6];

		// aabb
		fread(bbounds, sizeof(float), 6, file);

		uint32_t faceCount;
		fread(&faceCount, sizeof(uint32_t), 1, file);

		Polygon **polygons = new Polygon*[faceCount];

		for (uint32_t i = 0; i < faceCount; i++)
		{
			XMFLOAT3 faceNormal;
			fread(&faceNormal, sizeof(float), 3, file);
			uint16_t texId;
			fread(&texId, sizeof(uint16_t), 1, file);
			
			polygons[i] = new Polygon;
			if (!polygons[i]) return false;

			polygons[i]->m_texID = texId;
			polygons[i]->m_normal = faceNormal;
			
		}

		fread(&totalVertexCount, sizeof(uint32_t), 1, file);

		for (uint32_t i = 0; i < faceCount; i++)
		{
			uint16_t faceVertexCount;
			fread(&faceVertexCount, sizeof(uint16_t), 1, file);

			// Allocate enough vertices
			if (polygons[i]->AddVertex(faceVertexCount) < 0) return false;
			Vertex *pVertices = polygons[i]->m_pVertex;

			// Load Vertex Information
			for (uint16_t i = 0; i < faceVertexCount; i++)
			{
				float x, y, z;
				XMFLOAT3 vertexNormal;
				float tu, tv;
				float lu, lv;
				fread(&x, sizeof(float), 1, file);
				fread(&y, sizeof(float), 1, file);
				fread(&z, sizeof(float), 1, file);
				fread(&vertexNormal, sizeof(float), 3, file);
				fread(&tu, sizeof(float), 1, file);
				fread(&tv, sizeof(float), 1, file);
				fread(&lu, sizeof(float), 1, file);
				fread(&lv, sizeof(float), 1, file);

				pVertices[i].x = x;
				pVertices[i].y = y;
				pVertices[i].z = z;
				pVertices[i].Normal = vertexNormal;
				pVertices[i].tu = tu;
				pVertices[i].tv = tv;
				pVertices[i].lu = lu;
				pVertices[i].lv = lv;
			}

			polygons[i]->ComputeTextureSpaceVectors();

			if (meshIdx == 0) { // first mesh is always the BSP tree mesh
								// Add this new polygon to the spatial tree
				m_pSpatialTree->AddPolygon(polygons[i]);
			}
		}


		delete[] polygons;
	}

	return true;
}

bool Library::BSPEngine::Scene::LoadDoors(FILE *file)
{
	char meshName[300];
	uint32_t numDoors = 0;
	uint32_t totalVertexCount = 0;

	fread(&numDoors, sizeof(uint32_t), 1, file);

	for (uint32_t doorIdx = 0; doorIdx != numDoors; ++doorIdx) {

		uint8_t charCount;

		
		fread(&charCount, sizeof(uint8_t), 1, file);
		fread(meshName, 1, charCount, file);
		meshName[charCount] = 0;

		float bbounds[6];

		// aabb
		fread(bbounds, sizeof(float), 6, file);

		BSPEngine::TMoveDoor *pDoor = new BSPEngine::TMoveDoor;
		
		XMFLOAT3 &bmin = XMFLOAT3(bbounds[0], bbounds[1], bbounds[2]);
		XMFLOAT3 &bmax = XMFLOAT3(bbounds[3], bbounds[4], bbounds[5]);
		float doorSpeed = 30.f;
		float doorDisplace = 85.f;
		pDoor->Init(doorIdx, bmin, bmax, XMFLOAT3(0, doorDisplace, 0), doorSpeed);

		uint32_t faceCount;
		fread(&faceCount, sizeof(uint32_t), 1, file);

		Polygon **polygons = new Polygon*[faceCount];

		for (uint32_t i = 0; i < faceCount; i++)
		{
			XMFLOAT3 faceNormal;
			fread(&faceNormal, sizeof(float), 3, file);
			uint16_t texId;
			fread(&texId, sizeof(uint16_t), 1, file);

			polygons[i] = new Polygon;
			if (!polygons[i]) return false;

			polygons[i]->m_texID = texId;
			polygons[i]->m_normal = faceNormal;

		}

		fread(&totalVertexCount, sizeof(uint32_t), 1, file);

		for (uint32_t i = 0; i < faceCount; i++)
		{
			uint16_t faceVertexCount;
			fread(&faceVertexCount, sizeof(uint16_t), 1, file);

			// Allocate enough vertices
			if (polygons[i]->AddVertex(faceVertexCount) < 0) return false;
			Vertex *pVertices = polygons[i]->m_pVertex;

			// Load Vertex Information
			for (uint16_t i = 0; i < faceVertexCount; i++)
			{
				float x, y, z;
				XMFLOAT3 vertexNormal;
				float tu, tv;
				float lu, lv;
				fread(&x, sizeof(float), 1, file);
				fread(&y, sizeof(float), 1, file);
				fread(&z, sizeof(float), 1, file);
				fread(&vertexNormal, sizeof(float), 3, file);
				fread(&tu, sizeof(float), 1, file);
				fread(&tv, sizeof(float), 1, file);
				fread(&lu, sizeof(float), 1, file);
				fread(&lv, sizeof(float), 1, file);

				pVertices[i].x = x;
				pVertices[i].y = y;
				pVertices[i].z = z;
				pVertices[i].Normal = vertexNormal;
				pVertices[i].tu = tu;
				pVertices[i].tv = tv;
				pVertices[i].lu = lu;
				pVertices[i].lv = lv;
			}

			polygons[i]->ComputeTextureSpaceVectors();

			pDoor->AddPolygon(polygons[i]);
		}

		m_pSpatialTree->AddDoor(pDoor);

		delete[] polygons;
	}

	return true;
}

bool Library::BSPEngine::Scene::LoadPlayerInfo(FILE *file)
{
	fread(&m_infoPlayerStart.isValid, sizeof(bool), 1, file);
	fread(&m_infoPlayerStart.origin, sizeof(float), 3, file);
	fread(&m_infoPlayerStart.angles, sizeof(float), 3, file);

	return m_infoPlayerStart.isValid;
}

bool Library::BSPEngine::Scene::LoadLights(FILE *file)
{
	wchar_t lightMapFilePath[1000];
	bool useLightmaps;

	fread(&useLightmaps, sizeof(bool), 1, file);

	m_pSpatialTree->m_useLighting = useLightmaps;

	if (!useLightmaps) return true;

	uint16_t numValidLights;
	fread(&numValidLights, sizeof(uint16_t), 1, file);
	
	for (uint16_t i = 0; i != numValidLights; ++i) 
	{
		Light *light = new Light;
		light->index = i;
		fread(&light->origin, sizeof(float), 3, file);
		fread(&light->radius, sizeof(float), 1, file);

		// color, intensity, isActive, isAnimated, frequency
		light->color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		light->intensity = 1.0f;
		light->frequency = 2.0f;
		uint16_t numLeaves;
		fread(&numLeaves, sizeof(uint16_t), 1, file);
		//light->leaves.reserve(numLeaves);
		for (uint16_t j = 0; j != numLeaves; ++j)
		{
			uint16_t leafIndex;
			fread(&leafIndex, sizeof(uint16_t), 1, file);
			uint8_t lightClusterIndex;
			fread(&lightClusterIndex, sizeof(uint8_t), 1, file);
			//light->leaves.push_back({ leafIndex, lightClusterIndex });
			light->m_lightInLeafClusterIndex[leafIndex] = lightClusterIndex;
		}
		m_pSpatialTree->AddLight(light);
		wsprintf(lightMapFilePath, L"%s\\LightMap%d.dds", LIGHTMAPS_CLUSTERMAPS_FOLDER_PATH, i);
		m_pTextureResouces->addLightMap(lightMapFilePath);
	}

	m_pTextureResouces->createTextureArrayLightMaps(m_pApp->Direct3DDevice(), m_pApp->Direct3DDeviceContext());
	return true;
}

bool Library::BSPEngine::Scene::LoadClusterMaps()
{
	wchar_t buffer[1000];

	if (!m_pSpatialTree->m_useLighting) return true;

	size_t numLeaves = m_pSpatialTree->GetLeafList().size();
	for (size_t iLeaf = 0; iLeaf != numLeaves; ++iLeaf)
	{
		wsprintf(buffer, L"%s\\ClustersLeaf%d.dds", LIGHTMAPS_CLUSTERMAPS_FOLDER_PATH, iLeaf);
		m_pTextureResouces->addClusterMap(buffer);
	}

	m_pTextureResouces->createTextureArrayClusterMaps(m_pApp->Direct3DDevice(), m_pApp->Direct3DDeviceContext());

	m_pSpatialTree->SetClusterMapSize(m_pTextureResouces->getClusterMapSize());

	return true;
}


bool Library::BSPEngine::Scene::LoadSceneFromFile(char * strFileName)
{
	// Validate Parameters
	if (!strFileName) return false;

	SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

	// File loading may throw an exception
	try
	{

		// Allocate our spatial partitioning tree of the required type
		m_pSpatialTree = new BSPTree(m_pApp, strFileName);
		//m_pAlphaTree = new CBSPNodeTree(m_pD3DDevice, m_bHardwareTnL);

		// load data from file
		FILE *file;

		if ((file = fopen(strFileName, "rb")) == 0) {
			printf("Can't open %s input file...\n", strFileName);
			//printf("Exiting...\n");
			//system("PAUSE");
			//exit(EXIT_FAILURE);
			throw 0;
		}

		// Read TEXTURES
		LoadTextures(file);

		// Read world polygons
		LoadMeshes(file);

		// load bsp data
		if (!m_pSpatialTree->Load(file)) return false;

		// load doors
		LoadDoors(file);

		// Load player info
		LoadPlayerInfo(file);

		// Load Lights
		LoadLights(file);

		// Build the spatial tree and notify the collision engine
		if (!m_pSpatialTree->Build(m_pTextureResouces)) return false;

		// Load Cluster Maps
		// if (m_useLightmaps) ...
		LoadClusterMaps();
		
	}

	// Catch any exceptions
	catch (...)
	{
		return false;

	} // End Catch Block

	  // Success!
	return true;
}

void Library::BSPEngine::Scene::SetupPlayer()
{
	m_pPlayer = new Player(this);

	// Setup our player's default details
	float playerHeight = 18.f;
	m_pPlayer->SetGravity(XMFLOAT3(0, -750.0f, 0));
	m_pPlayer->SetCamOffset(XMFLOAT3(0.0f, 18.0f, 2.5f));
	m_pPlayer->InitCamera();
	// Set up the players collision volume info
	VOLUME_INFO Volume;
	
	float radiusY = 32;
	Volume.Min = XMFLOAT3(-10, -radiusY, -10);
	Volume.Max = XMFLOAT3(10, radiusY, 10);
	m_pPlayer->SetVolumeInfo(Volume);
	m_pPlayer->SetCamPHeightRatio(playerHeight / radiusY);

	// Lets give a small initial rotation and set initial position
	XMFLOAT3 origin = { m_infoPlayerStart.origin[0],
		                m_infoPlayerStart.origin[1]-30,
		                m_infoPlayerStart.origin[2] };
	m_pPlayer->SetPosition(origin);

	XMFLOAT3 rotation = { m_infoPlayerStart.angles[0],
		                  -m_infoPlayerStart.angles[1] + 90,
		                  m_infoPlayerStart.angles[2] };
	m_pPlayer->Rotate(rotation.x, rotation.y, rotation.z);
}
