#include "KeyboardComponent.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "Timer.h"

namespace Library
{
	RTTI_DEFINITIONS(KeyboardComponent)

		KeyboardComponent::KeyboardComponent(D3DApp& app, LPDIRECTINPUT8 directInput)
		: Component(app), mDirectInput(directInput), mDevice(nullptr)
	{
		assert(mDirectInput != nullptr);
		ZeroMemory(mCurrentState, sizeof(mCurrentState));
		ZeroMemory(mLastState, sizeof(mLastState));
	}

	KeyboardComponent::~KeyboardComponent()
	{
		if (mDevice != nullptr)
		{
			mDevice->Unacquire();
			mDevice->Release();
			mDevice = nullptr;
		}
	}

	const byte* const KeyboardComponent::CurrentState() const
	{
		return mCurrentState;
	}

	const byte* const KeyboardComponent::LastState() const
	{
		return mLastState;
	}

	void KeyboardComponent::Initialize()
	{
		if (FAILED(mDirectInput->CreateDevice(GUID_SysKeyboard, &mDevice, nullptr)))
		{
			throw D3DAppException("IDIRECTINPUT8::CreateDevice() failed");
		}

		if (FAILED(mDevice->SetDataFormat(&c_dfDIKeyboard)))
		{
			throw D3DAppException("IDIRECTINPUTDEVICE8::SetDataFormat() failed");
		}

		if (FAILED(mDevice->SetCooperativeLevel(mApp->WindowHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		{
			throw D3DAppException("IDIRECTINPUTDEVICE8::SetCooperativeLevel() failed");
		}

		mDevice->Acquire();

	}

	void KeyboardComponent::Update(const Timer& appTimer)
	{
		if (mDevice != nullptr)
		{
			memcpy(mLastState, mCurrentState, sizeof(mCurrentState));

			if (FAILED(mDevice->GetDeviceState(sizeof(mCurrentState), (LPVOID)mCurrentState)))
			{
				// Try to reacquire the device
				if (SUCCEEDED(mDevice->Acquire()))
				{
					mDevice->GetDeviceState(sizeof(mCurrentState), (LPVOID)mCurrentState);
				}
				// else ...if the device can't be reacquired warn the user...
			}
		}
	}

	bool KeyboardComponent::IsKeyUp(byte key) const
	{
		return ((mCurrentState[key] & 0x80) == 0);
	}

	bool KeyboardComponent::IsKeyDown(byte key) const
	{
		return ((mCurrentState[key] & 0x80) != 0);
	}

	bool KeyboardComponent::WasKeyUp(byte key) const
	{
		return ((mLastState[key] & 0x80) == 0);
	}

	bool KeyboardComponent::WasKeyDown(byte key) const
	{
		return ((mLastState[key] & 0x80) != 0);
	}

	bool KeyboardComponent::WasKeyPressedThisFrame(byte key) const
	{
		return (IsKeyDown(key) && WasKeyUp(key));
	}

	bool KeyboardComponent::WasKeyReleasedThisFrame(byte key) const
	{
		return (IsKeyUp(key) && WasKeyDown(key));
	}

	bool KeyboardComponent::IsKeyHeldDown(byte key) const
	{
		return (IsKeyDown(key) && WasKeyDown(key));
	}
}