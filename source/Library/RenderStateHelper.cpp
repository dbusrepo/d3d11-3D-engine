#include "RenderStateHelper.h"
#include "D3DApp.h"

namespace Library
{
	RenderStateHelper::RenderStateHelper(D3DApp& app)
		: mApp(app), mRasterizerState(nullptr), mBlendState(nullptr), mBlendFactor(new FLOAT[4]), mSampleMask(UINT_MAX), mDepthStencilState(nullptr), mStencilRef(UINT_MAX)
	{
	}

	RenderStateHelper::~RenderStateHelper()
	{
		ReleaseObject(mRasterizerState);
		ReleaseObject(mBlendState);
		ReleaseObject(mDepthStencilState);
		DeleteObjects(mBlendFactor);
	}

	void RenderStateHelper::ResetAll(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->RSSetState(nullptr);
		deviceContext->OMSetBlendState(nullptr, nullptr, UINT_MAX);
		deviceContext->OMSetDepthStencilState(nullptr, UINT_MAX);
	}

	void RenderStateHelper::SaveRasterizerState()
	{
		ReleaseObject(mRasterizerState);
		mApp.Direct3DDeviceContext()->RSGetState(&mRasterizerState);
	}

	void RenderStateHelper::RestoreRasterizerState() const
	{
		mApp.Direct3DDeviceContext()->RSSetState(mRasterizerState);
	}

	void RenderStateHelper::SaveBlendState()
	{
		ReleaseObject(mBlendState);
		mApp.Direct3DDeviceContext()->OMGetBlendState(&mBlendState, mBlendFactor, &mSampleMask);
	}

	void RenderStateHelper::RestoreBlendState() const
	{
		mApp.Direct3DDeviceContext()->OMSetBlendState(mBlendState, mBlendFactor, mSampleMask);
	}

	void RenderStateHelper::SaveDepthStencilState()
	{
		ReleaseObject(mDepthStencilState);
		mApp.Direct3DDeviceContext()->OMGetDepthStencilState(&mDepthStencilState, &mStencilRef);
	}

	void RenderStateHelper::RestoreDepthStencilState() const
	{
		mApp.Direct3DDeviceContext()->OMSetDepthStencilState(mDepthStencilState, mStencilRef);
	}

	void RenderStateHelper::SaveAll()
	{
		SaveRasterizerState();
		SaveBlendState();
		SaveDepthStencilState();
	}

	void RenderStateHelper::RestoreAll() const
	{
		RestoreRasterizerState();
		RestoreBlendState();
		RestoreDepthStencilState();
	}
}
