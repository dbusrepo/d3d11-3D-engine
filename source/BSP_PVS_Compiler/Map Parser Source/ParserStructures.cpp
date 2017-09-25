#include "..\..\Library\BSPTree.h"
#include <iostream>
#include <fstream>

#include "WAD3.h"
#include "ParserStructures.h"

using namespace std;


////////////////////////////////////////////////////////////////////
// Texture member functions
////////////////////////////////////////////////////////////////////

Texture* Texture::GetTexture(char *pacTexture_, LPVOID lpView_, DWORD dwFileSize_, Texture::eGT &rResult_)
{
	rResult_ = eGT::GT_ERROR;

	//
	// Check if texture already exists
	//
	if (_stricmp(name, pacTexture_) == 0)
	{
		rResult_ = eGT::GT_FOUND;

		return this;
	}

	if (!IsLast())
	{
		return m_pNext->GetTexture(pacTexture_, lpView_, dwFileSize_, rResult_);
	}

	//
	// Load texture information
	//
	Texture			*pTexture = new Texture;
	LPWAD3_HEADER	lpHeader = NULL;
	LPWAD3_LUMP		lpLump = NULL;
	LPWAD3_MIP		lpMip = NULL;

	DWORD			dwNumLumps = 0;
	DWORD			dwTableOffset = 0;
	DWORD			dwFilePos = 0;
	DWORD			dwPaletteOffset = 0;
	WORD			wPaletteSize = 0;
	DWORD			dwWidth = 0;
	DWORD			dwHeight = 0;

	// Make sure it's at least big enough to manipulate the header
	if (dwFileSize_ < sizeof(WAD3_HEADER))
	{
		CorruptWAD3("WAD3 file is malformed.", lpView_);

		delete pTexture;

		return NULL;
	}

	lpHeader = (LPWAD3_HEADER)lpView_;

	if (lpHeader->identification != WAD3_ID)
	{
		CorruptWAD3("Invalid WAD3 header id.", lpView_);

		delete pTexture;

		return NULL;
	}

	dwNumLumps = lpHeader->numlumps;
	dwTableOffset = lpHeader->infotableofs;

	// Make sure our table is really there
	if (((dwNumLumps * sizeof(WAD3_LUMP)) + dwTableOffset) > dwFileSize_)
	{
		CorruptWAD3("WAD3 file is malformed.", lpView_);

		delete pTexture;

		return NULL;
	}

	// Point at the first table entry
	lpLump = (LPWAD3_LUMP)((LPBYTE)lpView_ + dwTableOffset);

	bool	bFound = false;
	DWORD	j = 0;

	while ((!bFound) && (j < dwNumLumps))
	{
		if (lpLump->type == WAD3_TYPE_MIP)
		{
			if (_stricmp(lpLump->name, pacTexture_) == 0)
			{
				// Find out where the MIP actually is
				dwFilePos = lpLump->filepos;

				// Make sure it's in bounds
				if (dwFilePos >= dwFileSize_)
				{
					CorruptWAD3("Invalid lump entry; filepos is malformed.", lpView_);

					delete pTexture;

					return NULL;
				}

				// Point at the mip
				lpMip = (LPWAD3_MIP)((LPBYTE)lpView_ + dwFilePos);

				strcpy(pTexture->name, pacTexture_);

				pTexture->m_iWidth = lpMip->width;
				pTexture->m_iHeight = lpMip->height;
				bFound = true;
			}
		}

		j++;
		lpLump++;
	}

	if (!bFound)
	{
		delete pTexture;

		return NULL;
	}

	m_pNext = pTexture;

	rResult_ = eGT::GT_LOADED;

	return pTexture;
}


Texture::Texture()
{
	memset(name, 0, MAX_TEXTURE_NAME_LENGTH + 1);

	m_pNext = NULL;
	m_iHeight = 0;
	m_iWidth = 0;
}


