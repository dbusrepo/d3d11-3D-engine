#include "Common.h"
#include "MatrixHelper.h"
#include "MathUtility.h"
#include "Player.h"
#include "Camera.h"

Library::BSPEngine::Camera::Camera()
{
	m_pPlayer = nullptr;
	m_vecRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_vecUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_vecLook = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_vecPos = XMFLOAT3(0.0f, 0.0f, 0.0f);

	mView = MatrixHelper::Identity;
	mProj = MatrixHelper::Identity;
	mViewProj = MatrixHelper::Identity;

	mViewDirty = true;
	mViewProjDirty = true;
	mFrustumDirty = true;
}

Library::BSPEngine::Camera::~Camera()
{
	
}

float Library::BSPEngine::Camera::GetNearZ()const
{
	return mNearZ;
}

float Library::BSPEngine::Camera::GetFarZ()const
{
	return mFarZ;
}

float Library::BSPEngine::Camera::GetAspect()const
{
	return mAspect;
}

float Library::BSPEngine::Camera::GetFovY()const
{
	return mFovY;
}

float Library::BSPEngine::Camera::GetFovX()const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atan(halfWidth / mNearZ);
}

float Library::BSPEngine::Camera::GetNearWindowWidth()const
{
	return mAspect * mNearWindowHeight;
}

float Library::BSPEngine::Camera::GetNearWindowHeight()const
{
	return mNearWindowHeight;
}

float Library::BSPEngine::Camera::GetFarWindowWidth()const
{
	return mAspect * mFarWindowHeight;
}

float Library::BSPEngine::Camera::GetFarWindowHeight()const
{
	return mFarWindowHeight;
}

void Library::BSPEngine::Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f*mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f*mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);

	mViewProjDirty = true;
}

XMMATRIX Library::BSPEngine::Camera::ViewMatrix() const
{
	return XMLoadFloat4x4(&mView);
}

XMMATRIX Library::BSPEngine::Camera::ProjectionMatrix() const
{
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX Library::BSPEngine::Camera::ViewProjectionMatrix() const
{
	return XMLoadFloat4x4(&mViewProj);
}

void Library::BSPEngine::Camera::Update()
{
	if (mViewDirty) {
		UpdateViewMatrix();
		mViewDirty = false;
		mViewProjDirty = true;
	}

	if (mViewProjDirty) {
		UpdateViewProjectionMatrix();
		mViewProjDirty = false;
		mFrustumDirty = true;
	}

	if (mFrustumDirty) {
		CalcFrustumPlanes();
		mFrustumDirty = false;
	}
}

void Library::BSPEngine::Camera::SetPosition(const XMFLOAT3 & Position)
{
	//m_vecPos = Position; 
	XMStoreFloat3(&m_vecPos, XMLoadFloat3(&Position));

	mViewDirty = true;
}

void Library::BSPEngine::Camera::Move(const XMFLOAT3 & vecShift)
{
	//m_vecPos += vecShift;
	XMStoreFloat3(&m_vecPos, XMVectorAdd(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&vecShift)));

	mViewDirty = true;
}

void Library::BSPEngine::Camera::Rotate(float x, float y, float z)
{
	// Validate requirements
	if (!m_pPlayer) return;

	if (x != 0)
	{
		// Build rotation matrix
		//D3DXMatrixRotationAxis(&mtxRotate, &m_vecRight, D3DXToRadian(x));
		CXMMATRIX mtxRotateX = XMMatrixRotationAxis(XMLoadFloat3(&m_vecRight), XMConvertToRadians(x));
		
		// Update our vectors
		//D3DXVec3TransformNormal(&m_vecLook, &m_vecLook, &mtxRotate);
		XMStoreFloat3(&m_vecLook, XMVector3TransformNormal(XMLoadFloat3(&m_vecLook), mtxRotateX));
		//D3DXVec3TransformNormal(&m_vecUp, &m_vecUp, &mtxRotate);
		XMStoreFloat3(&m_vecUp, XMVector3TransformNormal(XMLoadFloat3(&m_vecUp), mtxRotateX));
		//D3DXVec3TransformNormal(&m_vecRight, &m_vecRight, &mtxRotate);
		XMStoreFloat3(&m_vecRight, XMVector3TransformNormal(XMLoadFloat3(&m_vecRight), mtxRotateX));
	} // End if Pitch

	if (y != 0)
	{
		// Build rotation matrix
		//D3DXMatrixRotationAxis(&mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian(y));
		CXMMATRIX mtxRotateY = XMMatrixRotationAxis(XMLoadFloat3(&m_pPlayer->GetUp()), XMConvertToRadians(y));

		// Adjust camera position
		//m_vecPos -= m_pPlayer->GetPosition();
		XMStoreFloat3(&m_vecPos, XMVectorSubtract(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&m_pPlayer->GetPosition())));
		//D3DXVec3TransformCoord(&m_vecPos, &m_vecPos, &mtxRotate);
		XMStoreFloat3(&m_vecPos, XMVector3TransformNormal(XMLoadFloat3(&m_vecPos), mtxRotateY));
		//m_vecPos += m_pPlayer->GetPosition();
		XMStoreFloat3(&m_vecPos, XMVectorAdd(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&m_pPlayer->GetPosition())));

		// Update our vectors
		//D3DXVec3TransformNormal(&m_vecLook, &m_vecLook, &mtxRotate);
		XMStoreFloat3(&m_vecLook, XMVector3TransformNormal(XMLoadFloat3(&m_vecLook), mtxRotateY));
		//D3DXVec3TransformNormal(&m_vecUp, &m_vecUp, &mtxRotate);
		XMStoreFloat3(&m_vecUp, XMVector3TransformNormal(XMLoadFloat3(&m_vecUp), mtxRotateY));
		//D3DXVec3TransformNormal(&m_vecRight, &m_vecRight, &mtxRotate);
		XMStoreFloat3(&m_vecRight, XMVector3TransformNormal(XMLoadFloat3(&m_vecRight), mtxRotateY));

	} // End if Yaw

	mViewDirty = true;
}

