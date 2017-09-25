#pragma once

#include "Common.h"
#include "CameraComponent.h"

#define MOUSE_FILTERING
#define MOUSE_HISTORY_FILTER_BUFFER_SIZE 10
#define MOUSE_FILTER_WEIGHT 0.5f

namespace Library
{
	class KeyboardComponent;
	class MouseComponent;

	class FirstPersonCameraComponent : public CameraComponent
	{
		
		RTTI_DECLARATIONS(FirstPersonCameraComponent, CameraComponent)

	public:

		FirstPersonCameraComponent(D3DApp& app);

		virtual ~FirstPersonCameraComponent();

		const KeyboardComponent& GetKeyboard() const;
		void SetKeyboard(KeyboardComponent& keyboard);

		const MouseComponent& GetMouse() const;
		void SetMouse(MouseComponent& mouse);

		float& MouseSensitivity();
		float& RotationRate();
		float& MovementRate();

		virtual void SetLens(float fovY, float aspect, float zn, float zf);

		virtual void Initialize() override;
		virtual void Update(const Timer &timer) override;
		
		static const float DefaultMouseSensitivity;
		static const float DefaultRotationRate;
		static const float DefaultMovementRate;

	protected:
		
		KeyboardComponent* mKeyboard;
		MouseComponent* mMouse;
		
		bool mViewDirty; // View matrix dirty ?
		bool mViewProjDirty; // Proj matrix dirty ?
		bool mFrustumDirty; // Frustum planes dirty ?

		float mMouseSensitivity;
		float mRotationRate;
		float mMovementRate;

#ifdef MOUSE_FILTERING
		XMFLOAT2 mouseHistory[MOUSE_HISTORY_FILTER_BUFFER_SIZE];
		void filterMouseMoves(float *dx, float *dy);
#endif

	private:
		FirstPersonCameraComponent(const FirstPersonCameraComponent& rhs);
		FirstPersonCameraComponent& operator=(const FirstPersonCameraComponent& rhs);
	};
}