Texture::~Texture()
{
	if (!IsLast())
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


void Texture::SetNext(Texture *pTexture_)
{
	if (IsLast())
	{
		m_pNext = pTexture_;

		return;
	}

	//
	// Insert the given list
	//
	if (pTexture_ != NULL)
	{
		Texture *pTexture = pTexture_;

		while (!pTexture->IsLast())
		{
			pTexture = pTexture->GetNext();
		}

		pTexture->SetNext(m_pNext);
	}

	m_pNext = pTexture_;
}


bool Texture::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////
// Property member functions
////////////////////////////////////////////////////////////////////

void Property::WriteProperty(ofstream &ofsFile_) const
{
	/*	Property:
	x char		Property name (zero terminated)
	x char		Property value (zero terminated) */

	ofsFile_ << GetName() << (char)0x00;
	ofsFile_ << GetValue() << (char)0x00;

	if (!IsLast())
	{
		GetNext()->WriteProperty(ofsFile_);
	}
}


Property::~Property()
{
	if (m_pacName != NULL)
	{
		delete[] m_pacName;
		m_pacName = NULL;
	}

	if (m_pacValue != NULL)
	{
		delete[] m_pacValue;
		m_pacValue = NULL;
	}

	if (m_pNext != NULL)
	{
		delete m_pNext;	// recursively delete whole list
		m_pNext = NULL;
	}
}


Property::Property()
{
	m_pacName = NULL;
	m_pacValue = NULL;
	m_pNext = NULL;
}


bool Property::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}


void Property::SetNext(Property *pProperty_)
{
	if (IsLast())
	{
		m_pNext = pProperty_;

		return;
	}

	//
	// Insert the given list
	//
	Property *pProperty = pProperty_;

	while (!pProperty->IsLast())
	{
		pProperty = pProperty->GetNext();
	}

	pProperty->SetNext(m_pNext);

	m_pNext = pProperty_;
}


void Property::SetName(const char *pacName_)
{
	if (m_pacName != NULL)
	{
		delete[] m_pacName;
		m_pacName = NULL;
	}

	m_pacName = new char[strlen(pacName_) + 1];

	strcpy(m_pacName, pacName_);
}


void Property::SetValue(const char *pacValue_)
{
	if (m_pacValue != NULL)
	{
		delete[] m_pacValue;
		m_pacValue = NULL;
	}

	m_pacValue = new char[strlen(pacValue_) + 1];

	strcpy(m_pacValue, pacValue_);
}


////////////////////////////////////////////////////////////////////
// Poly member functions
////////////////////////////////////////////////////////////////////

void Poly::WritePoly(ofstream &ofsFile_) const
{
	/*
	Polygon:
	1 uint		Texture ID
	1 Plane		Polygon plane
	1 uint		Number of vertices
	x Vertex	Vertices
	*/

	ofsFile_.write((char *)&TextureID, sizeof(unsigned int));
	ofsFile_.write((char *)&plane.n.x, sizeof(double));
	ofsFile_.write((char *)&plane.n.y, sizeof(double));
	ofsFile_.write((char *)&plane.n.z, sizeof(double));
	ofsFile_.write((char *)&plane.d, sizeof(double));

	unsigned int ui = (unsigned int)GetNumberOfVertices();

	ofsFile_.write((char *)&ui, sizeof(ui));

	for (int i = 0; i < GetNumberOfVertices(); i++)
	{
		ofsFile_.write((char *)&verts[i].p.x, sizeof(double));
		ofsFile_.write((char *)&verts[i].p.y, sizeof(double));
		ofsFile_.write((char *)&verts[i].p.z, sizeof(double));
		ofsFile_.write((char *)&verts[i].tex[0], sizeof(double));
		ofsFile_.write((char *)&verts[i].tex[1], sizeof(double));
	}

	if (!IsLast())
	{
		GetNext()->WritePoly(ofsFile_);
	}
}


