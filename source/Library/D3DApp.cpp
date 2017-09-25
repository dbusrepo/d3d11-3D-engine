#include "D3DApp.h"
#include "D3DAppException.h"
#include "DrawableComponent.h"

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	Library::D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

namespace Library
{
	const UINT D3DApp::DefaultScreenWidth = 1366;//WINDOW_WIDTH;
	const UINT D3DApp::DefaultScreenHeight = 768;//WINDOW_HEIGHT;
	const UINT D3DApp::DefaultFrameRate = 60;
	const UINT D3DApp::DefaultMultiSamplingCount = 4;

	D3DApp::D3DApp(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: mInstance(instance), mWindowClass(windowClass), mWindowTitle(windowTitle), mShowCommand(showCommand),
		mWindowHandle(), mWindow(),
		mScreenWidth(DefaultScreenWidth),
		mScreenHeight(DefaultScreenHeight),
		mFeatureLevel(D3D_FEATURE_LEVEL_9_1),
		mDirect3DDevice(nullptr),
		mDirect3DDeviceContext(nullptr),
		mSwapChain(nullptr),
		mFrameRate(DefaultFrameRate),
		mIsFullScreen(false),
		mDepthStencilBufferEnabled(false),
		mMultiSamplingEnabled(false),
		mMultiSamplingCount(DefaultMultiSamplingCount),
		mMultiSamplingQualityLevels(0),
		mDepthStencilBuffer(nullptr),
		mRenderTargetView(nullptr),
		mDepthStencilView(nullptr),
		mAppPaused(false),
		mMinimized(false),
		mMaximized(false),
		mResizing(false),
		mViewport()
	{
		// Get a pointer to the application object so we can forward 
		// Windows messages to the object's window procedure through
		// the global window procedure.
		gd3dApp = this;
	}

	D3DApp::~D3DApp()
	{

	}

	HINSTANCE D3DApp::Instance() const
	{
		return mInstance;
	}

	HWND D3DApp::WindowHandle() const
	{
		return mWindowHandle;
	}

	const WNDCLASSEX& D3DApp::Window() const
	{
		return mWindow;
	}

	const std::wstring& D3DApp::WindowClass() const
	{
		return mWindowClass;
	}

	const std::wstring& D3DApp::WindowTitle() const
	{
		return mWindowTitle;
	}

	int D3DApp::ScreenWidth() const
	{
		return mScreenWidth;
	}

	int D3DApp::ScreenHeight() const
	{
		return mScreenHeight;
	}

	ID3D11Device* D3DApp::Direct3DDevice() const
	{
		return mDirect3DDevice;
	}

	ID3D11DeviceContext* D3DApp::Direct3DDeviceContext() const
	{
		return mDirect3DDeviceContext;
	}

	void D3DApp::DisableRenderTarget()
	{
		mDirect3DDeviceContext->OMSetRenderTargets(0, nullptr, mDepthStencilView);
	}

	void D3DApp::EnableRenderTarget()
	{
		mDirect3DDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	}

	float D3DApp::AspectRatio() const
	{
		return static_cast<float>(mScreenWidth) / mScreenHeight;
	}

	bool D3DApp::IsFullScreen() const
	{
		return mIsFullScreen;
	}

	const D3D11_TEXTURE2D_DESC& D3DApp::BackBufferDesc() const
	{
		return mBackBufferDesc;
	}

	const D3D11_VIEWPORT& D3DApp::Viewport() const
	{
		return mViewport;
	}

	const std::vector<Component*>& D3DApp::Components() const
	{
		return mComponents;
	}

	const ServiceContainer& D3DApp::Services() const
	{
		return mServices;
	}

	void D3DApp::Run()
	{
		MSG msg = { 0 };

		mTimer.Reset();

		while (msg.message != WM_QUIT)
		{
			// If there are Window messages then process them.
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Otherwise, do animation/game stuff.
			else
			{				
				mTimer.Tick();
				if (!mAppPaused)
				{
					//CalculateFrameStats();

					Update(mTimer); //mTimer.DeltaTime());
					Draw(mTimer);
				}
				else
				{
					//Sleep(100);
				}
			}
		}

		Shutdown();
	}

	void D3DApp::Exit()
	{
		PostQuitMessage(0);
	}

	void D3DApp::Initialize()
	{
		InitializeWindow();
		InitializeDirectX();

		for (Component* component : mComponents)
		{
			component->Initialize();
		}
		
	}

	void D3DApp::Shutdown()
	{
		ReleaseObject(mRenderTargetView);
		ReleaseObject(mDepthStencilView);
		ReleaseObject(mSwapChain);
		ReleaseObject(mDepthStencilBuffer);

		if (mDirect3DDeviceContext != nullptr)
		{
			mDirect3DDeviceContext->ClearState();
		}

		ReleaseObject(mDirect3DDeviceContext);
#if defined(DEBUG) || defined(_DEBUG)  
		ReleaseObject(mDebugger);
#endif
		ReleaseObject(mDirect3DDevice);

		UnregisterClass(mWindowClass.c_str(), mWindow.hInstance);
	}

	void D3DApp::Update(const Timer &timer)
	{
		for (Component* component : mComponents)
		{
			component->Update(timer);
		}
	}

	void D3DApp::Draw(const Timer &timer)
	{
		for (Component* component : mComponents)
		{
			DrawableComponent* drawableGameComponent = component->As<DrawableComponent>();
			if (drawableGameComponent != nullptr && drawableGameComponent->Visible())
			{
				drawableGameComponent->Draw(timer);
			}
		}
	}

	void D3DApp::InitializeWindow()
	{
		ZeroMemory(&mWindow, sizeof(mWindow));
		mWindow.cbSize = sizeof(WNDCLASSEX);
		mWindow.style = CS_CLASSDC;
		mWindow.lpfnWndProc = MainWndProc;
		mWindow.hInstance = mInstance;
		mWindow.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		mWindow.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		mWindow.hCursor = LoadCursor(nullptr, IDC_ARROW);
		mWindow.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
		mWindow.lpszClassName = mWindowClass.c_str();

		RECT windowRectangle = { 0, 0, (LONG)mScreenWidth, (LONG)mScreenHeight };
		AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

		RegisterClassEx(&mWindow);
		POINT center = CenterWindow(mScreenWidth, mScreenHeight);
		mWindowHandle = CreateWindow(mWindowClass.c_str(), mWindowTitle.c_str(), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, center.x, center.y, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, nullptr, nullptr, mInstance, nullptr);

		ShowWindow(mWindowHandle, mShowCommand);
		UpdateWindow(mWindowHandle);
		CenterMousePos();
		//SetCursorPos(center.x + mScreenWidth/2, center.y + mScreenHeight/2);
		ShowCursor(false);
	}

	void D3DApp::InitializeDirectX()
	{
		HRESULT hr;
		UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS
#endif

		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		ID3D11Device* direct3DDevice = nullptr;
		ID3D11DeviceContext* direct3DDeviceContext = nullptr;
		if (FAILED(hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &direct3DDevice, &mFeatureLevel, &direct3DDeviceContext)))
		{
throw D3DAppException("D3D11CreateDevice() failed", hr);
		}

		if (FAILED(hr = direct3DDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&mDirect3DDevice))))
		{
			throw D3DAppException("ID3D11Device::QueryInterface() failed", hr);
		}

		if (FAILED(hr = direct3DDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&mDirect3DDeviceContext))))
		{
			throw D3DAppException("ID3D11Device::QueryInterface() failed", hr);
		}
		
