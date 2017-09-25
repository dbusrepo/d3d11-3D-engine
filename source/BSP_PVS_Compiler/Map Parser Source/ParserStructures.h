#pragma once

#include <Windows.h>
#include "ParserMath.h"

const unsigned int MAX_TEXTURE_NAME_LENGTH = 32;

////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////

class Vertex
{
public:
	Vector3	p;
	double	tex[2];
};


class Texture
{
private:
	Texture*		m_pNext;
	int				m_iWidth;
	int				m_iHeight;

public:
	enum eGT { GT_FOUND = 0, GT_LOADED, GT_ERROR };

	uint16_t	uiID;
	bool isNullTex = false;
	char			name[MAX_TEXTURE_NAME_LENGTH + 1];

	Texture();
	~Texture();

	Texture* GetTexture(char *pacTexture_, LPVOID lpView_, DWORD dwFileSize_, eGT &rResult_);
	Texture* GetNext() const { return m_pNext; }

	bool IsLast() const;
	int GetHeight() const { return m_iHeight; }
	int GetWidth() const { return m_iWidth; }

	void SetNext(Texture* pTexture_);
};



class Poly
{
public:
	enum eCP
	{
		FRONT = 0, SPLIT, BACK, ONPLANE
	};

	Poly *GetNext() const { return m_pNext; }
	Poly *CopyList() const;
	Poly *CopyPoly() const;
	Poly *ClipToList(Poly *pPoly_, bool bClipOnPlane_);

	int GetNumberOfVertices() const { return m_iNumberOfVertices; }

	void AddVertex(Vertex &Vertex_);
	void AddPoly(Poly *pPoly_);
	void SetNext(Poly *pPoly_);
	void WritePoly(ofstream &ofsFile_) const;

	bool CalculatePlane();
	void SortVerticesCW();
	void ToLeftHanded();
	void CalculateTextureCoordinates(int texWidth, int texHeight, Plane texAxis[2], double texScale[2]);
	void Poly::SplitPoly(Poly *pPoly_, Poly **ppFront_, Poly **ppBack_);
	eCP ClassifyPoly(Poly *pPoly_);

	bool IsLast() const;

	const bool operator == (const Poly &arg_) const;

	Poly();
	~Poly();

	Vertex			*verts;
	Plane			plane;
	uint16_t	TextureID;
	Poly	       *m_pNext;
	unsigned long	m_iNumberOfVertices;
};


class Property
{
private:
	char		*m_pacName;		// Property's name (zero terminated string)
	char		*m_pacValue;	// Property's value (zero terminated string)
	Property	*m_pNext;		// Next property in linked list

public:
	const char *GetName() const { return m_pacName; }
	const char *GetValue() const { return m_pacValue; }
	Property *GetNext() const { return m_pNext; }

	void SetName(const char *pacName_);
	void SetValue(const char *pacValue_);
	void SetNext(Property *pProperty_);
	void WriteProperty(ofstream &ofsFile_) const;

	bool IsLast() const;

	Property();
	~Property();
};


class Entity
{
public:
	Entity		*m_pNext;
	Property	*m_pProperties; // first property is classname
	Poly		*m_pPolys;


	Entity *GetNext() const { return m_pNext; }
	Property *GetProperties() { return m_pProperties; }
	Poly *GetPolys() const { return m_pPolys; }

	unsigned int GetNumberOfProperties() const;
	unsigned int GetNumberOfPolys() const;
	const char *GetEntityClassName() { return m_pProperties->GetValue(); }

	void AddEntity(Entity *pEntity_);
	void AddProperty(Property *pProperty_);
	void AddPoly(Poly *pPoly_);
	void WriteEntity(ofstream& ofsFile_) const;

	bool IsLast() const;

	Entity();
	~Entity();
};


class Face
{
private:
	Face	*m_pNext;

public:
	Plane	plane;
	Plane	texAxis[2];
	double	texScale[2];
	Texture	*pTexture;

	Face *GetNext() const { return m_pNext; }

	void AddFace(Face *pFace_);
	void SetNext(Face *pFace_);

	Poly *GetPolys();

	bool IsLast() const;

	Face();
	~Face();
};


class Brush
{
private:
	Vector3	min, max;	// AABB around brush used to quickly reject far off brushes in CSG
	Brush	*m_pNext;
	Poly	*m_pPolys;

public:
	Brush *GetNext() const { return m_pNext; }
	Poly *GetPolys() { return m_pPolys; }
	Brush *CopyList() const;

	void SetNext(Brush *pBrush_);
	void AddPoly(Poly *pPoly_);

	Poly *MergeList();
	void ClipToBrush(Brush *pBrush_, bool bClipOnPlane_);
	void CalculateAABB();

	unsigned int GetNumberOfPolys() const;
	unsigned int GetNumberOfBrushes() const;

	bool IsLast() const;
	bool AABBIntersect(Brush *pBrush_);

	Brush();
	~Brush();
};