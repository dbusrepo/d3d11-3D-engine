#include "FirstPersonCameraComponent.h"
#include "D3DApp.h"
#include "Timer.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "VectorHelper.h"

#include <iostream>
using namespace std;

namespace Library
{
	RTTI_DEFINITIONS(FirstPersonCameraComponent)

	const float FirstPersonCameraComponent::DefaultRotationRate = XMConvertToRadians(0.1f);
	const float FirstPersonCameraComponent::DefaultMovementRate = 300.0f;
	const float FirstPersonCameraComponent::DefaultMouseSensitivity = 80.0f;
	
	FirstPersonCameraComponent::FirstPersonCameraComponent(D3DApp& app)
		: CameraComponent(app), mKeyboard(nullptr), mMouse(nullptr),
		mMouseSensitivity(DefaultMouseSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate)
	{
		mViewDirty = true;
		mViewProjDirty = true;
		mFrustumDirty = true;
	}

	FirstPersonCameraComponent::~FirstPersonCameraComponent()
	{
		mKeyboard = nullptr;
		mMouse = nullptr;
	}

	const KeyboardComponent& FirstPersonCameraComponent::GetKeyboard() const
	{
		return *mKeyboard;
	}

	void FirstPersonCameraComponent::SetKeyboard(KeyboardComponent& keyboard)
	{
		mKeyboard = &keyboard;
	}

	const MouseComponent& FirstPersonCameraComponent::GetMouse() const
	{
		return *mMouse;
	}

	void FirstPersonCameraComponent::SetMouse(MouseComponent& mouse)
	{
		mMouse = &mouse;
	}

	float&FirstPersonCameraComponent::MouseSensitivity()
	{
		return mMouseSensitivity;
	}


	float& FirstPersonCameraComponent::RotationRate()
	{
		return mRotationRate;
	}

	float& FirstPersonCameraComponent::MovementRate()
	{
		return mMovementRate;
	}

	void FirstPersonCameraComponent::SetLens(float fovY, float aspect, float zn, float zf)
	{
		CameraComponent::SetLens(fovY, aspect, zn, zf);
		mViewProjDirty = true;
	}

	void FirstPersonCameraComponent::Initialize()
	{
		mKeyboard = (KeyboardComponent*)mApp->Services().GetService(KeyboardComponent::TypeIdClass());
		mMouse = (MouseComponent*)mApp->Services().GetService(MouseComponent::TypeIdClass());

#ifdef MOUSE_FILTERING
		ZeroMemory(mouseHistory, sizeof(XMFLOAT2) * MOUSE_HISTORY_FILTER_BUFFER_SIZE);
#endif
		CameraComponent::Initialize();
	}

	void FirstPersonCameraComponent::Update(const Timer &timer)
	{
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_W))
			{
				movementAmount.z = 1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_S))
			{
				movementAmount.z = -1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_A))
			{
				movementAmount.x = -1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_D))
			{
				movementAmount.x = 1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_Q))
			{
				movementAmount.y = 1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_Z))
			{
				movementAmount.y = -1.0f;
			}
		}

		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (mMouse != nullptr) //if ((mMouse != nullptr) && (mMouse->IsButtonHeldDown(MouseButtonsLeft)))
		{
			LPDIMOUSESTATE mouseState = mMouse->CurrentState();
			rotationAmount.x = mouseState->lX * mMouseSensitivity;
			rotationAmount.y = mouseState->lY * mMouseSensitivity;

		}

		mApp->CenterMousePos();

		float elapsedTime = (float)timer.DeltaTime();
		// rotate

		XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount) * mRotationRate * elapsedTime;

		if (!XMVector3Equal(rotationVector, XMLoadFloat3(&Vector3Helper::Zero)))
		{
			XMMATRIX pitchMatrix = XMMatrixRotationAxis(XMLoadFloat3(&mRight), XMVectorGetY(rotationVector));
			XMMATRIX yawMatrix = XMMatrixRotationY(XMVectorGetX(rotationVector));

			ApplyRotation(XMMatrixMultiply(pitchMatrix, yawMatrix));
			mViewDirty = true;
		}

		// translate
		XMVECTOR translationVector = XMVectorMultiply(XMLoadFloat3(&movementAmount), XMVectorReplicate(mMovementRate * elapsedTime));

		if (!XMVector3Equal(translationVector, XMLoadFloat3(&Vector3Helper::Zero)))
		{
			XMStoreFloat3(&mPosition,
				XMVectorMultiplyAdd(XMVectorReplicate(XMVectorGetZ(translationVector)), XMLoadFloat3(&mDirection),
					XMVectorMultiplyAdd(XMVectorReplicate(XMVectorGetX(translationVector)), XMLoadFloat3(&mRight),
						XMVectorMultiplyAdd(XMVectorReplicate(XMVectorGetY(translationVector)), XMLoadFloat3(&Vector3Helper::Up), XMLoadFloat3(&mPosition)))));
			mViewDirty = true;
		}

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


#ifdef MOUSE_FILTERING
	void FirstPersonCameraComponent::filterMouseMoves(float * dx, float * dy)
	{
		memmove(&mouseHistory[1], mouseHistory, (MOUSE_HISTORY_FILTER_BUFFER_SIZE - 1) * sizeof(XMFLOAT2));

		mouseHistory[0] = { *dx, *dy };
		float avgX = 0.0f, avgY = 0.0f, avgTot = 0.0f, currentWeight = 1.0f;

		for (int i = 0; i != MOUSE_HISTORY_FILTER_BUFFER_SIZE; ++i) {
			avgX += mouseHistory[i].x * currentWeight;
			avgY += mouseHistory[i].y * currentWeight;
			avgTot += currentWeight;
			currentWeight *= MOUSE_FILTER_WEIGHT;
		}

		*dx = avgX / avgTot;
		*dy = avgY / avgTot;
	}
#endif
}