const bool Poly::operator==(const Poly &arg_) const
{
	if (m_iNumberOfVertices == arg_.m_iNumberOfVertices)
	{
		if (plane.d == arg_.plane.d)
		{
			if (plane.n == arg_.plane.n)
			{
				for (int i = 0; i < GetNumberOfVertices(); i++)
				{
					if (verts[i].p == arg_.verts[i].p)
					{
						if (verts[i].tex[0] != arg_.verts[i].tex[0])
						{
							return false;
						}

						if (verts[i].tex[1] != arg_.verts[i].tex[1])
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}

				if (TextureID == arg_.TextureID)
				{
					return true;
				}
			}
		}
	}

	return false;
}


Poly *Poly::ClipToList(Poly *pPoly_, bool bClipOnPlane_)
{
	switch (ClassifyPoly(pPoly_))
	{
	case eCP::FRONT:
	{
		return pPoly_->CopyPoly();
	} break;

	case eCP::BACK:
	{
		if (IsLast())
		{
			return NULL;
		}

		return m_pNext->ClipToList(pPoly_, bClipOnPlane_);
	} break;

	case eCP::ONPLANE:
	{
		double	Angle = plane.n.Dot(pPoly_->plane.n) - 1;

		if ((Angle < epsilon) && (Angle > -epsilon))
		{
			if (!bClipOnPlane_)
			{
				return pPoly_->CopyPoly();
			}
		}

		if (IsLast())
		{
			return NULL;
		}

		return m_pNext->ClipToList(pPoly_, bClipOnPlane_);
	} break;

	case eCP::SPLIT:
	{
		Poly *pFront = NULL;
		Poly *pBack = NULL;

		SplitPoly(pPoly_, &pFront, &pBack);

		if (IsLast())
		{
			delete pBack;

			return pFront;
		}

		Poly *pBackFrags = m_pNext->ClipToList(pBack, bClipOnPlane_);

		if (pBackFrags == NULL)
		{
			delete pBack;

			return pFront;
		}

		if (*pBackFrags == *pBack)
		{
			delete pFront;
			delete pBack;
			delete pBackFrags;

			return pPoly_->CopyPoly();
		}

		delete pBack;

		pFront->AddPoly(pBackFrags);

		return pFront;
	} break;
	}

	return NULL;
}


Poly *Poly::CopyPoly() const
{
	Poly *pPoly = new Poly;

	pPoly->TextureID = TextureID;

	pPoly->m_iNumberOfVertices = m_iNumberOfVertices;
	pPoly->plane = plane;

	pPoly->verts = new Vertex[m_iNumberOfVertices];
	memcpy(pPoly->verts, verts, sizeof(Vertex) * m_iNumberOfVertices);

	return pPoly;
}


Poly *Poly::CopyList() const
{
	Poly *pPoly = new Poly;

	pPoly->TextureID = TextureID;

	pPoly->m_iNumberOfVertices = m_iNumberOfVertices;
	pPoly->plane = plane;

	pPoly->verts = new Vertex[m_iNumberOfVertices];
	memcpy(pPoly->verts, verts, sizeof(Vertex) * m_iNumberOfVertices);

	if (!IsLast())
	{
		pPoly->AddPoly(m_pNext->CopyList());
	}

	return pPoly;
}


Poly::eCP Poly::ClassifyPoly(Poly *pPoly_)
{
	bool	bFront = false, bBack = false;
	double	dist;

	for (int i = 0; i < (int)pPoly_->GetNumberOfVertices(); i++)
	{
		dist = plane.n.Dot(pPoly_->verts[i].p) + plane.d;

		if (dist > 0.001)
		{
			if (bBack)
			{
				return eCP::SPLIT;
			}

			bFront = true;
		}
		else if (dist < -0.001)
		{
			if (bFront)
			{
				return eCP::SPLIT;
			}

			bBack = true;
		}
	}

	if (bFront)
	{
		return eCP::FRONT;
	}
	else if (bBack)
	{
		return eCP::BACK;
	}

	return eCP::ONPLANE;
}


void Poly::SplitPoly(Poly *pPoly_, Poly **ppFront_, Poly **ppBack_)
{
	Plane::eCP	*pCP = new Plane::eCP[pPoly_->GetNumberOfVertices()];
	int i;

	//
	// Classify all points
	//
	for (i = 0; i < pPoly_->GetNumberOfVertices(); i++)
	{
		pCP[i] = plane.ClassifyPoint(pPoly_->verts[i].p);
	}

	//
	// Build fragments
	//
	Poly		*pFront = new Poly;
	Poly		*pBack = new Poly;

	pFront->TextureID = pPoly_->TextureID;
	pBack->TextureID = pPoly_->TextureID;
	pFront->plane = pPoly_->plane;
	pBack->plane = pPoly_->plane;

	for (i = 0; i < pPoly_->GetNumberOfVertices(); i++)
	{
		//
		// Add point to appropriate list
		//
		switch (pCP[i])
		{
		case Plane::eCP::FRONT:
		{
			pFront->AddVertex(pPoly_->verts[i]);
		} break;

		case Plane::eCP::BACK:
		{
			pBack->AddVertex(pPoly_->verts[i]);
		} break;

		case Plane::eCP::ONPLANE:
		{
			pFront->AddVertex(pPoly_->verts[i]);
			pBack->AddVertex(pPoly_->verts[i]);
		} break;
		}

		//
		// Check if edges should be split
		//
		int		iNext = i + 1;
		bool	bIgnore = false;

		if (i == (pPoly_->GetNumberOfVertices() - 1))
		{
			iNext = 0;
		}

		if ((pCP[i] == Plane::eCP::ONPLANE) && (pCP[iNext] != Plane::eCP::ONPLANE))
		{
			bIgnore = true;
		}
		else if ((pCP[iNext] == Plane::eCP::ONPLANE) && (pCP[i] != Plane::eCP::ONPLANE))
		{
			bIgnore = true;
		}

		if ((!bIgnore) && (pCP[i] != pCP[iNext]))
		{
			Vertex	v;	// New vertex created by splitting
			double	p;	// Percentage between the two points

			plane.GetIntersection(pPoly_->verts[i].p, pPoly_->verts[iNext].p, v.p, p);

			v.tex[0] = pPoly_->verts[iNext].tex[0] - pPoly_->verts[i].tex[0];
			v.tex[1] = pPoly_->verts[iNext].tex[1] - pPoly_->verts[i].tex[1];

			v.tex[0] = pPoly_->verts[i].tex[0] + (p * v.tex[0]);
			v.tex[1] = pPoly_->verts[i].tex[1] + (p * v.tex[1]);

			pFront->AddVertex(v);
			pBack->AddVertex(v);
		}
	}

	delete[] pCP;

	pFront->CalculatePlane();
	pBack->CalculatePlane();

	*ppFront_ = pFront;
	*ppBack_ = pBack;
}


void Poly::CalculateTextureCoordinates(int texWidth, int texHeight, Plane texAxis[2], double texScale[2])
{
	int i;
	//
	// Calculate texture coordinates
	//
	for (i = 0; i < GetNumberOfVertices(); i++)
	{
		double U, V;

		U = texAxis[0].n.Dot(verts[i].p);
		U = U / ((double)texWidth) / texScale[0];
		U = U + (texAxis[0].d / (double)texWidth);

		V = texAxis[1].n.Dot(verts[i].p);
		V = V / ((double)texHeight) / texScale[1];
		V = V + (texAxis[1].d / (double)texHeight);

		verts[i].tex[0] = U;
		verts[i].tex[1] = V;
	}

	//
	// Check which axis should be normalized
	//
	bool	bDoU = true;
	bool	bDoV = true;

	for (i = 0; i < GetNumberOfVertices(); i++)
	{
		if ((verts[i].tex[0] < 1) && (verts[i].tex[0] > -1))
		{
			bDoU = false;
		}

		if ((verts[i].tex[1] < 1) && (verts[i].tex[1] > -1))
		{
			bDoV = false;
		}
	}

	//
	// Calculate coordinate nearest to 0
	//
	if (bDoU || bDoV)
	{
		double	NearestU = 0;
		double	U = verts[0].tex[0];

		double	NearestV = 0;
		double	V = verts[0].tex[1];

		if (bDoU)
		{
			if (U > 1)
			{
				NearestU = floor(U);
			}
			else
			{
				NearestU = ceil(U);
			}
		}

		if (bDoV)
		{
			if (V > 1)
			{
				NearestV = floor(V);
			}
			else
			{
				NearestV = ceil(V);
			}
		}

		for (i = 0; i < GetNumberOfVertices(); i++)
		{
			if (bDoU)
			{
				U = verts[i].tex[0];

				if (fabs(U) < fabs(NearestU))
				{
					if (U > 1)
					{
						NearestU = floor(U);
					}
					else
					{
						NearestU = ceil(U);
					}
				}
			}

			if (bDoV)
			{
				V = verts[i].tex[1];

				if (fabs(V) < fabs(NearestV))
				{
					if (V > 1)
					{
						NearestV = floor(V);
					}
					else
					{
						NearestV = ceil(V);
					}
				}
			}
		}

		//
		// Normalize texture coordinates
		//
		for (i = 0; i < GetNumberOfVertices(); i++)
		{
			verts[i].tex[0] = verts[i].tex[0] - NearestU;
			verts[i].tex[1] = verts[i].tex[1] - NearestV;
		}
	}
}


void Poly::SortVerticesCW()
{
	//
	// Calculate center of polygon
	//
	Vector3	center;
	int i;

	for (i = 0; i < GetNumberOfVertices(); i++)
	{
		center = center + verts[i].p;
	}

	center = center / GetNumberOfVertices();

	//
	// Sort vertices
	//
	for (i = 0; i < GetNumberOfVertices() - 2; i++)
	{
		Vector3	a;
		Plane	p;
		double	SmallestAngle = -1;
		int		Smallest = -1;

		a = verts[i].p - center;
		a.Normalize();

		p.PointsToPlane(verts[i].p, center, center + plane.n);

		for (int j = i + 1; j < GetNumberOfVertices(); j++)
		{
			if (p.ClassifyPoint(verts[j].p) != Plane::eCP::BACK)
			{
				Vector3	b;
				double	Angle;

				b = verts[j].p - center;
				b.Normalize();

				Angle = a.Dot(b);

				if (Angle > SmallestAngle)
				{
					SmallestAngle = Angle;
					Smallest = j;
				}
			}
		}

		if (Smallest == -1)
		{
			cout << "Error: Degenerate polygon!" << endl;

			abort();
		}

		Vertex	t = verts[Smallest];
		verts[Smallest] = verts[i + 1];
		verts[i + 1] = t;
	}

	//
	// Check if vertex order needs to be reversed for back-facing polygon
	//
	Plane	oldPlane = plane;

	CalculatePlane();

	if (plane.n.Dot(oldPlane.n) < 0)
	{
		int j = GetNumberOfVertices();

		for (int i = 0; i < j / 2; i++)
		{
			Vertex v = verts[i];
			verts[i] = verts[j - i - 1];
			verts[j - i - 1] = v;
		}
	}
}


bool Poly::CalculatePlane()
{
	Vector3	centerOfMass;
	double	magnitude;
	int     i, j;

	if (GetNumberOfVertices() < 3)
	{
		cout << "Polygon has less than 3 vertices!" << endl;

		return false;
	}

	plane.n.x = 0.0f;
	plane.n.y = 0.0f;
	plane.n.z = 0.0f;
	centerOfMass.x = 0.0f;
	centerOfMass.y = 0.0f;
	centerOfMass.z = 0.0f;

	for (i = 0; i < GetNumberOfVertices(); i++)
	{
		j = i + 1;

		if (j >= GetNumberOfVertices())
		{
			j = 0;
		}

		plane.n.x += (verts[i].p.y - verts[j].p.y) * (verts[i].p.z + verts[j].p.z);
		plane.n.y += (verts[i].p.z - verts[j].p.z) * (verts[i].p.x + verts[j].p.x);
		plane.n.z += (verts[i].p.x - verts[j].p.x) * (verts[i].p.y + verts[j].p.y);

		centerOfMass.x += verts[i].p.x;
		centerOfMass.y += verts[i].p.y;
		centerOfMass.z += verts[i].p.z;
	}

	if ((fabs(plane.n.x) < epsilon) && (fabs(plane.n.y) < epsilon) &&
		(fabs(plane.n.z) < epsilon))
	{
		return false;
	}

	magnitude = sqrt(plane.n.x * plane.n.x + plane.n.y * plane.n.y + plane.n.z * plane.n.z);

	if (magnitude < epsilon)
	{
		return false;
	}

	plane.n.x /= magnitude;
	plane.n.y /= magnitude;
	plane.n.z /= magnitude;

	centerOfMass.x /= (double)GetNumberOfVertices();
	centerOfMass.y /= (double)GetNumberOfVertices();
	centerOfMass.z /= (double)GetNumberOfVertices();

	plane.d = -(centerOfMass.Dot(plane.n));

	return true;
}


void Poly::AddPoly(Poly *pPoly_)
{
	if (pPoly_ != NULL)
	{
		if (IsLast())
		{
			m_pNext = pPoly_;

			return;
		}

		Poly *pPoly = m_pNext;

		while (!pPoly->IsLast())
		{
			pPoly = pPoly->GetNext();
		}

		pPoly->m_pNext = pPoly_;
	}
}


void Poly::SetNext(Poly *pPoly_)
{
	if (IsLast())
	{
		m_pNext = pPoly_;

		return;
	}

	//
	// Insert the given list
	//
	Poly *pPoly = pPoly_;

	while (!pPoly->IsLast())
	{
		pPoly = pPoly->GetNext();
	}

	pPoly->SetNext(m_pNext);

	m_pNext = pPoly_;
}


void Poly::AddVertex(Vertex &Vertex_)
{
	Vertex *pVertices = new Vertex[m_iNumberOfVertices + 1];

	memcpy(pVertices, verts, sizeof(Vertex) * m_iNumberOfVertices);

	delete[] verts;

	verts = pVertices;

	verts[m_iNumberOfVertices] = Vertex_;

	m_iNumberOfVertices++;
}


bool Poly::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}