#if defined(DEBUG) || defined(_DEBUG)  
		if (FAILED(hr = direct3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&mDebugger))))
		{
			throw D3DAppException("ID3D11Device::QueryInterface() failed", hr);
		}
#endif
		ReleaseObject(direct3DDevice);
		ReleaseObject(direct3DDeviceContext);

		mDirect3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, mMultiSamplingCount, &mMultiSamplingQualityLevels);
		if (mMultiSamplingQualityLevels == 0)
		{
			throw D3DAppException("Unsupported multi-sampling quality");
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.Width = mScreenWidth;
		swapChainDesc.Height = mScreenHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	
		if (mMultiSamplingEnabled)
		{
			swapChainDesc.SampleDesc.Count = mMultiSamplingCount;
			swapChainDesc.SampleDesc.Quality = mMultiSamplingQualityLevels - 1;
		}
		else
		{
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
		}

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGIDevice* dxgiDevice = nullptr;
		if (FAILED(hr = mDirect3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice))))
		{
			throw D3DAppException("ID3D11Device::QueryInterface() failed", hr);
		}

		IDXGIAdapter *dxgiAdapter = nullptr;
		if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter))))
		{
			ReleaseObject(dxgiDevice);
			throw D3DAppException("IDXGIDevice::GetParent() failed retrieving adapter.", hr);
		}

		IDXGIFactory2* dxgiFactory = nullptr;
		if (FAILED(hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory))))
		{
			ReleaseObject(dxgiDevice);
			ReleaseObject(dxgiAdapter);
			throw D3DAppException("IDXGIAdapter::GetParent() failed retrieving factory.", hr);
		}

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
		ZeroMemory(&fullScreenDesc, sizeof(fullScreenDesc));
		fullScreenDesc.RefreshRate.Numerator = 0; //mFrameRate;
		fullScreenDesc.RefreshRate.Denominator = 1;
		fullScreenDesc.Windowed = !mIsFullScreen;
		
		if (FAILED(hr = dxgiFactory->CreateSwapChainForHwnd(dxgiDevice, mWindowHandle, &swapChainDesc, &fullScreenDesc, nullptr, &mSwapChain)))
		{
			ReleaseObject(dxgiDevice);
			ReleaseObject(dxgiAdapter);
			ReleaseObject(dxgiFactory);
			throw D3DAppException("IDXGIDevice::CreateSwapChainForHwnd() failed.", hr);
		}

		ReleaseObject(dxgiDevice);
		ReleaseObject(dxgiAdapter);
		ReleaseObject(dxgiFactory);

		// The remaining steps that need to be carried out for d3d creation
		// also need to be executed every time the window is resized.  So
		// just call the OnResize method here to avoid code duplication.

		OnResize();
	}

	void D3DApp::OnResize()
	{
		HRESULT hr;

		assert(mDirect3DDeviceContext);
		assert(mDirect3DDevice);
		assert(mSwapChain);

		// Release the old views, as they hold references to the buffers we
		// will be destroying.  Also release the old depth/stencil buffer.

		ReleaseObject(mRenderTargetView);
		ReleaseObject(mDepthStencilView);
		ReleaseObject(mDepthStencilBuffer);
		
		if (FAILED(hr = mSwapChain->ResizeBuffers(1, mScreenWidth, mScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
		{
			throw D3DAppException("IDXGISwapChain::ResizeBuffers() failed.", hr);
		}

		ID3D11Texture2D* backBuffer;
		if (FAILED(hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
		{
			throw D3DAppException("IDXGISwapChain::GetBuffer() failed.", hr);
		}

		backBuffer->GetDesc(&mBackBufferDesc);

		if (FAILED(hr = mDirect3DDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView)))
		{
			ReleaseObject(backBuffer);
			throw D3DAppException("IDXGIDevice::CreateRenderTargetView() failed.", hr);
		}

		ReleaseObject(backBuffer);

		if (mDepthStencilBufferEnabled)
		{
			D3D11_TEXTURE2D_DESC depthStencilDesc;
			ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
			depthStencilDesc.Width = mScreenWidth;
			depthStencilDesc.Height = mScreenHeight;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DXGI_FORMAT_D16_UNORM //DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

			if (mMultiSamplingEnabled)
			{
				depthStencilDesc.SampleDesc.Count = mMultiSamplingCount;
				depthStencilDesc.SampleDesc.Quality = mMultiSamplingQualityLevels - 1;
			}
			else
			{
				depthStencilDesc.SampleDesc.Count = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
			}

			if (FAILED(hr = mDirect3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, &mDepthStencilBuffer)))
			{
				throw D3DAppException("IDXGIDevice::CreateTexture2D() for Depth Stencil Buffer failed.", hr);
			}

			if (FAILED(hr = mDirect3DDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr, &mDepthStencilView)))
			{
				throw D3DAppException("IDXGIDevice::CreateDepthStencilView() failed.", hr);
			}
		}

		mDirect3DDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
		
		mViewport.TopLeftX = 0.0f;
		mViewport.TopLeftY = 0.0f;
		mViewport.Width = static_cast<float>(mScreenWidth);
		mViewport.Height = static_cast<float>(mScreenHeight);
		mViewport.MinDepth = 0.0f;
		mViewport.MaxDepth = 1.0f;
		
		mDirect3DDeviceContext->RSSetViewports(1, &mViewport);

	}
	
	POINT D3DApp::CenterWindow(int windowWidth, int windowHeight)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		POINT center;
		center.x = (screenWidth - windowWidth) / 2;
		center.y = (screenHeight - windowHeight) / 2;

		return center;
	}

	void D3DApp::CenterMousePos()
	{
		RECT R;
		GetWindowRect(mWindowHandle, &R);
		POINT windowCenterScreen = { (R.right - R.left) / 2, (R.bottom - R.top) / 2 };
		ClientToScreen(mWindowHandle, &windowCenterScreen);
		SetCursorPos(windowCenterScreen.x, windowCenterScreen.y); // initial mouse position
	}

	LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			// WM_ACTIVATE is sent when the window is activated or deactivated.  
			// We pause the game when the window is deactivated and unpause it 
			// when it becomes active.  
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;

			// WM_SIZE is sent when the user resizes the window.  
		case WM_SIZE:
			// Save the new client area dimensions.
			mScreenWidth = LOWORD(lParam);
			mScreenHeight = HIWORD(lParam);
			if (mDirect3DDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					OnResize();
				}
				else if (wParam == SIZE_RESTORED)
				{

					// Restoring from minimized state?
					if (mMinimized)
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}

					// Restoring from maximized state?
					else if (mMaximized)
					{
						mAppPaused = false;
						mMaximized = false;
						OnResize();
					}
					else if (mResizing)
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					{
						OnResize();
					}
				}
			}
			return 0;

			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			mAppPaused = true;
			mResizing = true;
			mTimer.Stop();
			return 0;

			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing = false;
			mTimer.Start();
			OnResize();
			return 0;

			// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}