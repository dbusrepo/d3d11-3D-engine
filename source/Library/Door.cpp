#include "Door.h"
#include "Polygon.h"

Library::BSPEngine::Door::Door()
	: BrushEntity()
{

}

Library::BSPEngine::Door::~Door()
{
}

void Library::BSPEngine::Door::Init(size_t index,const XMFLOAT3 & bmin, const XMFLOAT3 & bmax, float distToOpen)
{
	SetIndex(index);
	SetBounds(bmin, bmax);
	m_distToOpen = distToOpen;
	m_state = CLOSE;
}



void Library::BSPEngine::Door::Update(float timeScale)
{

	m_hasMoved = false;

	switch (m_state) {
	case	CLOSE:
		//printf("door %zd closed\n", m_index);
		break;
	case	OPENING:
		//printf("door %zd opening\n", m_index);
		StepOpen(timeScale);
		m_hasMoved = true;
		break;
	case	OPEN:
		if (!m_pBspTree->CheckDoorOpenCondition(this, false)) {
			m_state = CLOSING;
		}
		//printf("door %zd opened\n", m_index);
		break;
	case	CLOSING:
		if (m_pBspTree->CheckDoorOpenCondition(this, false)) {
			m_state = OPENING;
			break;
		}
		//printf("door %zd closing\n", m_index);
		StepClose(timeScale);
		m_hasMoved = true;
		break;
	default:
		break;
	}

	if (m_hasMoved) {
		UpdateBspLeaves();
	}
}

bool Library::BSPEngine::Door::BuildRenderData(BSPTree * pBspTree, DoorMaterial * pDoorMaterial)
{
	BrushEntity::BuildRenderData(pBspTree, pDoorMaterial);
	UpdateBspLeaves();
	return true;
}

void Library::BSPEngine::Door::UpdateBspLeaves()
{

	for (auto it = begin(m_leafVector); it != end(m_leafVector); ++it)
	{
		BSPTreeLeaf *leaf = *it;
		leaf->removeDoor(m_index);
	} // Next Data Item


	m_leafVector.clear();
	m_pBspTree->CollectLeavesAABB(m_leafVector, m_boundsMin, m_boundsMax);

	for (auto it = begin(m_leafVector); it != end(m_leafVector); ++it)
	{
		BSPTreeLeaf *leaf = *it;
		leaf->addDoor(m_index);
	}
}

void Library::BSPEngine::Door::StepOpen(float timeScale)
{
}

void Library::BSPEngine::Door::StepClose(float timeScale)
{
}

bool Library::BSPEngine::Door::CheckOpenCondition(const XMFLOAT3 & pos)
{
	/*
	XMFLOAT3 center;

	XMStoreFloat3(&center,
		XMVectorMultiply(XMVectorAdd(XMLoadFloat3(&m_boundsMin), XMLoadFloat3(&m_boundsMax)),
			XMVectorReplicate(0.5f)));

	float dist = XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(XMLoadFloat3(&pos), XMLoadFloat3(&center))));
	return (dist < m_distToOpen);
	*/
	return false;
}

XMFLOAT3 Library::BSPEngine::Door::computeBBoxCenter()
{
	XMFLOAT3 center;

	XMStoreFloat3(&center,
		XMVectorMultiply(XMVectorAdd(XMLoadFloat3(&m_boundsMin), XMLoadFloat3(&m_boundsMax)),
			XMVectorReplicate(0.5f)));

	return center;
}

Library::BSPEngine::TMoveDoor::TMoveDoor()
	: Door()
{
	
}

Library::BSPEngine::TMoveDoor::~TMoveDoor()
{
}

void Library::BSPEngine::TMoveDoor::Init(size_t index, const XMFLOAT3 & bmin, const XMFLOAT3 & bmax, const XMFLOAT3 & moveVec, float speed, float distToOpen)
{
	Door::Init(index, bmin, bmax, distToOpen);
	
	m_origCenter = computeBBoxCenter();
	//m_curCenter = m_origCenter;

	// destination point
	//XMFLOAT3 dstPoint;
	//XMStoreFloat3(&dstPoint,
	//	XMVectorAdd(XMLoadFloat3(&m_origCenter), XMLoadFloat3(&moveVec)));
	m_moveVec = moveVec;
	float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_moveVec)));
	m_tstep = speed / dist;
	/*
	XMStoreFloat3(&m_moveVec,
		XMVectorMultiply(
			XMLoadFloat3(&moveVec),
			XMVectorReplicate(1.f / dist)));
			*/
	m_tdist = 0.f;
}

