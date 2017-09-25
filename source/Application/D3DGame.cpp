#include "D3DGame.h"
#include "D3DAppException.h"
#include <sstream>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "ColorHelper.h"
#include "FpsComponent.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "FirstPersonCameraComponent.h"
#include "CpuUsageComponent.h"
#include "GridComponent.h"
#include "GridTextureComponent.h"
#include "RenderStateHelper.h"
#include "Utility.h"
#include "SkyboxComponent.h"
#include "Scene.h"
#include "Player.h"
#include "Camera.h"

namespace Application {

	const XMVECTORF32 D3DGame::backgroundColor = Colors::Black;

	D3DGame::D3DGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: D3DApp(instance, windowClass, windowTitle, showCommand),
		mFps(nullptr),
		mCpuUsage(nullptr),
		mDirectInput(nullptr),
		mKeyboard(nullptr),
		mMouse(nullptr),
		mRenderStateHelper(nullptr),
		mSpriteBatch(nullptr),
		mSpriteFont(nullptr),
		m_bVsyncEnabled(false),
		mDepthStencilState(nullptr),
		mScene(nullptr),
		m_pPlayer(nullptr)
	{
		mDepthStencilBufferEnabled = true;
		mMultiSamplingEnabled = true;
		mIsFullScreen = false;
		m_bVsyncEnabled = false;
		// m_bPVSEnabled, m_bFrustumEnabled, ...
		m_bFlyMode = true;
	}

	D3DGame::~D3DGame()
	{
		Shutdown();
	}

	void D3DGame::Initialize()
	{
		if (FAILED(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr)))
		{
			throw D3DAppException("DirectInput8Create() failed");
		}
		
