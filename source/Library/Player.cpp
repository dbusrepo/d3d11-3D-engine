#include "Camera.h"
#include "Scene.h"
#include "Player.h"

#include <iostream>

Library::BSPEngine::Player::Player(Scene *pScene)
{
	m_pScene = pScene;

	// Clear any required variables
	m_pCamera = nullptr;

	// Players position & orientation (independent of camera)
	m_vecPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_vecRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_vecUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_vecLook = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	// The following force related values are used in conjunction with 'Update' only
	m_vecVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_vecGravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_vecAppliedForce = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTraction = 1.0f;
	m_fAirResistance = 0.001f;
	m_fSurfaceFriction = 10.0f;
	m_fMass = 3.0f;

	// Camera offset values (from the players origin)
	m_vecCamOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Default volume information
	m_Volume.Min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Volume.Max = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Used for determining if we are on the floor or not
	m_fOffFloorTime = 1.0f;

	// Collision detection on by default
	m_bCollision = true;
}

Library::BSPEngine::Player::~Player()
{
	// Release any allocated memory
	if (m_pCamera) delete m_pCamera;

	// Clear required values
	m_pCamera = nullptr;
	m_pScene = nullptr;
}

void Library::BSPEngine::Player::InitCamera()
{
	Camera * pNewCamera = NULL;
	
	pNewCamera = new Camera;

	// Attach the new camera to 'this' player object
	pNewCamera->AttachToPlayer(this);

	// Reset common camera values
	pNewCamera->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
	pNewCamera->SetUp(XMFLOAT3(0.0f, 1.0f, 0.0f));
	pNewCamera->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));

	// Set position
	// pNewCamera->SetPosition(m_vecPos + m_vecCamOffset);
	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos,
		XMVectorAdd(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&m_vecCamOffset))
	);

	pNewCamera->SetPosition(camPos);

	// Rotate camera back into place
	pNewCamera->Rotate(m_fPitch, m_fYaw, m_fRoll);

	// Destroy our old camera and replace with our new one
	if (m_pCamera) delete m_pCamera;
	m_pCamera = pNewCamera;
}