Poly::Poly()
{
	m_pNext = NULL;
	verts = NULL;
	m_iNumberOfVertices = 0;
	TextureID = 0;
}


Poly::~Poly()
{
	if (!IsLast())
	{
		delete m_pNext;
		m_pNext = NULL;
	}

	if (verts != NULL)
	{
		delete[] verts;
		verts = NULL;
		m_iNumberOfVertices = 0;
	}
}



////////////////////////////////////////////////////////////////////
// Entity member functions
////////////////////////////////////////////////////////////////////

void Entity::WriteEntity(ofstream &ofsFile_) const
{
	/*	Entity:
	x char		Entity class (zero terminated)
	1 uint		Number of properties
	x Property	Entities properties
	1 uint		Number of polygons
	x Polygon	Polygons */

	ofsFile_ << m_pProperties->GetValue() << (char)0x00;

	unsigned int ui = GetNumberOfProperties() - 1;

	ofsFile_.write((char *)&ui, sizeof(ui));

	if (!m_pProperties->IsLast())
	{
		m_pProperties->GetNext()->WriteProperty(ofsFile_);
	}

	ui = GetNumberOfPolys();

	ofsFile_.write((char *)&ui, sizeof(ui));

	if (GetNumberOfPolys() > 0)
	{
		m_pPolys->WritePoly(ofsFile_);
	}

	if (!IsLast())
	{
		GetNext()->WriteEntity(ofsFile_);
	}
}