void Library::BSPEngine::Camera::UpdateViewMatrix()
{
	// Because many rotations will cause floating point errors, the axis will eventually become
	// non-perpendicular to one other causing all hell to break loose. Therefore, we must
	// perform base vector regeneration to ensure that all vectors remain unit length and
	// perpendicular to one another. This need not be done on EVERY call to rotate (i.e. you
	// could do this once every 50 calls for instance).

	//D3DXVec3Normalize(&m_vecLook, &m_vecLook);
	XMStoreFloat3(&m_vecLook, XMVector3Normalize(XMLoadFloat3(&m_vecLook)));
	//D3DXVec3Cross(&m_vecRight, &m_vecUp, &m_vecLook);
	XMStoreFloat3(&m_vecRight,
		XMVector3Cross(XMLoadFloat3(&m_vecUp), XMLoadFloat3(&m_vecLook)));
	//D3DXVec3Normalize(&m_vecRight, &m_vecRight);
	XMStoreFloat3(&m_vecRight, XMVector3Normalize(XMLoadFloat3(&m_vecRight)));
	//D3DXVec3Cross( &m_vecUp, &m_vecLook, &m_vecRight );
	XMStoreFloat3(&m_vecUp,
		XMVector3Cross(XMLoadFloat3(&m_vecLook), XMLoadFloat3(&m_vecRight)));
	//D3DXVec3Normalize( &m_vecUp, &m_vecUp );
	XMStoreFloat3(&m_vecUp, XMVector3Normalize(XMLoadFloat3(&m_vecUp)));


	CXMVECTOR eyePosition = XMLoadFloat3(&m_vecPos);
	CXMVECTOR direction = XMLoadFloat3(&m_vecLook);
	CXMVECTOR upDirection = XMLoadFloat3(&m_vecUp);

	XMStoreFloat4x4(&mView, XMMatrixLookToLH(eyePosition, direction, upDirection));
}

void Library::BSPEngine::Camera::UpdateViewProjectionMatrix()
{
	XMStoreFloat4x4(&mViewProj, XMMatrixMultiply(ViewMatrix(), ProjectionMatrix()));
}

