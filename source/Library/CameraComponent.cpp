#include "CameraComponent.h"
#include "D3DApp.h"
#include "Timer.h"
#include "VectorHelper.h"
#include "MathUtility.h"
//#include "MatrixHelper.h"


namespace Library
{
	RTTI_DEFINITIONS(CameraComponent)

	const float CameraComponent::DefaultFovY = XMConvertToRadians(60); //XM_PIDIV4;
	const float CameraComponent::DefaultAspect = 1;
	const float CameraComponent::DefaultNearZ = 1.0f;
	const float CameraComponent::DefaultFarZ = 3000.0f;

	CameraComponent::CameraComponent(D3DApp& app)
		: Component(app)
	{
		//SetLens(DefaultFovY, DefaultAspect, DefaultNearZ, DefaultFarZ);
	}

	CameraComponent::~CameraComponent()
	{
	}

	const XMFLOAT3& CameraComponent::Position() const
	{
		return mPosition;
	}

	const XMFLOAT3& CameraComponent::Direction() const
	{
		return mDirection;
	}

	const XMFLOAT3& CameraComponent::Up() const
	{
		return mUp;
	}

	const XMFLOAT3& CameraComponent::Right() const
	{
		return mRight;
	}

	XMVECTOR CameraComponent::PositionVector() const
	{
		return XMLoadFloat3(&mPosition);
	}

	void CameraComponent::SetPosition(FLOAT x, FLOAT y, FLOAT z)
	{
		XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
		SetPosition(position);
	}

	void CameraComponent::SetPosition(FXMVECTOR position)
	{
		XMStoreFloat3(&mPosition, position);
	}

	void CameraComponent::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;
	}

	XMVECTOR CameraComponent::DirectionVector() const
	{
		return XMLoadFloat3(&mDirection);
	}

	XMVECTOR CameraComponent::UpVector() const
	{
		return XMLoadFloat3(&mUp);
	}

	XMVECTOR CameraComponent::RightVector() const
	{
		return XMLoadFloat3(&mRight);
	}

	float CameraComponent::GetNearZ()const
	{
		return mNearZ;
	}

	float CameraComponent::GetFarZ()const
	{
		return mFarZ;
	}

	float CameraComponent::GetAspect()const
	{
		return mAspect;
	}

	float CameraComponent::GetFovY()const
	{
		return mFovY;
	}

	float CameraComponent::GetFovX()const
	{
		float halfWidth = 0.5f*GetNearWindowWidth();
		return 2.0f*atan(halfWidth / mNearZ);
	}

	float CameraComponent::GetNearWindowWidth()const
	{
		return mAspect * mNearWindowHeight;
	}

	float CameraComponent::GetNearWindowHeight()const
	{
		return mNearWindowHeight;
	}

	float CameraComponent::GetFarWindowWidth()const
	{
		return mAspect * mFarWindowHeight;
	}

	float CameraComponent::GetFarWindowHeight()const
	{
		return mFarWindowHeight;
	}

	void CameraComponent::SetLens(float fovY, float aspect, float zn, float zf)
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
	}

	void CameraComponent::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
	{
		XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
		XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
		XMVECTOR U = XMVector3Cross(L, R);

		XMStoreFloat3(&mPosition, pos);
		XMStoreFloat3(&mDirection, L);
		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
	}

	void CameraComponent::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
	{
		XMVECTOR P = XMLoadFloat3(&pos);
		XMVECTOR T = XMLoadFloat3(&target);
		XMVECTOR U = XMLoadFloat3(&up);

		LookAt(P, T, U);
	}

	XMMATRIX CameraComponent::ViewMatrix() const
	{
		return XMLoadFloat4x4(&mView);
	}

	XMMATRIX CameraComponent::ProjectionMatrix() const
	{
		return XMLoadFloat4x4(&mProj);
	}

	XMMATRIX CameraComponent::ViewProjectionMatrix() const
	{
		return XMLoadFloat4x4(&mViewProj); //XMMatrixMultiply(ViewMatrix(), ProjectionMatrix());
	}

	void CameraComponent::Reset()
	{
		mPosition = Vector3Helper::Zero;
		mDirection = Vector3Helper::Forward;
		mUp = Vector3Helper::Up;
		mRight = Vector3Helper::Right;

		UpdateViewMatrix();
	}

	void CameraComponent::Initialize()
	{
		Reset();
	}

	void CameraComponent::Update(const Timer &timer)
	{
		UpdateViewMatrix();
		UpdateViewProjectionMatrix();
		CalcFrustumPlanes();
	}

	void CameraComponent::UpdateViewMatrix()
	{
		XMVECTOR eyePosition = XMLoadFloat3(&mPosition);
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR upDirection = XMLoadFloat3(&mUp);

		XMMATRIX viewMatrix = XMMatrixLookToLH(eyePosition, direction, upDirection);
		XMStoreFloat4x4(&mView, viewMatrix);
	}

	void CameraComponent::UpdateViewProjectionMatrix()
	{
		XMStoreFloat4x4(&mViewProj, XMMatrixMultiply(ViewMatrix(), ProjectionMatrix()));
	}

	void CameraComponent::ApplyRotation(const XMFLOAT4X4& transform)
	{
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		ApplyRotation(transformMatrix);
	}

	CameraComponent::FRUSTUM_COLLIDE CameraComponent::BoundsInFrustum(const XMFLOAT3 & vecMin, const XMFLOAT3 & vecMax, const XMFLOAT4X4 * mtxWorld, UCHAR * FrustumBits, signed char * LastOutside) const
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

	void CameraComponent::CalcFrustumPlanes()
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

	void CameraComponent::ApplyRotation(CXMMATRIX transform)
	{
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR up = XMLoadFloat3(&mUp);

		direction = XMVector3TransformNormal(direction, transform);
		direction = XMVector3Normalize(direction);

		up = XMVector3TransformNormal(up, transform);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(up, direction);
		up = XMVector3Cross(direction, right);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);
	}

}