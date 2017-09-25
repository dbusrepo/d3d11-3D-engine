#pragma once

#include "Component.h"

namespace Library
{
	class GameTime;

	enum MouseButtons
	{
		MouseButtonsLeft = 0,
		MouseButtonsRight = 1,
		MouseButtonsMiddle = 2,
		MouseButtonsX1 = 3
	};

	class MouseComponent : public Component
	{
		RTTI_DECLARATIONS(MouseComponent, Component)

	public:
		MouseComponent(D3DApp& app, LPDIRECTINPUT8 directInput);
		~MouseComponent();

		LPDIMOUSESTATE CurrentState();
		LPDIMOUSESTATE LastState();

		virtual void Initialize() override;
		virtual void Update(const Timer &timer) override;

		long X() const;
		long Y() const;
		long Wheel() const;

		bool IsButtonUp(MouseButtons button) const;
		bool IsButtonDown(MouseButtons button) const;
		bool WasButtonUp(MouseButtons button) const;
		bool WasButtonDown(MouseButtons button) const;
		bool WasButtonPressedThisFrame(MouseButtons button) const;
		bool WasButtonReleasedThisFrame(MouseButtons button) const;
		bool IsButtonHeldDown(MouseButtons button) const;

	private:
		MouseComponent();

		LPDIRECTINPUT8 mDirectInput;
		LPDIRECTINPUTDEVICE8 mDevice;
		DIMOUSESTATE mCurrentState;
		DIMOUSESTATE mLastState;

		long mX;
		long mY;
		long mWheel;
	};
}