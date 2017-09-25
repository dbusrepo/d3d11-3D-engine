#include "Polygon.h"


//-----------------------------------------------------------------------------
// Name : CPolygon () (Constructor)
// Desc : CPolygon Class Constructor
//-----------------------------------------------------------------------------
Library::BSPEngine::Polygon::Polygon()
{
	// Reset / Clear all required values
	m_nVertexCount = 0;
	m_materialID = 0;
	m_texID = 0;
	m_pVertex = NULL;
	m_normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_bVisible = true;
}

//-----------------------------------------------------------------------------
// Name : ~Polygon () (Destructor)
// Desc : Polygon Class Destructor
//-----------------------------------------------------------------------------
Library::BSPEngine::Polygon::~Polygon()
{
	// Release our vertices & Indices
	if (m_pVertex) delete[]m_pVertex;

	// Clear variables
	m_nVertexCount = 0;
	m_texID = 0;
	m_pVertex = NULL;
	m_normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this polygon.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long Library::BSPEngine::Polygon::AddVertex(USHORT Count)
{
	Vertex * pVertexBuffer = NULL;

	// Allocate new resized array
	if (!(pVertexBuffer = new Vertex[m_nVertexCount + Count])) return -1;

	// Existing Data?
	if (m_pVertex)
	{
		// Copy old data into new buffer
		memcpy(pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(Vertex));

		// Release old buffer
		delete[]m_pVertex;

	} // End if

	  // Store pointer for new buffer
	m_pVertex = pVertexBuffer;
	m_nVertexCount += Count;

	// Return first vertex
	return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : InsertVertex ()
// Desc : Inserts a single vertex at the position specified by nVertexPos.
//-----------------------------------------------------------------------------
long Library::BSPEngine::Polygon::InsertVertex(USHORT nVertexPos)
{
	Vertex *VertexBuffer = NULL;

	// Add a vertex to the end and new indices
	if (AddVertex(1) < 0) return -1;

	// Reshuffle the array unlesss we have inserted at the end
	if (nVertexPos != m_nVertexCount - 1)
	{
		// Move all the verts after the insert location, up by 1.
		memmove(&m_pVertex[nVertexPos + 1], &m_pVertex[nVertexPos], ((m_nVertexCount - 1) - nVertexPos) * sizeof(Vertex));

	} // End if not at end.

	  // Overwrite data with default values
	m_pVertex[nVertexPos] = Vertex(0.0f, 0.0f, 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));

	// Return the position
	return nVertexPos;
}

void Library::BSPEngine::Polygon::ComputeTextureSpaceVectors()
{
	XMFLOAT3 edge0, edge1;
	XMFLOAT3 v0, v1, v2;
	XMFLOAT2 uv0, uv1, uv2;
	float area;
	int i0 = 0;
	int i1 = 1;
	int i2 = 2;
	do {
		// test on i2 out of bound index ?
		v0 = XMFLOAT3(m_pVertex[i0].x, m_pVertex[i0].y, m_pVertex[i0].z);
		v1 = XMFLOAT3(m_pVertex[i1].x, m_pVertex[i1].y, m_pVertex[i1].z);
		v2 = XMFLOAT3(m_pVertex[i2].x, m_pVertex[i2].y, m_pVertex[i2].z);
		uv0 = XMFLOAT2(m_pVertex[i0].tu, m_pVertex[i0].tv);
		uv1 = XMFLOAT2(m_pVertex[i1].tu, m_pVertex[i1].tv);
		uv2 = XMFLOAT2(m_pVertex[i2].tu, m_pVertex[i2].tv);

		i1 = i2;
		i2++;

		//XMFLOAT3 edge0 = v1 - v0;
		XMStoreFloat3(&edge0, XMVectorSubtract(
			XMLoadFloat3(&v1), XMLoadFloat3(&v0)
		));

		//XMFLOAT3 edge1 = v2 - v0;
		XMStoreFloat3(&edge1, XMVectorSubtract(
			XMLoadFloat3(&v2), XMLoadFloat3(&v0)
		));

		area = XMVectorGetX(XMVector3Length(XMVector3Cross(XMLoadFloat3(&edge0),
			XMLoadFloat3(&edge1))));

	} while (area == 0.0f);

	//XMFLOAT3 deltaUV0 = uv1 - uv0;
	XMFLOAT2 deltaUV0;
	XMStoreFloat2(&deltaUV0, XMVectorSubtract(
		XMLoadFloat2(&uv1), XMLoadFloat2(&uv0)
	));

	//XMFLOAT3 deltaUV1 = uv2 - uv0;
	XMFLOAT2 deltaUV1;
	XMStoreFloat2(&deltaUV1, XMVectorSubtract(
		XMLoadFloat2(&uv2), XMLoadFloat2(&uv0)
	));

	float f = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

	XMFLOAT3 tangent;
	tangent.x = f * (deltaUV1.y * edge0.x - deltaUV0.y * edge1.x);
	tangent.y = f * (deltaUV1.y * edge0.y - deltaUV0.y * edge1.y);
	tangent.z = f * (deltaUV1.y * edge0.z - deltaUV0.y * edge1.z);
	//tangent = tangent.Normalize();
	XMStoreFloat3(&tangent, XMVector3Normalize(XMLoadFloat3(&tangent)));

	XMFLOAT3 bitangent;
	bitangent.x = f * (-deltaUV1.x * edge0.x + deltaUV0.x * edge1.x);
	bitangent.y = f * (-deltaUV1.x * edge0.y + deltaUV0.x * edge1.y);
	bitangent.z = f * (-deltaUV1.x * edge0.z + deltaUV0.x * edge1.z);
	//bitangent = bitangent.Normalize();
	XMStoreFloat3(&bitangent, XMVector3Normalize(XMLoadFloat3(&bitangent)));

	float dotCross = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&bitangent),
						XMVector3Cross(XMLoadFloat3(&m_normal), XMLoadFloat3(&tangent))));
	float w = (dotCross < 0.0f) ? -1.0f : 1.0f;

	m_tangent = XMFLOAT4(tangent.x, tangent.y, tangent.z, w);
	//m_bitangent = bitangent;
}

void Library::BSPEngine::Polygon::ComputeBounds()
{
	XMFLOAT3 & Min = m_vecBoundsMin;
	XMFLOAT3 & Max = m_vecBoundsMax;

	// Reset bounding box
	Min = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	Max = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// Build polygon bounds
	for (int i = 0; i < m_nVertexCount; ++i)
	{
		Vertex & vtx = m_pVertex[i];
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
}
