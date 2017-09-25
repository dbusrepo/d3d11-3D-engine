#pragma once
#include "Common.h"

namespace Library
{
	namespace BSPEngine
	{
		//-----------------------------------------------------------------------------
		// Name : Vertex (Class)
		// Desc : Vertex class used to construct & store vertex components.
		// Note : A simple vertex type, used to store pos/normal/tex coords.
		//-----------------------------------------------------------------------------
		class Vertex
		{
		public:
			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			Vertex(float fX, float fY, float fZ, const XMFLOAT3 & vecNormal, float ftu = 0.0f, float ftv = 0.0f)
			{
				x = fX; y = fY; z = fZ; 
				Normal = vecNormal; 
				tu = ftu; tv = ftv;
			}

			Vertex(XMFLOAT3 & vecPos, const XMFLOAT3 & vecNormal, float ftu = 0.0f, float ftv = 0.0f)
			{
				x = vecPos.x; y = vecPos.y; z = vecPos.z; 
				Normal = vecNormal; 
				tu = ftu; tv = ftv;
			}

			Vertex()
			{
				x = 0.0f; y = 0.0f; z = 0.0f;
				Normal = XMFLOAT3(0, 0, 0);
				tu = 0.0f; tv = 0.0f;
			}

			//-------------------------------------------------------------------------
			// Public Variables for This Class
			//-------------------------------------------------------------------------
			float       x;          // Vertex Position X Component
			float       y;          // Vertex Position Y Component
			float       z;          // Vertex Position Z Component
			XMFLOAT3 Normal;     // Vertex normal.
			XMFLOAT4 Tangent;
			float       tu;         // Texture u coordinate
			float       tv;         // Texture v coordinate
			float       lu;			// lightmap u coordinate
			float	    lv;			// lightmap v coordinate
			uint32_t    leafIndex;     // used in shader for leaf offset
		};

		//-----------------------------------------------------------------------------
		// Name : Polygon (Class)
		// Desc : Basic polygon class used to store this polygons vertex data.
		//-----------------------------------------------------------------------------
		class Polygon
		{
		public:
			//-------------------------------------------------------------------------
			// Constructors & Destructors for This Class.
			//-------------------------------------------------------------------------
			~Polygon();
			Polygon();
			
			//-------------------------------------------------------------------------
			// Public Functions for This Class
			//-------------------------------------------------------------------------
			long            AddVertex(USHORT Count = 1);
			long            InsertVertex(USHORT nVertexPos);
			void		    ComputeTextureSpaceVectors();
			void			ComputeBounds();
			//-------------------------------------------------------------------------
			// Public Variables for This Class
			//-------------------------------------------------------------------------
			bool        m_bVisible;             // Visible for the purposes of rendering?
			uint16_t  m_materialID;
			uint16_t  m_texID;            // Attribute ID of face
			XMFLOAT3 m_normal;            // The face normal.
			XMFLOAT4 m_tangent;
			//XMFLOAT3 m_bitangent;
			USHORT      m_nVertexCount;         // Number of vertices stored.
			Vertex     *m_pVertex;              // Simple vertex array
			XMFLOAT3 m_vecBoundsMin;         // Minimum bounding box extents of this polygon
			XMFLOAT3 m_vecBoundsMax;         // Maximum bounding box extents of this polygon
		};

	}

}

