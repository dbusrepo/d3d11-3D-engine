#pragma once
#include "Player.h"

namespace Library
{

	namespace BSPEngine
	{
		class Camera {
		public:

			enum FRUSTUM_COLLIDE {
				FRUSTUM_OUTSIDE = 0,
				FRUSTUM_INSIDE = 1,
				FRUSTUM_INTERSECT = 2,
			};

			Camera();
			~Camera();

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

			const XMFLOAT3&  GetPosition() const { return m_vecPos; }
			const XMFLOAT3&  GetLook() const { return m_vecLook; }
			const XMFLOAT3&  GetUp() const { return m_vecUp; }
			const XMFLOAT3&  GetRight() const { return m_vecRight; }

			// Set frustum.
			void SetLens(float fovY, float aspect, float zn, float zf);

			// Get View/Proj matrices
			XMMATRIX ViewMatrix() const;
			XMMATRIX ProjectionMatrix() const;
			XMMATRIX ViewProjectionMatrix() const;

			Player *GetPlayer() const { return m_pPlayer; }
			void AttachToPlayer(Player * pPlayer) { m_pPlayer = pPlayer; }
			void DetachFromPlayer() { m_pPlayer = nullptr; }

			void  SetLook(const XMFLOAT3& Vector) { m_vecLook = Vector; }
			void  SetUp(const XMFLOAT3& Vector) { m_vecUp = Vector; }
			void  SetRight(const XMFLOAT3& Vector) { m_vecRight = Vector; }

			void Update();
			void SetPosition(const XMFLOAT3& Position);
			void Move(const XMFLOAT3& vecShift);
			void Rotate(float x, float y, float z);

			void UpdateViewMatrix();
			void UpdateViewProjectionMatrix();


			Camera::FRUSTUM_COLLIDE BoundsInFrustum(const XMFLOAT3 & Min, const XMFLOAT3 & Max, const XMFLOAT4X4 * mtxWorld = NULL, UCHAR * FrustumBits = NULL, char * LastOutside = NULL) const;

		protected:
			Player *m_pPlayer; // The player object we are attached to
			VOLUME_INFO     m_Volume; // Stores information about cameras collision volume
		
			void CalcFrustumPlanes();

			XMFLOAT4 mFrustum[6];

			// Cache frustum properties.
			float mNearZ;
			float mFarZ;
			float mAspect;
			float mFovY;
			float mNearWindowHeight;
			float mFarWindowHeight;

			// coordinate system with coordinates relative to world space
			XMFLOAT3 m_vecPos;
			XMFLOAT3 m_vecLook;
			XMFLOAT3 m_vecUp;
			XMFLOAT3 m_vecRight;

			// Cache View/Proj matrices
			XMFLOAT4X4 mView;
			XMFLOAT4X4 mProj;
			XMFLOAT4X4 mViewProj;

			bool mViewDirty; // View matrix dirty ?
			bool mViewProjDirty; // Proj matrix dirty ?
			bool mFrustumDirty; // Frustum planes dirty ?

		};

	}
}