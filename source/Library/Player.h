#pragma once
#include "Common.h"

namespace Library
{
	namespace BSPEngine
	{
		class Camera;
		class Scene;

		struct VOLUME_INFO     // Stores information about our object volume
		{
			XMFLOAT3 Min;            // Minimum object space extents of the volume
			XMFLOAT3 Max;            // Maximum object space extents of the volume
		};

		enum DIRECTION {
			DIR_FORWARD = 1,
			DIR_BACKWARD = 2,
			DIR_LEFT = 4,
			DIR_RIGHT = 8,
			DIR_UP = 16,
			DIR_DOWN = 32,
		};

		class Player {
		public:
			Player(Scene *pScene);
			~Player();

			void InitCamera();
			void Update(float TimeScale, bool flyMode);

			void                SetGravity(const XMFLOAT3& Gravity) { m_vecGravity = Gravity; }
			void                SetVelocity(const XMFLOAT3& Velocity) { m_vecVelocity = Velocity; }
			void                SetTraction(float Traction) { m_fTraction = Traction; }
			void                SetSurfaceFriction(float Friction) { m_fSurfaceFriction = Friction; }
			void                SetAirResistance(float Resistance) { m_fAirResistance = Resistance; }
			void                SetMass(float Mass) { m_fMass = Mass; }
			void                SetCamOffset(const XMFLOAT3& Offset);
			void                SetVolumeInfo(const VOLUME_INFO& Volume);
			const VOLUME_INFO&  GetVolumeInfo() const;

			Camera * GetCamera() const { return m_pCamera; }
			const XMFLOAT3 & GetVelocity() const { return m_vecVelocity; }
			
			const XMFLOAT3 & GetPosition() const { return m_vecPos; }
			const XMFLOAT3 & GetOldPosition() const { return m_vecOldPos; }
			const XMFLOAT3 & GetLook() const { return m_vecLook; }
			const XMFLOAT3 & GetUp() const { return m_vecUp; }
			const XMFLOAT3 & GetRight() const { return m_vecRight; }

			float               GetYaw() const { return m_fYaw; }
			float               GetPitch() const { return m_fPitch; }
			float               GetRoll() const { return m_fRoll; }

			void                SetPosition(const XMFLOAT3& Position);

			void                Move(ULONG Direction, float Distance);
			void                Move(const XMFLOAT3& vecShift);
			void                ApplyForce(ULONG Direction, float Force, bool flyMode);
			void                Rotate(float x, float y, float z);

			void                SetOnFloor(bool OnFloor = true);
			bool                GetOnFloor() const;

			void                EnableCollision(bool Enabled = true) { m_bCollision = Enabled; }

			void SetCrouching(bool state) { m_bCrouching = state; }
			
			bool IsCrouching() { return m_bCrouching || m_crouchMovementOn; }
			void SetCamPHeightRatio(float ratio) { m_camPlayerHeightRatio = ratio; }
		private:

			const float CROUCH_HEIGHT_REDUCE_FACTOR = 2.f / 3;
			const float CROUCH_VOLUME_REDUCE_Y = 0.9f;

			Scene *m_pScene;
			Camera *m_pCamera; // Our current camera object
			VOLUME_INFO m_Volume; // Stores information about players collision volume

			// crouch related variables
			float m_playerHeight; // max volume y radius
			//float m_crouchMinRadiusY; // least value possible
			bool m_crouchMovementOn = false;
			bool m_bCrouching = false;
			// end crouch related variables
			float m_camPlayerHeightRatio;
			
			void UpdateCrouchMovement(float timeScale);

			// Players position and orientation values
			XMFLOAT3     m_vecPos;               // Player Position
			XMFLOAT3     m_vecOldPos;            // Player position prior to move.
			XMFLOAT3     m_vecUp;                // Player Up Vector
			XMFLOAT3     m_vecRight;             // Player Right Vector
			XMFLOAT3     m_vecLook;              // Player Look Vector
			XMFLOAT3     m_vecCamOffset;         // Camera offset
			float        m_fPitch;               // Player pitch
			float        m_fRoll;                // Player roll
			float        m_fYaw;                 // Player yaw

			// Force / Player Update Variables
			XMFLOAT3     m_vecVelocity;          // Movement velocity vector
			XMFLOAT3     m_vecAppliedForce;      // Our motor force
			XMFLOAT3     m_vecGravity;           // Gravity vector
			float        m_fTraction;            // How much traction we can apply
			float        m_fAirResistance;       // Air resistance coefficient.
			float        m_fSurfaceFriction;     // Fake Surface friction scalar to cause is to decelerate
			float        m_fMass;                // Mass of player

			// Floor contact variables
			float m_fOffFloorTime;        // How long have we been off the floor if at all

		
			// State Variables
			bool m_bCollision;           // Collision detection is enabled?
			
		};

	}
}