//-----------------------------------------------------------------------------
// File: CMapParser.h
//
// Desc: This file houses the primary .map parser interface
//
//-----------------------------------------------------------------------------

#ifndef _CLEVELFILE_H_
#define _CLEVELFILE_H_

//-----------------------------------------------------------------------------
// Specific Includes
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <vector>
#include "../Map Parser Source/ParserMapFile.h"

//-----------------------------------------------------------------------------
// Reserved Flags
//-----------------------------------------------------------------------------
#define MESH_WORLD                          0x0
#define MESH_DETAIL                         0x1
#define MESH_DOOR						    0x2

#define FACE_WORLD                          0x0
#define FACE_DETAIL                         0x1
#define FACE_DOOR						    0x2

//-----------------------------------------------------------------------------
// Entities Names
//-----------------------------------------------------------------------------
#define WORLDSPAWN "worldspawn"
#define FUNC_WALL "func_wall"
#define FUNC_DOOR "func_door"
#define PLAYER "info_player_start"
#define LIGHT "light"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CMesh;
class CFace;
class CBSPTree;

//-----------------------------------------------------------------------------
// Typedefs Structures & Enumerators
//-----------------------------------------------------------------------------
#define LEVEL_NO_DRAW_TEX_NAME "NULL"

//class CEntity {
//};

struct InfoPlayerStart {
	bool isValid = false;
	float origin[3];
	float angles[3];
};

struct Light {
	bool isValid = false;
	float origin[3];
	float radius = 500.f;
	struct LightInLeafInfo {
		uint16_t leafIdx;
		uint8_t lightClusterIndex;
	};
	std::vector<LightInLeafInfo> leaves; // leaves reached by the light
};

struct Door {
	CMesh *mesh = nullptr;
};

typedef std::vector<CMesh*>         vectorMesh;
typedef std::vector<Door*>			vectorDoor;
//typedef std::vector<TextureInfo>    vectorTexture;
//typedef std::vector<iwfEntity*>     vectorEntity;
//typedef std::vector<iwfShader*>     vectorShader;
//typedef std::vector<iwfMaterial*>   vectorMaterial;
typedef std::vector<Light> vectorLight;


//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CLevelFile (Class)
// Desc : This is the CLevelFile class,
//		  which handles all our specific .MAP reading & BSP/PVS writing requirements.
//-----------------------------------------------------------------------------



class CLevelFile
{
public:
	//-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	CLevelFile();
	~CLevelFile();

	//-------------------------------------------------------------------------
	// Public Functions for This Class.
	//-------------------------------------------------------------------------
	void            Load(LPCTSTR FileName);
	void            Save(LPCTSTR FileName, CBSPTree * pTree);
	void            ClearObjects();

	//-------------------------------------------------------------------------
	// Public Variables for This Class.
	//-------------------------------------------------------------------------
	vectorMesh      m_vpMeshList;       // A list of all meshes loaded for bsp.
	vectorDoor		m_vpDoorList;
	//vectorTexture   m_vpTextureList;    // A list of all textures loaded.
	//vectorEntity    m_vpEntityList;     // A list of all entities loaded.
	//vectorMaterial  m_vpMaterialList;   // A list of all materials loaded.
	//vectorShader    m_vpShaderList;     // A list of all shaders loaded
	vectorLight m_lightsVec;
	bool m_useLightmaps = false;

private:

	ParserMAPFile m_mapParser;
	Entity	*m_mapEntities;
	Texture *m_mapTextures;

	bool mNoDrawTexPresent;
	uint16_t m_noDrawTexID;

	InfoPlayerStart m_infoPlayerStart;
	
	void SearchNoDrawTexture();
	void GenerateMeshes();
	void GenerateFace(CFace *face, const Poly *, UINT flags);
	void LoadPlayerInfo();
	void LoadLights();

	void SaveMeshes(FILE *file);
	void SaveDoorMeshes(FILE *file);
	void SaveTextures(FILE *file);
	void SaveTree(FILE *file, CBSPTree * pTree);
	void SavePlayerInfo(FILE *file);
	void SaveLights(FILE *file);
	//void SaveLightMapGenInfo();

	//-------------------------------------------------------------------------
	// Private Functions for This Class.
	//-------------------------------------------------------------------------
	
	//void            ReadName(UCHAR LengthSize, char ** Name);
	//void            ReadScriptRef(iwfScriptRef * pScript);
	//void            WriteScriptRef(iwfScriptRef *pScript);
	//void            SeekName(UCHAR LengthSize);
	//void            SeekScriptRef();
	//void            WriteName(UCHAR LengthSize, char * Name);

	//void            WriteTextures();
	//void            WriteMaterials();
	//void            WriteShaders();
	//void            WriteMesh(CMesh * pMesh);
	//void            WriteEntity(iwfEntity * pEntity);
	//void            WriteTree(CBSPTree * pTree);

	//-------------------------------------------------------------------------
	// Private Static CHUNKPROC Callbacks Registered for This Class.
	//-------------------------------------------------------------------------
	
	//static void     ReadGroup(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadMesh(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadSurfaces(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadVertices(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadIndices(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadEntity(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadTextures(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadMaterials(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);
	//static void     ReadShaders(LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData);

};

#endif
