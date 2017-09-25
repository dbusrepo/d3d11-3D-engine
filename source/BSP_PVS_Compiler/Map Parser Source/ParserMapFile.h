#pragma once

const unsigned int MAX_TOKEN_LENGTH = 512;

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;

#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>

#ifdef new
#undef new
#endif

#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

#endif

#include "Parsermath.h"
#include "ParserStructures.h"


////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////

class ParserMAPFile
{
private:
	enum Result
	{
		RESULT_SUCCEED = 0, RESULT_FAIL, RESULT_EOF
	};

	char	m_acToken[MAX_TOKEN_LENGTH + 1];

	HANDLE	m_hFile;

	int		m_iWADFiles;
	LPVOID	*m_pWAD;
	DWORD	*m_pWADSize;
	Texture	*m_pTextureList;

	unsigned int		m_iEntities;
	unsigned int		m_iPolygons;
	uint16_t		m_iTextures;


	Result GetToken();
	Result GetString();

	Result ParseEntity(Entity **ppEntity_);
	Result ParseProperty(Property **ppProperty_);
	Result ParseBrush(Brush **ppBrush_);
	Result ParseFace(Face **ppFace_);
	Result ParseVector(Vector3 &v_);
	Result ParsePlane(Plane &p_);

public:
	bool Load(LPCSTR pcFile_, Entity **ppEntities_, Texture **pTexture_);
};