//-----------------------------------------------------------------------------
// Name : Update ()
// Desc : Update the players position based on the current velocity / gravity
//        settings. These will be scaled by the TimeScale factor passed in.
//-----------------------------------------------------------------------------
void Library::BSPEngine::Player::Update(float TimeScale, bool flyMode)
{
	XMFLOAT3 vecTractive, vecDrag, vecFriction, vecForce, vecAccel;
	bool        bUpdated = false;
	float       fSpeed;
	
	float fForce = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecAppliedForce)));

	if (flyMode) {

		if (fForce > 0.1f) {
			//Move(m_vecAppliedForce);
			//m_vecAppliedForce = XMFLOAT3(0.0f, 0.0f, 0.0f);
			//m_vecVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
			//return;

			//vecAccel = m_vecAppliedForce / m_fMass;
			XMStoreFloat3(&vecAccel,
				XMVectorDivide(XMLoadFloat3(&m_vecAppliedForce), XMVectorReplicate(m_fMass)));
			//m_vecVelocity += vecAccel * TimeScale;
			XMStoreFloat3(&m_vecVelocity,
				XMVectorMultiply(
					XMLoadFloat3(&vecAccel),
					XMVectorReplicate(TimeScale)
				)
			);
			// Just move
			XMFLOAT3 moveVec;
			// m_vecVelocity * TimeScale
			XMStoreFloat3(&moveVec,
				XMVectorMultiply(XMLoadFloat3(&m_vecVelocity), XMVectorReplicate(TimeScale)));

			Move(moveVec);
		}

		m_vecAppliedForce = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_vecVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		return;
	}

	// Scale our traction force by the amount we have available.
	//m_vecAppliedForce *= m_fTraction;
	XMStoreFloat3(&m_vecAppliedForce,
			XMVectorMultiply(XMLoadFloat3(&m_vecAppliedForce), 
				             XMVectorReplicate(m_fTraction)));

	// First calculate the tractive force of the body
	//vecTractive = m_vecAppliedForce + (m_vecGravity * m_fMass);
	XMStoreFloat3(&vecTractive,
		XMVectorMultiplyAdd(
			XMLoadFloat3(&m_vecGravity),
			XMVectorReplicate(m_fMass),
			XMLoadFloat3(&m_vecAppliedForce)
		)
	);

	// Now calculate the speed the body is currently moving
	//fSpeed = D3DXVec3Length(&m_vecVelocity);
	fSpeed = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecVelocity)));

	// Calculate drag / air resistance (relative to the speed squared).
	//vecDrag = -m_fAirResistance * (m_vecVelocity * fSpeed);
	XMStoreFloat3(&vecDrag,
		XMVectorMultiply(XMLoadFloat3(&m_vecVelocity), XMVectorReplicate(-m_fAirResistance * fSpeed)));

	// Calculate the friction force
	//vecFriction = -(m_fTraction * m_fSurfaceFriction) * m_vecVelocity;
	XMStoreFloat3(&vecFriction,
		XMVectorMultiply(XMLoadFloat3(&m_vecVelocity), XMVectorReplicate(-m_fTraction * m_fSurfaceFriction)));

	// Calculate our final force vector
	//vecForce = vecTractive + vecDrag + vecFriction;
	XMStoreFloat3(&vecForce,
		XMVectorAdd(
			XMLoadFloat3(&vecTractive),
			XMVectorAdd(
				XMLoadFloat3(&vecDrag),
				XMLoadFloat3(&vecFriction)
			)
		)
	);

	// Now calculate acceleration
	//vecAccel = vecForce / m_fMass;
	XMStoreFloat3(&vecAccel,
		XMVectorDivide(XMLoadFloat3(&vecForce), XMVectorReplicate(m_fMass)));

	UpdateCrouchMovement(TimeScale);

	// Static Friction update!
	bool bStaticApplied = false;
	//float fVel = D3DXVec3Length(&m_vecVelocity);
	//float fVel = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecVelocity)));
	//bool fForce = D3DXVec3Length(&m_vecAppliedForce);


	if (GetOnFloor() && fSpeed < 500.0f * TimeScale && fForce < 1.0f)
	{
		bStaticApplied = true;
		m_vecVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	} // End if apply 'static friction'
	else
	{
		// Finally apply the acceleration for this frame
		//m_vecVelocity += vecAccel * TimeScale;
		XMStoreFloat3(&m_vecVelocity,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&vecAccel),
				XMVectorReplicate(TimeScale),
				XMLoadFloat3(&m_vecVelocity)
			)
		);
	} // End if accelerate as normal

	if (GetOnFloor() && m_crouchMovementOn) {
		// -m_vecPos.y; //use a negative offset as velocity
		float delta = (m_Volume.Max.y - m_Volume.Min.y);
		m_vecVelocity.y += -delta;// *(m_bCrouching ? -1 : 0.1);
		m_crouchMovementOn = (delta != m_playerHeight);
	}

	// Reset our 'motor' force.
	m_vecAppliedForce = XMFLOAT3(0.0f, 0.0f, 0.0f);
		
	if (m_bCollision) {
		// invoke scene update player for collision detection
		if (m_pScene->UpdatePlayer(this, TimeScale, false)) {		
			bUpdated = true;
		}
	} // End if collision enabled

	if (!bUpdated)
	{
		// Just move
		XMFLOAT3 moveVec;
		// m_vecVelocity * TimeScale
		XMStoreFloat3(&moveVec,
			XMVectorMultiply(XMLoadFloat3(&m_vecVelocity), XMVectorReplicate(TimeScale)));
		
		Move(moveVec);

	} // End if collision disabled

	  // If we've been off the ground for more than 200 milliseconds
	  // then lower traction (We leave them with a little bit of control, 
	  // i.e. if they flap their arms hard enough ;)
	if (!GetOnFloor())
	{
		SetTraction(0.1f);
		SetSurfaceFriction(0.0f);

	} // End if not on floor
	else
	{
		SetTraction(1.0f);
		SetSurfaceFriction(10.0f);

	} // End if on floor

	  // Increment timer
	if (!bStaticApplied) m_fOffFloorTime += TimeScale;

	// Update the objects actions (done prior to deceleration to ensure that
	// all properties we may rely on are those used for THIS frame's updates.)
	//ActionUpdate(TimeScale);
}

void Library::BSPEngine::Player::SetCamOffset(const XMFLOAT3 & Offset)
{
	m_vecCamOffset = Offset;

	if (!m_pCamera) return;
	
	//m_pCamera->SetPosition(m_vecPos + Offset);
	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos,
		XMVectorAdd(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&Offset))
	);
	
	m_pCamera->SetPosition(newPos);
}

//-----------------------------------------------------------------------------
// Name : SetVolumeInfo ()
// Desc : Set the players collision volume information
//-----------------------------------------------------------------------------
void Library::BSPEngine::Player::SetVolumeInfo(const VOLUME_INFO & Volume)
{
	m_Volume = Volume;
	m_playerHeight = Volume.Max.y - Volume.Min.y;
	//m_crouchMinRadiusY = m_radiusY / 3;
}

//-----------------------------------------------------------------------------
// Name : GetVolumeInfo ()
// Desc : Retrieve the players collision volume information
//-----------------------------------------------------------------------------
const Library::BSPEngine::VOLUME_INFO & Library::BSPEngine::Player::GetVolumeInfo() const
{
	return m_Volume;
}