Entity::Entity()
{
	m_pNext = NULL;
	m_pProperties = NULL;
	m_pPolys = NULL;
}


Entity::~Entity()
{
	if (m_pProperties != NULL)
	{
		delete m_pProperties;
		m_pProperties = NULL;
	}

	if (m_pPolys != NULL)
	{
		delete m_pPolys;
		m_pPolys = NULL;
	}

	if (m_pNext != NULL)
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


bool Entity::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}


void Entity::AddEntity(Entity *pEntity_)
{
	if (IsLast())
	{
		m_pNext = pEntity_;

		return;
	}

	Entity *pEntity = m_pNext;

	while (!pEntity->IsLast())
	{
		pEntity = pEntity->GetNext();
	}

	pEntity->m_pNext = pEntity_;
}


void Entity::AddProperty(Property *pProperty_)
{
	if (m_pProperties == NULL)
	{
		m_pProperties = pProperty_;

		return;
	}

	Property *pProperty = m_pProperties;

	while (!pProperty->IsLast())
	{
		pProperty = pProperty->GetNext();
	}

	pProperty->SetNext(pProperty_);
}


void Entity::AddPoly(Poly *pPoly_)
{
	if (m_pPolys == NULL)
	{
		m_pPolys = pPoly_;

		return;
	}

	Poly *pPoly = m_pPolys;

	while (!pPoly->IsLast())
	{
		pPoly = pPoly->GetNext();
	}

	pPoly->SetNext(pPoly_);
}