Library::BSPEngine::Camera::FRUSTUM_COLLIDE Library::BSPEngine::Camera::BoundsInFrustum(const XMFLOAT3 & vecMin, const XMFLOAT3 & vecMax, const XMFLOAT4X4 * mtxWorld, UCHAR * FrustumBits, char * LastOutside) const
{
	XMFLOAT3      NearPoint, FarPoint, Normal, Min = vecMin, Max = vecMax;
	FRUSTUM_COLLIDE Result = FRUSTUM_INSIDE;
	UCHAR           nBits = 0;
	signed char     nLastOutside = -1;

	// Make a copy of the bits passed in if provided
	if (FrustumBits) nBits = *FrustumBits;

	// Make a copy of the 'last outside' value to prevent us having to dereference
	if (LastOutside) nLastOutside = *LastOutside;

	// Transform bounds if matrix provided
	if (mtxWorld) BSPEngine::TransformAABB(Min, Max, *mtxWorld);

	// If the 'last outside plane' index was specified, test it first!
	if (nLastOutside >= 0 && (((nBits >> nLastOutside) & 0x1) == 0x0))
	{
		// Store the plane normal
		Normal = XMFLOAT3(mFrustum[nLastOutside].x, mFrustum[nLastOutside].y, mFrustum[nLastOutside].z);

		// Calculate near / far extreme points
		if (Normal.x > 0.0f) { FarPoint.x = Max.x; NearPoint.x = Min.x; }
		else { FarPoint.x = Min.x; NearPoint.x = Max.x; }

		if (Normal.y > 0.0f) { FarPoint.y = Max.y; NearPoint.y = Min.y; }
		else { FarPoint.y = Min.y; NearPoint.y = Max.y; }

		if (Normal.z > 0.0f) { FarPoint.z = Max.z; NearPoint.z = Min.z; }
		else { FarPoint.z = Min.z; NearPoint.z = Max.z; }

		if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&mFrustum[nLastOutside]), XMLoadFloat3(&NearPoint))) > 0.0f)
		{
			return FRUSTUM_COLLIDE::FRUSTUM_OUTSIDE;
		}

		if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&mFrustum[nLastOutside]), XMLoadFloat3(&FarPoint))) > 0.0f)
		{
			Result = FRUSTUM_COLLIDE::FRUSTUM_INTERSECT;
		}
		else {
			nBits |= (0x1 << nLastOutside); // We were totally inside this frustum plane, update our bit set
		}
	} // End if last outside plane specified

	  // Loop through all the planes
	for (int i = 0; i < 6; i++)
	{
		// Check the bit in the uchar passed to see if it should be tested (if it's 1, it's already passed)
		if (((nBits >> i) & 0x1) == 0x1) continue;

		// If 'last outside plane' index was specified, skip if it matches the plane index
		if (nLastOutside >= 0 && nLastOutside == (signed char)i) continue;

		// Store the plane normal
		Normal = XMFLOAT3(mFrustum[i].x, mFrustum[i].y, mFrustum[i].z);

		// Calculate near / far extreme points
		if (Normal.x > 0.0f) { FarPoint.x = Max.x; NearPoint.x = Min.x; }
		else { FarPoint.x = Min.x; NearPoint.x = Max.x; }

		if (Normal.y > 0.0f) { FarPoint.y = Max.y; NearPoint.y = Min.y; }
		else { FarPoint.y = Min.y; NearPoint.y = Max.y; }

		if (Normal.z > 0.0f) { FarPoint.z = Max.z; NearPoint.z = Min.z; }
		else { FarPoint.z = Min.z; NearPoint.z = Max.z; }

		// If near extreme point is outside, then the AABB is totally outside the frustum
		if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&mFrustum[i]), XMLoadFloat3(&NearPoint))) > 0.0f)
		{
			// Store the 'last outside' index
			if (LastOutside) *LastOutside = (signed char)i;

			// Store the frustum bits so far and return
			if (FrustumBits) *FrustumBits = nBits;

			return FRUSTUM_COLLIDE::FRUSTUM_OUTSIDE;
		} // End if outside frustum plane

		  // If far extreme point is outside, then the AABB is intersecting the frustum
		if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&mFrustum[i]), XMLoadFloat3(&FarPoint))) > 0.0f)
		{
			Result = FRUSTUM_COLLIDE::FRUSTUM_INTERSECT;
		}
		else {
			nBits |= (0x1 << i); // We were totally inside this frustum plane, update our bit set
		}

	}

	// Store none outside
	if (LastOutside) *LastOutside = -1;

	// Return the result
	if (FrustumBits) *FrustumBits = nBits;

	return Result;
}

void Library::BSPEngine::Camera::CalcFrustumPlanes()
{
	XMFLOAT4X4 &m = mViewProj;

	// Left clipping plane
	mFrustum[0].x = -(m._14 + m._11);
	mFrustum[0].y = -(m._24 + m._21);
	mFrustum[0].z = -(m._34 + m._31);
	mFrustum[0].w = -(m._44 + m._41);

	// Right clipping plane
	mFrustum[1].x = -(m._14 - m._11);
	mFrustum[1].y = -(m._24 - m._21);
	mFrustum[1].z = -(m._34 - m._31);
	mFrustum[1].w = -(m._44 - m._41);

	// Top clipping plane
	mFrustum[2].x = -(m._14 - m._12);
	mFrustum[2].y = -(m._24 - m._22);
	mFrustum[2].z = -(m._34 - m._32);
	mFrustum[2].w = -(m._44 - m._42);

	// Bottom clipping plane
	mFrustum[3].x = -(m._14 + m._12);
	mFrustum[3].y = -(m._24 + m._22);
	mFrustum[3].z = -(m._34 + m._32);
	mFrustum[3].w = -(m._44 + m._42);

	// Near clipping plane
	mFrustum[4].x = -(m._13);
	mFrustum[4].y = -(m._23);
	mFrustum[4].z = -(m._33);
	mFrustum[4].w = -(m._43);

	// Far clipping plane
	mFrustum[5].x = -(m._14 - m._13);
	mFrustum[5].y = -(m._24 - m._23);
	mFrustum[5].z = -(m._34 - m._33);
	mFrustum[5].w = -(m._44 - m._43);

	// Normalize the mFrustum
	for (int i = 0; i < 6; i++) {
		XMStoreFloat4(&mFrustum[i], XMPlaneNormalize(XMLoadFloat4(&mFrustum[i])));
	}

}