		mKeyboard = new KeyboardComponent(*this, mDirectInput);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard);

		mMouse = new MouseComponent(*this, mDirectInput);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse);

		mFps = new FpsComponent(*this, ColorHelper::Yellow);

		mCpuUsage = new CpuUsageComponent(*this, ColorHelper::Yellow);

		D3DApp::Initialize(); // init windows and directx and registered components

		mFps->Initialize();
		mFps->TextPosition().y = 30;
		mCpuUsage->Initialize();
		mCpuUsage->TextPosition().y = 30+32;
		
		initDepthStencilState();
		mRenderStateHelper = new RenderStateHelper(*this);

		SetupGameState();

		///*
		//SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
		mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_11_Regular.spritefont");
		//*/
	}

	void D3DGame::OnResize()
	{
		D3DApp::OnResize();
		if (m_pPlayer) {
			m_pPlayer->GetCamera()->SetLens(m_fovY, AspectRatio(), m_nearPlane, m_farPlane);
		}
	}

	void D3DGame::Update(const Timer& appTimer)
	{
		//static float acc = 0.f;
		float dt = appTimer.DeltaTime(); //1.f / 60;
		//printf("%f\n", dt);
		//float deltaTime = appTimer.DeltaTime();
		//acc += deltaTime;

		//int nUpdates = 0;
		//while (acc >= dt && nUpdates != 10)
		//{
			//printf("%f\n", dt);
			//acc -= dt;
			//++nUpdates;

			// update all registered components (mouse, keyboard)
			D3DApp::Update(appTimer);

			if (mKeyboard && mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
			{
				Exit(); // ?...
			}

			ProcessInput();

			// Update the player and camera (now separated from ProcessInput because collision detection
			// against the animating objects (which may be triggered by input) has to be performed after
			// those animating objects have been updated / moved).
			// Update our camera (updates velocity etc)
			if (m_pPlayer) m_pPlayer->Update(dt, m_bFlyMode);

			// Animate the scene objects
			//m_Scene.AnimateObjects(m_Timer);

			if (mScene) mScene->Update(dt);

			if (mFps) mFps->Update(appTimer);
			if (mCpuUsage) mCpuUsage->Update(appTimer);

		//}
	}

	void D3DGame::ProcessInput()
	{
		ULONG        Direction = 0;
		float        X = 0.0f, Y = 0.0f;

		if (!mKeyboard || !mMouse || !m_pPlayer) return;
		//goto upd;

		if (mKeyboard->IsKeyDown(DIK_W) || mKeyboard->IsKeyDown(DIK_UPARROW))
			Direction |= DIRECTION::DIR_FORWARD;
		if (mKeyboard->IsKeyDown(DIK_S) || mKeyboard->IsKeyDown(DIK_DOWNARROW))
			Direction |= DIRECTION::DIR_BACKWARD;
		if (mKeyboard->IsKeyDown(DIK_A) || mKeyboard->IsKeyDown(DIK_LEFTARROW))
			Direction |= DIRECTION::DIR_LEFT;
		if (mKeyboard->IsKeyDown(DIK_D) || mKeyboard->IsKeyDown(DIK_RIGHTARROW))
			Direction |= DIRECTION::DIR_RIGHT;
		if (mKeyboard->IsKeyDown(DIK_Q))
			Direction |= DIRECTION::DIR_UP;
		if (mKeyboard->IsKeyDown(DIK_Z))
			Direction |= DIRECTION::DIR_DOWN;

		LPDIMOUSESTATE mouseState = mMouse->CurrentState();
		X = mouseState->lX / 10.0f;// *mMouseSensitivity;
		Y = mouseState->lY / 10.0f;// *mMouseSensitivity;

		CenterMousePos();

		// Update if we have moved
		if (Direction > 0 || X != 0.0f || Y != 0.0f)
		{
			// Rotate our camera
			if (X || Y)
			{
				m_pPlayer->Rotate(Y, X, 0.0f);
			} // End if any rotation

			  // Any Movement ?
			if (Direction)
			{
				// Apply a force to the player.
				float fForce;

				if (!m_bFlyMode) {
					fForce = 1300.0f;
					if (mKeyboard->IsKeyDown(DIK_LSHIFT)) {
						fForce += 1000.0f;
					}
				}
				else {
					fForce = 3000.f;
					if (mKeyboard->IsKeyDown(DIK_LSHIFT)) {
						fForce += 3000.0f;
					}
				}
			//upd:
			//	fForce = 30000.f;//.008f*30000.f;
			//	Direction = DIRECTION::DIR_RIGHT;
				m_pPlayer->ApplyForce(Direction, fForce, m_bFlyMode);

			} // End if any movement

		} // End if camera moved

		// Jump pressed?
		if (!m_bFlyMode && m_pPlayer->GetOnFloor())
		{
						
			if (!m_pPlayer->IsCrouching() &&
					mKeyboard->IsKeyDown(DIK_SPACE))
			{
					XMFLOAT3 Velocity = m_pPlayer->GetVelocity();
					Velocity.y += 250.f;// 240.0f;
					//Velocity.y += 800.0f;
					m_pPlayer->SetVelocity(Velocity);
					m_pPlayer->SetOnFloor(false);
			}
			
			m_pPlayer->SetCrouching(mKeyboard->IsKeyDown(DIK_LCONTROL));

		} // End if Jumping Key

	}

	void D3DGame::Draw(const Timer& appTimer)
	{
		assert(mDirect3DDeviceContext);
		assert(mSwapChain);

		BeginScene();

		Direct3DDeviceContext()->OMSetDepthStencilState(mDepthStencilState, 1);

			//D3DApp::Draw(appTimer); // draw all drawable components

		mRenderStateHelper->SaveAll();

		if (mScene) mScene->Draw(appTimer);

			//mRenderStateHelper->RestoreAll();

			//mSkybox->Draw(appTimer);
			//mRenderStateHelper->RestoreAll();

			//mRenderStateHelper->SaveAll();
		if (mFps) mFps->Draw(mTimer);

			//mRenderStateHelper->RestoreAll();

			//mRenderStateHelper->SaveAll();
			//mCpuUsage->Draw(mTimer);
			//mRenderStateHelper->RestoreAll();

			//drawPlayerInfo();

			// mouse xy-position
			/*
			mSpriteBatch->Begin();
			std::wostringstream mouseLabel;
			mouseLabel << L"Mouse Position: " << mMouse->X() << ", " << mMouse->Y() << "  Mouse Wheel: " << mMouse->Wheel();
			mSpriteFont->DrawString(mSpriteBatch, mouseLabel.str().c_str(), mMouseTextPosition);
			mSpriteBatch->End();
			*/
		mRenderStateHelper->RestoreAll();

		EndScene();

		
	}

	void D3DGame::SetupGameState()
	{
		mScene = new Scene(this);
		mScene->Initialize();
		m_pPlayer = mScene->GetPlayer();

		// Set up initial configuration options
		m_fovY = XMConvertToRadians(70); //XM_PIDIV4;
		m_nearPlane = 1.01f;
		m_farPlane = 13000.0f;
		m_pPlayer->GetCamera()->SetLens(m_fovY, AspectRatio(), m_nearPlane, m_farPlane);
		
		// Collision detection on by default
		m_bNoClip = false;

		// Culling modes on by default
		m_bPVSEnabled = true;
		m_bFrustumEnabled = true;
		// -> notify bsp tree/scene
		m_bDebugDraw = false;
	}


	void D3DGame::BeginScene()
	{
		//mDirect3DDeviceContext->ClearState();
		//mDirect3DDeviceContext->Flush();
		///*
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView,
			reinterpret_cast<const float*>(&backgroundColor));
		//*/
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
	}

	void D3DGame::EndScene()
	{
		HRESULT hr;

		// Present the back buffer to the screen since rendering is complete.
		if (m_bVsyncEnabled)
		{
			// Lock to screen refresh rate.
			hr = mSwapChain->Present(1, 0);
		}
		else
		{
			// Present as fast as possible.
			hr = mSwapChain->Present(0, 0);
		}
	
		if (FAILED(hr)) {
			throw D3DAppException("IDXGISwapChain::Present() failed.", hr);
		}

	}

	void D3DGame::Shutdown()
	{
		DeleteObject(mScene);
		m_pPlayer = nullptr;

		DeleteObject(mKeyboard);
		DeleteObject(mMouse);
		DeleteObject(mFps);
		DeleteObject(mCpuUsage);
		DeleteObject(mRenderStateHelper);

		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);

		ReleaseObject(mDirectInput);
		ReleaseObject(mDepthStencilState);


		D3DApp::Shutdown();
	}

	void D3DGame::initDepthStencilState()
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS; //D3D11_COMPARISON_GREATER;

		// Create depth stencil state
		HRESULT hr = Direct3DDevice()->CreateDepthStencilState(&dsDesc, &mDepthStencilState);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateDepthStencilState() failed", hr);
		}

	}

	void D3DGame::drawPlayerInfo()
	{
		mPlayerInfoTextPosition = XMFLOAT2(0, 100);

		mSpriteBatch->Begin();
		std::wostringstream posLabel;
		std::wostringstream forwardLabel, upLabel, rightLabel;

		const XMFLOAT3& position = mScene->GetPlayer()->GetCamera()->GetPosition();
		
		posLabel.precision(4);
		posLabel << position.x << ", " << position.y << ", " << position.z;
		mSpriteFont->DrawString(mSpriteBatch, posLabel.str().c_str(), mPlayerInfoTextPosition);
		
		//const XMFLOAT3& position = mCamera->
		//forwardLabel.precision(4);
		//forwardLabel << position.x << ", " << position.y << ", " << position.z;
		//mSpriteFont->DrawString(mSpriteBatch, forwardLabel.str().c_str(), mPlayerInfoTextPosition);

		
		mSpriteBatch->End();
	}

}