unsigned int Entity::GetNumberOfProperties() const
{
	Property		*pProperty = m_pProperties;
	unsigned int	uiCount = 0;

	while (pProperty != NULL)
	{
		pProperty = pProperty->GetNext();
		uiCount++;
	}

	return uiCount;
}


unsigned int Entity::GetNumberOfPolys() const
{
	Poly			*pPoly = m_pPolys;
	unsigned int	uiCount = 0;

	while (pPoly != NULL)
	{
		pPoly = pPoly->GetNext();
		uiCount++;
	}

	return uiCount;
}



////////////////////////////////////////////////////////////////////
// Brush member functions
////////////////////////////////////////////////////////////////////

void Brush::ClipToBrush(Brush *pBrush_, bool bClipOnPlane_)
{
	Poly *pPolyList = NULL;
	Poly *pPoly = m_pPolys;

	for (unsigned int i = 0; i < GetNumberOfPolys(); i++)
	{
		Poly *pClippedPoly = pBrush_->GetPolys()->ClipToList(pPoly, bClipOnPlane_);

		if (pPolyList == NULL)
		{
			pPolyList = pClippedPoly;
		}
		else
		{
			pPolyList->AddPoly(pClippedPoly);
		}

		pPoly = pPoly->GetNext();
	}

	delete m_pPolys;
	m_pPolys = pPolyList;
}