bool Library::BSPEngine::TMoveDoor::CheckOpenCondition(const XMFLOAT3 & pos)
{
	float dist = XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(XMLoadFloat3(&pos), XMLoadFloat3(&m_origCenter))));
	return (dist < m_distToOpen);
}

void Library::BSPEngine::TMoveDoor::StepOpen(float timeScale)
{
	float tdist = m_tstep * timeScale;

	if (tdist > 1.f - m_tdist) {
		tdist = 1.f - m_tdist;
		//m_tdist = 1.f;
		m_state = OPEN;
	}

	m_tdist += tdist;

	XMStoreFloat4x4(&m_model,
		XMMatrixTranslationFromVector(
			XMVectorMultiply(XMLoadFloat3(&m_moveVec),
				XMVectorReplicate(m_tdist))));

	XMFLOAT3 tVec;
	XMStoreFloat3(&tVec,
		XMVectorMultiply(XMLoadFloat3(&m_moveVec),
			XMVectorReplicate(tdist)));

	UpdateBBox(tVec);
	UpdatePolygons(tVec);
	
}

void Library::BSPEngine::TMoveDoor::StepClose(float timeScale)
{
	float tdist = m_tstep * timeScale;

	if (tdist > m_tdist) {
		tdist = m_tdist;
		m_state = CLOSE;
	}

	m_tdist -= tdist;

	XMStoreFloat4x4(&m_model,
		XMMatrixTranslationFromVector(
			XMVectorMultiply(XMLoadFloat3(&m_moveVec),
				XMVectorReplicate(m_tdist))));

	XMFLOAT3 tVec;
	XMStoreFloat3(&tVec,
		XMVectorMultiply(XMLoadFloat3(&m_moveVec),
			XMVectorReplicate(-tdist)));

	UpdateBBox(tVec);
	UpdatePolygons(tVec);
}

void Library::BSPEngine::TMoveDoor::UpdateBBox(const XMFLOAT3 &tVec)
{
	XMStoreFloat3(&m_boundsMin,
		XMVectorAdd(XMLoadFloat3(&m_boundsMin),
			XMLoadFloat3(&tVec)));
	
	XMStoreFloat3(&m_boundsMax,
		XMVectorAdd(XMLoadFloat3(&m_boundsMax),
			XMLoadFloat3(&tVec)));

	/*
	XMStoreFloat3(&m_curCenter,
		XMVectorAdd(XMLoadFloat3(&m_curCenter),
			XMLoadFloat3(&tVec)));
	*/
}

void Library::BSPEngine::TMoveDoor::UpdatePolygons(const XMFLOAT3 & tVec)
{
	float dx = tVec.x;
	float dy = tVec.y;
	float dz = tVec.z;

	size_t numPolygons = GetPolygonCount();
	for (size_t i = 0; i != numPolygons; ++i)
	{
		Polygon *polygon = GetPolygon(i);
		
		size_t numVertices = polygon->m_nVertexCount;
		for (size_t j = 0; j != numVertices; ++j) 
		{
			polygon->m_pVertex[j].x += dx;
			polygon->m_pVertex[j].y += dy;
			polygon->m_pVertex[j].z += dz;
		}

		// update bounds
		XMStoreFloat3(&polygon->m_vecBoundsMin,
			XMVectorAdd(XMLoadFloat3(&polygon->m_vecBoundsMin),
				XMLoadFloat3(&tVec)));

		XMStoreFloat3(&polygon->m_vecBoundsMax,
			XMVectorAdd(XMLoadFloat3(&polygon->m_vecBoundsMax),
				XMLoadFloat3(&tVec)));

	}
}



