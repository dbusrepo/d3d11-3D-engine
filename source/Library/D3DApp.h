#pragma once

#include "Common.h"
#include "Timer.h"
#include "Component.h"
#include "ServiceContainer.h"



namespace Library
{
	class D3DApp
	{
	public:
		D3DApp(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
		virtual ~D3DApp();

		HINSTANCE Instance() const;
		HWND WindowHandle() const;
		const WNDCLASSEX& Window() const;
		const std::wstring& WindowClass() const;
		const std::wstring& WindowTitle() const;
		int ScreenWidth() const;
		int ScreenHeight() const;

		ID3D11Device* Direct3DDevice() const;
		ID3D11DeviceContext* Direct3DDeviceContext() const;
		//bool DepthBufferEnabled() const;
		void DisableRenderTarget();
		void EnableRenderTarget();

		float AspectRatio() const;
		bool IsFullScreen() const;
		const D3D11_TEXTURE2D_DESC& BackBufferDesc() const;
		const D3D11_VIEWPORT& Viewport() const;
		void CenterMousePos();

		const std::vector<Component*>& Components() const;
		const ServiceContainer& Services() const;

		virtual void Run();
		virtual void Exit();
		virtual void Initialize();
		virtual void OnResize();
		virtual void Update(const Timer &timer);
		virtual void Draw(const Timer &timer);
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		Timer mTimer;
	protected:
		virtual void InitializeWindow();
		virtual void InitializeDirectX();
		virtual void Shutdown();

		static const UINT DefaultScreenWidth;
		static const UINT DefaultScreenHeight;
		static const UINT DefaultFrameRate;
		static const UINT DefaultMultiSamplingCount;

		HINSTANCE mInstance;
		std::wstring mWindowClass;
		std::wstring mWindowTitle;
		int mShowCommand;

		HWND mWindowHandle;
		WNDCLASSEX mWindow;
		bool      mAppPaused;
		bool      mMinimized;
		bool      mMaximized;
		bool      mResizing;

		UINT mScreenWidth;
		UINT mScreenHeight;

		
		std::vector<Component*> mComponents;
		ServiceContainer mServices;

		D3D_FEATURE_LEVEL mFeatureLevel;
		ID3D11Device1* mDirect3DDevice;
		ID3D11DeviceContext1* mDirect3DDeviceContext;
#if defined(DEBUG) || defined(_DEBUG)  
		ID3D11Debug *mDebugger;
#endif		
		IDXGISwapChain1* mSwapChain;

		UINT mFrameRate;
		bool mIsFullScreen;
		bool mDepthStencilBufferEnabled;
		bool mMultiSamplingEnabled;
		UINT mMultiSamplingCount;
		UINT mMultiSamplingQualityLevels;

		ID3D11Texture2D* mDepthStencilBuffer;
		D3D11_TEXTURE2D_DESC mBackBufferDesc;
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11DepthStencilView* mDepthStencilView;
		D3D11_VIEWPORT mViewport;

	private:
		D3DApp(const D3DApp& rhs);
		D3DApp& operator=(const D3DApp& rhs);

		
		POINT CenterWindow(int windowWidth, int windowHeight);
	};
}