Poly *Brush::MergeList()
{
	Brush			*pClippedList = CopyList();
	Brush			*pClip = pClippedList;
	Brush			*pBrush = NULL;
	Poly			*pPolyList = NULL;

	bool			bClipOnPlane = false;
	unsigned int	uiBrushes = GetNumberOfBrushes();

	for (unsigned int i = 0; i < uiBrushes; i++)
	{
		pBrush = this;
		bClipOnPlane = false;

		for (unsigned int j = 0; j < uiBrushes; j++)
		{
			if (i == j)
			{
				bClipOnPlane = true;
			}
			else
			{
				if (pClip->AABBIntersect(pBrush))
				{
					pClip->ClipToBrush(pBrush, bClipOnPlane);
				}
			}

			pBrush = pBrush->GetNext();
		}

		pClip = pClip->GetNext();
	}

	pClip = pClippedList;

	while (pClip != NULL)
	{
		if (pClip->GetNumberOfPolys() != 0)
		{
			//
			// Extract brushes left over polygons and add them to the list
			//
			Poly *pPoly = pClip->GetPolys()->CopyList();

			if (pPolyList == NULL)
			{
				pPolyList = pPoly;
			}
			else
			{
				pPolyList->AddPoly(pPoly);
			}

			pClip = pClip->GetNext();
		}
		else
		{
			//
			// Brush has no polygons and should be deleted
			//
			if (pClip == pClippedList)
			{
				pClip = pClippedList->GetNext();

				pClippedList->SetNext(NULL);

				delete pClippedList;

				pClippedList = pClip;
			}
			else
			{
				Brush	*pTemp = pClippedList;

				while (pTemp != NULL)
				{
					if (pTemp->GetNext() == pClip)
					{
						break;
					}

					pTemp = pTemp->GetNext();
				}

				pTemp->m_pNext = pClip->GetNext();
				pClip->SetNext(NULL);

				delete pClip;

				pClip = pTemp->GetNext();
			}
		}
	}

	delete pClippedList;

	return pPolyList;
}


Brush *Brush::CopyList() const
{
	Brush *pBrush = new Brush;

	pBrush->max = max;
	pBrush->min = min;

	pBrush->m_pPolys = m_pPolys->CopyList();

	if (!IsLast())
	{
		pBrush->SetNext(m_pNext->CopyList());
	}

	return pBrush;
}


bool Brush::AABBIntersect(Brush *pBrush_)
{
	if ((min.x > pBrush_->max.x) || (pBrush_->min.x > max.x))
	{
		return false;
	}

	if ((min.y > pBrush_->max.y) || (pBrush_->min.y > max.y))
	{
		return false;
	}

	if ((min.z > pBrush_->max.z) || (pBrush_->min.z > max.z))
	{
		return false;
	}

	return true;
}


void Brush::CalculateAABB()
{
	min = m_pPolys->verts[0].p;
	max = m_pPolys->verts[0].p;

	Poly *pPoly = m_pPolys;

	for (unsigned int i = 0; i < GetNumberOfPolys(); i++)
	{
		for (int j = 0; j < pPoly->GetNumberOfVertices(); j++)
		{
			//
			// Calculate min
			//
			if (pPoly->verts[j].p.x < min.x)
			{
				min.x = pPoly->verts[j].p.x;
			}

			if (pPoly->verts[j].p.y < min.y)
			{
				min.y = pPoly->verts[j].p.y;
			}

			if (pPoly->verts[j].p.z < min.z)
			{
				min.z = pPoly->verts[j].p.z;
			}

			//
			// Calculate max
			//
			if (pPoly->verts[j].p.x > max.x)
			{
				max.x = pPoly->verts[j].p.x;
			}

			if (pPoly->verts[j].p.y > max.y)
			{
				max.y = pPoly->verts[j].p.y;
			}

			if (pPoly->verts[j].p.z > max.z)
			{
				max.z = pPoly->verts[j].p.z;
			}
		}

		pPoly = pPoly->GetNext();
	}
}


Brush::Brush()
{
	m_pNext = NULL;
	m_pPolys = NULL;
}


