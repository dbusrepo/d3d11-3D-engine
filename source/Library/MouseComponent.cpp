#include "MouseComponent.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "Timer.h"

namespace Library
{
	RTTI_DEFINITIONS(MouseComponent)

		MouseComponent::MouseComponent(D3DApp& app, LPDIRECTINPUT8 directInput)
		: Component(app), mDirectInput(directInput), mDevice(nullptr), mX(0), mY(0), mWheel(0)
	{
		assert(mDirectInput != nullptr);
		ZeroMemory(&mCurrentState, sizeof(mCurrentState));
		ZeroMemory(&mLastState, sizeof(mLastState));
	}

	MouseComponent::~MouseComponent()
	{
		if (mDevice != nullptr)
		{
			mDevice->Unacquire();
			mDevice->Release();
			mDevice = nullptr;
		}
	}

	LPDIMOUSESTATE MouseComponent::CurrentState()
	{
		return &mCurrentState;
	}

	LPDIMOUSESTATE MouseComponent::LastState()
	{
		return &mLastState;
	}

	long MouseComponent::X() const
	{
		return mX;
	}

	long MouseComponent::Y() const
	{
		return mY;
	}

	long MouseComponent::Wheel() const
	{
		return mWheel;
	}

	void MouseComponent::Initialize()
	{
		if (FAILED(mDirectInput->CreateDevice(GUID_SysMouse, &mDevice, nullptr)))
		{
			throw D3DAppException("IDIRECTINPUT8::CreateDevice() failed");
		}

		if (FAILED(mDevice->SetDataFormat(&c_dfDIMouse)))
		{
			throw D3DAppException("IDIRECTINPUTDEVICE8::SetDataFormat() failed");
		}

		if (FAILED(mDevice->SetCooperativeLevel(mApp->WindowHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		{
			throw D3DAppException("IDIRECTINPUTDEVICE8::SetCooperativeLevel() failed");
		}

		mDevice->Acquire();
	}

	void MouseComponent::Update(const Timer& appTimer)
	{
		if (mDevice != nullptr)
		{
			memcpy(&mLastState, &mCurrentState, sizeof(mCurrentState));

			if (FAILED(mDevice->GetDeviceState(sizeof(mCurrentState), &mCurrentState)))
			{
				// Try to reacquire the device
				if (SUCCEEDED(mDevice->Acquire()))
				{
					if (FAILED(mDevice->GetDeviceState(sizeof(mCurrentState), &mCurrentState)))
					{
						return;
					}
				}
			}

			// Accumulate positions
			mX += mCurrentState.lX;
			mY += mCurrentState.lY;
			mWheel += mCurrentState.lZ;
		}
	}

	bool MouseComponent::IsButtonUp(MouseButtons button) const
	{
		return ((mCurrentState.rgbButtons[button] & 0x80) == 0);
	}

	bool MouseComponent::IsButtonDown(MouseButtons button) const
	{
		return ((mCurrentState.rgbButtons[button] & 0x80) != 0);
	}

	bool MouseComponent::WasButtonUp(MouseButtons button) const
	{
		return ((mLastState.rgbButtons[button] & 0x80) == 0);
	}

	bool MouseComponent::WasButtonDown(MouseButtons button) const
	{
		return ((mLastState.rgbButtons[button] & 0x80) != 0);
	}

	bool MouseComponent::WasButtonPressedThisFrame(MouseButtons button) const
	{
		return (IsButtonDown(button) && WasButtonUp(button));
	}

	bool MouseComponent::WasButtonReleasedThisFrame(MouseButtons button) const
	{
		return (IsButtonUp(button) && WasButtonDown(button));
	}

	bool MouseComponent::IsButtonHeldDown(MouseButtons button) const
	{
		return (IsButtonDown(button) && WasButtonDown(button));
	}
}