void Library::BSPEngine::Player::SetPosition(const XMFLOAT3 & Position)
{
	//Move(Position - m_vecPos);
	XMFLOAT3 moveVec;
	XMStoreFloat3(&moveVec, XMVectorSubtract(XMLoadFloat3(&Position), XMLoadFloat3(&m_vecPos)));
	Move(moveVec);
}

//-----------------------------------------------------------------------------
// Name : Move ()
// Desc : Move the camera in the specified direction for the specified distance
// Note : Specify 'true' to velocity if you wish the move to affect the cameras
//        velocity rather than it's absolute position
//-----------------------------------------------------------------------------
void Library::BSPEngine::Player::Move(ULONG Direction, float Distance)
{
	XMFLOAT3 vecShift = XMFLOAT3(0, 0, 0);

	// Which direction are we moving ?
	if (Direction & DIR_FORWARD) {
		//vecShift += m_vecLook  * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecLook),
				XMVectorReplicate(Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}
	if (Direction & DIR_BACKWARD) {
		//vecShift -= m_vecLook  * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecLook),
				XMVectorReplicate(-Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}
	if (Direction & DIR_RIGHT) {
		//vecShift += m_vecRight * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecRight),
				XMVectorReplicate(Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}
	if (Direction & DIR_LEFT) {
		//vecShift -= m_vecRight * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecRight),
				XMVectorReplicate(-Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}
	if (Direction & DIR_UP) {
		//vecShift += m_vecUp    * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecUp),
				XMVectorReplicate(Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}
	if (Direction & DIR_DOWN) {
		//vecShift -= m_vecUp    * Distance;
		XMStoreFloat3(&vecShift,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&m_vecUp),
				XMVectorReplicate(-Distance),
				XMLoadFloat3(&vecShift)
			)
		);
	}

	// Update camera vectors
	if (Direction) Move(vecShift);
}

//-----------------------------------------------------------------------------
// Name : Move ()
// Desc : Move the camera by the specified amount based on the vector passed.
//-----------------------------------------------------------------------------
void Library::BSPEngine::Player::Move(const XMFLOAT3 & vecShift)
{
	//printf("%f %f %f\n", vecShift.x, vecShift.y, vecShift.z);
	// Store previous position
	m_vecOldPos = m_vecPos;

	// Update position
	//m_vecPos += vecShift;
	XMStoreFloat3(&m_vecPos,
		XMVectorAdd(XMLoadFloat3(&m_vecPos), XMLoadFloat3(&vecShift))
	);
	
	m_pCamera->Move(vecShift);
}

void Library::BSPEngine::Player::ApplyForce(ULONG Direction, float Force, bool flyMode)
{
	XMFLOAT3 vecShift = XMFLOAT3(0, 0, 0);

	XMFLOAT3 vecLook = (flyMode) ? m_pCamera->GetLook() : m_vecLook;
	XMFLOAT3 vecRight = (flyMode) ? m_pCamera->GetRight() : m_vecRight;
 	XMFLOAT3 vecUp = (flyMode) ? m_pCamera->GetUp() : m_vecUp;

	// Which direction are we moving ?
	if (Direction & DIR_FORWARD) {
		//vecShift += m_vecLook;
		XMStoreFloat3(&vecShift,
			XMVectorAdd(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecLook)));
	}
	if (Direction & DIR_BACKWARD) {
		//vecShift -= m_vecLook;
		XMStoreFloat3(&vecShift,
			XMVectorSubtract(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecLook)));
	}
	if (Direction & DIR_RIGHT) {
		//vecShift += m_vecRight;
		XMStoreFloat3(&vecShift,
			XMVectorAdd(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecRight)));
	}
	if (Direction & DIR_LEFT) {
		//vecShift -= m_vecRight;
		XMStoreFloat3(&vecShift,
			XMVectorSubtract(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecRight)));
	}
	if (Direction & DIR_UP) {
		//vecShift += m_vecUp;
		XMStoreFloat3(&vecShift,
			XMVectorAdd(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecUp)));
	}
	if (Direction & DIR_DOWN) {
		//vecShift -= m_vecUp;
		XMStoreFloat3(&vecShift,
			XMVectorSubtract(XMLoadFloat3(&vecShift), XMLoadFloat3(&vecUp)));
	}

	//if (flyMode) {
		//m_vecAppliedForce = XMFLOAT3(0.f, 0.f, 0.f);
	//}

	// Normalize the direction vector
	//D3DXVec3Normalize(&vecShift, &vecShift);
	//m_vecAppliedForce += vecShift * Force;
	XMStoreFloat3(&m_vecAppliedForce,
		XMVectorMultiplyAdd(
			XMVector3Normalize(XMLoadFloat3(&vecShift)),
			XMVectorReplicate(Force),
			XMLoadFloat3(&m_vecAppliedForce)
		)
	);
}