Brush::~Brush()
{
	if (m_pPolys != NULL)
	{
		delete m_pPolys;
		m_pPolys = NULL;
	}

	if (!IsLast())
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


void Brush::AddPoly(Poly *pPoly_)
{
	if (m_pPolys == NULL)
	{
		m_pPolys = pPoly_;

		return;
	}

	Poly *pPoly = m_pPolys;

	while (!pPoly->IsLast())
	{
		pPoly = pPoly->GetNext();
	}

	pPoly->SetNext(pPoly_);
}


unsigned int Brush::GetNumberOfBrushes() const
{
	Brush			*pBrush = m_pNext;
	unsigned int	uiCount = 1;

	while (pBrush != NULL)
	{
		pBrush = pBrush->GetNext();
		uiCount++;
	}

	return uiCount;
}


unsigned int Brush::GetNumberOfPolys() const
{
	Poly			*pPoly = m_pPolys;
	unsigned int	uiCount = 0;

	while (pPoly != NULL)
	{
		pPoly = pPoly->GetNext();
		uiCount++;
	}

	return uiCount;
}


void Brush::SetNext(Brush *pBrush_)
{
	if (IsLast())
	{
		m_pNext = pBrush_;

		return;
	}

	if (pBrush_ == NULL)
	{
		m_pNext = NULL;
	}
	else
	{
		Brush *pBrush = pBrush_;

		while (!pBrush->IsLast())
		{
			pBrush = pBrush->GetNext();
		}

		pBrush->SetNext(m_pNext);

		m_pNext = pBrush_;
	}
}


bool Brush::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////
// Face member functions
////////////////////////////////////////////////////////////////////

Poly *Face::GetPolys()
{
	//
	// Create the polygons from the faces
	//
	unsigned int	uiFaces = 0;
	Face			*pFace = this;

	while (pFace != NULL)
	{
		pFace = pFace->GetNext();
		uiFaces++;
	}

	Poly			*pPolyList = NULL;
	Face			*lfi = NULL;
	Face			*lfj = NULL;
	Face			*lfk = NULL;

	//
	// Create polygons
	//
	pFace = this;

	for (unsigned int c = 0; c < uiFaces; c++)
	{
		if (pPolyList == NULL)
		{
			pPolyList = new Poly;
		}
		else
		{
			pPolyList->AddPoly(new Poly);
		}

		if (c == uiFaces - 3)
		{
			lfi = pFace->GetNext();
		}
		else if (c == uiFaces - 2)
		{
			lfj = pFace->GetNext();
		}
		else if (c == uiFaces - 1)
		{
			lfk = pFace->GetNext();
		}

		pFace = pFace->GetNext();
	}

	//
	// Loop through faces and create polygons
	//
	Poly	*pi = pPolyList;

	for (Face *fi = this; fi != lfi; fi = fi->GetNext())
	{
		Poly	*pj = pi->GetNext();

		for (Face *fj = fi->GetNext(); fj != lfj; fj = fj->GetNext())
		{
			Poly	*pk = pj->GetNext();

			for (Face *fk = fj->GetNext(); fk != lfk; fk = fk->GetNext())
			{
				Vector3 p;

				if (fi->plane.GetIntersection(fj->plane, fk->plane, p))
				{
					Face *f = this;

					while (true)
					{
						if (f->plane.ClassifyPoint(p) == Plane::eCP::FRONT)
						{
							break;
						}

						if (f->IsLast())	// The point is not outside the brush
						{
							Vertex v;

							v.p = p;

							pi->AddVertex(v);
							pj->AddVertex(v);
							pk->AddVertex(v);

							break;
						}

						f = f->GetNext();
					}
				}

				pk = pk->GetNext();
			}

			pj = pj->GetNext();
		}

		pi = pi->GetNext();
	}

	return pPolyList;
}


Face::Face()
{
	m_pNext = NULL;
}


Face::~Face()
{
	if (!IsLast())
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


bool Face::IsLast() const
{
	if (m_pNext == NULL)
	{
		return true;
	}

	return false;
}


void Face::SetNext(Face *pFace_)
{
	if (IsLast())
	{
		m_pNext = pFace_;

		return;
	}

	//
	// Insert the given list
	//
	Face *pFace = pFace_;

	while (!pFace->IsLast())
	{
		pFace = pFace->GetNext();
	}

	pFace->SetNext(m_pNext);

	m_pNext = pFace_;
}


void Face::AddFace(Face *pFace_)
{
	if (IsLast())
	{
		m_pNext = pFace_;

		return;
	}

	Face *pFace = m_pNext;

	while (!pFace->IsLast())
	{
		pFace = pFace->GetNext();
	}

	pFace->m_pNext = pFace_;
}