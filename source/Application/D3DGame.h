#pragma once

#include "Common.h"
#include "D3DApp.h"

using namespace Library;
using namespace Library::BSPEngine;

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Library
{
	class FpsComponent;
	class KeyboardComponent;
	class MouseComponent;
	class FirstPersonCameraComponent;
	class CpuUsageComponent;
	class RenderStateHelper;

	namespace BSPEngine {
		class Scene;
		class Player;
	}
}



namespace Application {

	class D3DGame : public D3DApp
	{
	public:
		D3DGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
		~D3DGame();

		void Initialize() override;
		void OnResize() override;
		void Update(const Timer& appTimer) override;
		void Draw(const Timer& appTimer) override;

	protected:
		void SetupGameState();
		void ProcessInput();
		void BeginScene();
		void EndScene();
		void Shutdown() override;
		void initDepthStencilState();

	private:

		static const XMVECTORF32 backgroundColor;

		void drawPlayerInfo();

		LPDIRECTINPUT8 mDirectInput;
		FpsComponent* mFps; 
		CpuUsageComponent* mCpuUsage;
		KeyboardComponent* mKeyboard;
		MouseComponent* mMouse;

		//FirstPersonCameraComponent* mCamera;
		RenderStateHelper* mRenderStateHelper;

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mPlayerInfoTextPosition;

		ID3D11DepthStencilState* mDepthStencilState;

		float m_fovY;
		float m_nearPlane;
		float m_farPlane;

		bool m_bVsyncEnabled;
		bool m_bNoClip;          // Collision detection on / off.
		bool m_bDebugDraw;       // Should we be drawing debug information?
		bool m_bPVSEnabled;      // Is PVS culling enabled in this application?
		bool m_bFrustumEnabled;  // Is Frustum culling enabled in this application?
		bool m_bFlyMode;

		Scene *mScene;
		Player *m_pPlayer;
	};

}