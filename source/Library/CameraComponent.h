#pragma once

#include "Component.h"

namespace Library
{
	class CameraComponent : public Component
	{

		RTTI_DECLARATIONS(CameraComponent, Component)

	public:

		enum FRUSTUM_COLLIDE {
			FRUSTUM_OUTSIDE = 0,
			FRUSTUM_INSIDE = 1,
			FRUSTUM_INTERSECT = 2,
		};

		CameraComponent(D3DApp& app);
		
		virtual ~CameraComponent();

		// Get camera basis vectors
		const XMFLOAT3& Direction() const;
		const XMFLOAT3& Up() const;
		const XMFLOAT3& Right() const;
		XMVECTOR DirectionVector() const;
		XMVECTOR UpVector() const;
		XMVECTOR RightVector() const;

		// Get frustum properties
		float GetNearZ()const;
		float GetFarZ()const;
		float GetAspect()const;
		float GetFovY()const;
		float GetFovX()const;

		// Get near and far plane dimensions in view space coordinates
		float GetNearWindowWidth()const;
		float GetNearWindowHeight()const;
		float GetFarWindowWidth()const;
		float GetFarWindowHeight()const;

		// Set frustum.
		virtual void SetLens(float fovY, float aspect, float zn, float zf);

		// Define camera space via LookAt parameters
		void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
		void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

		// Get/Set world camera position
		const XMFLOAT3& Position() const;
		XMVECTOR PositionVector() const;
		virtual void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		virtual void SetPosition(FXMVECTOR position);
		virtual void SetPosition(const XMFLOAT3& position);

		// Get View/Proj matrices
		XMMATRIX ViewMatrix() const;
		XMMATRIX ProjectionMatrix() const;
		XMMATRIX ViewProjectionMatrix() const;

		virtual void Reset();
		virtual void Initialize() override;
		virtual void Update(const Timer &timer) override;
		virtual void UpdateViewMatrix();
		virtual void UpdateViewProjectionMatrix();
		void ApplyRotation(CXMMATRIX transform);
		void ApplyRotation(const XMFLOAT4X4& transform);
		

		FRUSTUM_COLLIDE BoundsInFrustum(const XMFLOAT3 & Min, const XMFLOAT3 & Max, const XMFLOAT4X4 * mtxWorld = NULL, UCHAR * FrustumBits = NULL, signed char * LastOutside = NULL) const;

		// Frustum default values
		static const float DefaultFovY;
		static const float DefaultAspect;
		static const float DefaultNearZ;
		static const float DefaultFarZ;
		
	protected:

		void CalcFrustumPlanes();

		XMFLOAT4 mFrustum[6];

		// Cache frustum properties.
		float mNearZ;
		float mFarZ;
		float mAspect;
		float mFovY;
		float mNearWindowHeight;
		float mFarWindowHeight;

		// CameraComponent coordinate system with coordinates relative to world space
		XMFLOAT3 mPosition;
		XMFLOAT3 mDirection;
		XMFLOAT3 mUp;
		XMFLOAT3 mRight;

		// Cache View/Proj matrices
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;
		XMFLOAT4X4 mViewProj;

	private:
		CameraComponent(const CameraComponent& rhs);
		CameraComponent& operator=(const CameraComponent& rhs);
	};
}

