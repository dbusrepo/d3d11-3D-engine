#include "SkyboxComponent.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "CameraComponent.h"
#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "Mesh.h"
#include "Model.h"
#include "SkyboxMaterial.h"
#include <DDSTextureLoader.h>

namespace Library
{
	RTTI_DEFINITIONS(SkyboxComponent)


	SkyboxComponent::SkyboxComponent(D3DApp& app, CameraComponent& camera, const std::wstring& cubeMapFileName, float sphereRadius)
		: DrawableComponent(app, camera), mCubeMapFileName(cubeMapFileName), mRasterizerState(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mMaterial(nullptr),
		mWorldMatrix(MatrixHelper::Identity), mCubeMapShaderResourceView(nullptr), mSphereRadius(sphereRadius)
	{
	}

	SkyboxComponent::~SkyboxComponent()
	{
		ReleaseObject(mRasterizerState);
		ReleaseObject(mDepthStencilState);
		ReleaseObject(mCubeMapShaderResourceView);
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
		DeleteObject(mMaterial);
	}


	void SkyboxComponent::Initialize()
	{
		HRESULT hr;

		mMaterial = new SkyboxMaterial;
		mMaterial->Initialize(mApp->Direct3DDevice(), mApp->WindowHandle());

		Model *mSphere = Model::CreateSphere(mSphereRadius, 30, 30);
		Mesh *mesh = mSphere->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mApp->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(mApp->Direct3DDevice(), &mIndexBuffer);
		mIndexCount = (UINT)mesh->Indices().size();

		//hr = DirectX::CreateDDSTextureFromFile(mApp->Direct3DDevice(), mCubeMapFileName.c_str(), nullptr, &mCubeMapShaderResourceView);
		hr = CreateDDSTextureFromFileEx(mApp->Direct3DDevice(), mCubeMapFileName.c_str(),
			0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, nullptr, &mCubeMapShaderResourceView);


		if (FAILED(hr))
		{
			throw D3DAppException("CreateDDSTextureFromFile() failed.", hr);
		}

		D3D11_RASTERIZER_DESC rsDesc;
		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.DepthClipEnable = true;

		hr = mApp->Direct3DDevice()->CreateRasterizerState(&rsDesc, &mRasterizerState);

		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateRasterizerState() failed", hr);
		}

		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		// Create depth stencil state
		hr = mApp->Direct3DDevice()->CreateDepthStencilState(&dsDesc, &mDepthStencilState);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateDepthStencilState() failed", hr);
		}

	}

	void SkyboxComponent::Update(const Timer &timer)
	{
		const XMFLOAT3& position = mCamera->Position();
		XMStoreFloat4x4(&mWorldMatrix, XMMatrixTranslation(position.x, position.y, position.z));
	}

	void SkyboxComponent::Draw(const Timer &timer)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mApp->Direct3DDeviceContext();

		direct3DDeviceContext->RSSetState(mRasterizerState);
		direct3DDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = (UINT)mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = world * mCamera->ViewProjectionMatrix();//ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->Update(direct3DDeviceContext, wvp, mCubeMapShaderResourceView);

		mMaterial->Apply(direct3DDeviceContext);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}


}