void Library::BSPEngine::Player::Rotate(float x, float y, float z)
{
	// Validate requirements
	if (!m_pCamera) return;

	// Update & Clamp pitch / roll / Yaw values

	if (x)
	{
		// Make sure we don't overstep our pitch boundaries
		m_fPitch += x;
		if (m_fPitch >  89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = 89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }

	} // End if any Pitch

	if (y)
	{
		// Ensure yaw (in degrees) wraps around between 0 and 360
		m_fYaw += y;
		if (m_fYaw >  360.0f) m_fYaw -= 360.0f;
		if (m_fYaw <  0.0f) m_fYaw += 360.0f;

	} // End if any yaw

	  // Roll is purely a statistical value, no player rotation actually occurs
	if (z)
	{
		// Make sure we don't overstep our roll boundaries
		m_fRoll += z;
		if (m_fRoll >  20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = 20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }

	} // End if any roll

	// Allow camera to rotate prior to updating our axis
	m_pCamera->Rotate(x, y, z);

	// Now rotate our axis
	if (y)
	{
		// Build rotation matrix
		//D3DXMatrixRotationAxis(&mtxRotate, &m_vecUp, D3DXToRadian(y));
		CXMMATRIX mtxRotateY = XMMatrixRotationAxis(XMLoadFloat3(&m_vecUp), XMConvertToRadians(y));
		// Update our vectors
		//D3DXVec3TransformNormal(&m_vecLook, &m_vecLook, &mtxRotate);
		XMStoreFloat3(&m_vecLook, XMVector3TransformNormal(XMLoadFloat3(&m_vecLook), mtxRotateY));
		//D3DXVec3TransformNormal(&m_vecRight, &m_vecRight, &mtxRotate);
		XMStoreFloat3(&m_vecRight, XMVector3TransformNormal(XMLoadFloat3(&m_vecRight), mtxRotateY));
	} // End if any yaw

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
}

//-----------------------------------------------------------------------------
// Name : SetOnFloor ()
// Desc : Allows update callbacks to specify whether or not the player is on
//        the floor.
//-----------------------------------------------------------------------------
void Library::BSPEngine::Player::SetOnFloor(bool OnFloor)
{
	// Set whether or not we're on the floor.
	if (OnFloor)
		m_fOffFloorTime = 0.0f;
	else
		m_fOffFloorTime = 1.0f;
}

//-----------------------------------------------------------------------------
// Name : GetOnFloor ()
// Desc : Return whether or not we're on the floor
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Player::GetOnFloor() const
{
	// Only return true if we've been off the floor for > 200ms
	return (m_fOffFloorTime < 0.200);
}


void Library::BSPEngine::Player::UpdateCrouchMovement(float timeScale)
{
	//printf("%f\n", m_Volume.Max.y - m_Volume.Min.y);
	//printf("%f\n", m_Volume.Max.y - m_Volume.Min.y);
	//printf("%f\n\n", m_pCamera->GetPosition().y);

	if (m_bCrouching) {
		m_crouchMovementOn = true;
		m_Volume.Max.y -= CROUCH_VOLUME_REDUCE_Y;
		m_Volume.Min.y += CROUCH_VOLUME_REDUCE_Y;
		float delta = m_Volume.Max.y - m_Volume.Min.y;
		if (delta < m_playerHeight * CROUCH_HEIGHT_REDUCE_FACTOR)
		{
			m_Volume.Max.y =  .5f * m_playerHeight * CROUCH_HEIGHT_REDUCE_FACTOR;
			m_Volume.Min.y = -.5f * m_playerHeight * CROUCH_HEIGHT_REDUCE_FACTOR;
		}	
	}
	else {
		if (!m_crouchMovementOn) return;
		bool bUpdated = false;
		XMFLOAT3 velSave = m_vecVelocity;
		// ceil collision test ?
		m_vecVelocity.y = (m_Volume.Max.y - m_Volume.Min.y);
		if (m_bCollision) {
			// invoke scene update player for collision detection
			if (m_pScene->UpdatePlayer(this, timeScale, true)) {
				bUpdated = true;
			}
		}
		m_vecVelocity = velSave;
		if (bUpdated) return;
		m_Volume.Max.y += CROUCH_VOLUME_REDUCE_Y;
		m_Volume.Min.y -= CROUCH_VOLUME_REDUCE_Y;
		float delta = m_Volume.Max.y - m_Volume.Min.y;
		if (delta >= m_playerHeight)
		{
			m_Volume.Max.y =  .5f * m_playerHeight;
			m_Volume.Min.y = -.5f * m_playerHeight;
		}
	}

	// update camera offset using original ratio
	m_vecCamOffset.y = m_Volume.Max.y * m_camPlayerHeightRatio;
	SetCamOffset(m_vecCamOffset);
}
