#include "GridTextureComponent.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "CameraComponent.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "Utility.h"
#include "BasicMaterialTexture.h"
#include "Mesh.h"
#include "Model.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

namespace Library
{
	RTTI_DEFINITIONS(GridTextureComponent)

	const UINT GridTextureComponent::DefaultWidth = 64;
	const UINT GridTextureComponent::DefaultDepth = 64;
	const UINT GridTextureComponent::DefaultNumRows = 32;
	const UINT GridTextureComponent::DefaultNumCols = 32;

	GridTextureComponent::GridTextureComponent(D3DApp& app, CameraComponent& camera)
		: DrawableComponent(app, camera), mRasterizerState(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mMaterial(nullptr), mTextureView(nullptr),
		mPosition(Vector3Helper::Zero), mWidth(DefaultWidth), mDepth(DefaultDepth), mRows(DefaultNumRows), mCols(DefaultNumCols), mWorldMatrix(MatrixHelper::Identity), mTexMatrix(MatrixHelper::Identity)
	{
	}

	GridTextureComponent::GridTextureComponent(D3DApp& app, CameraComponent& camera, UINT size, UINT scale, XMFLOAT4 color)
		: DrawableComponent(app, camera),mRasterizerState(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mMaterial(nullptr), mTextureView(nullptr),
		mPosition(Vector3Helper::Zero), mWidth(DefaultWidth), mDepth(DefaultDepth), mRows(DefaultNumRows), mCols(DefaultNumCols), mWorldMatrix(MatrixHelper::Identity), mTexMatrix(MatrixHelper::Identity)
	{
	}

	GridTextureComponent::~GridTextureComponent()
	{
		ReleaseObject(mRasterizerState);
		ReleaseObject(mIndexBuffer);
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mTextureView);
		DeleteObject(mMaterial);
	}

	const XMFLOAT3& GridTextureComponent::Position() const
	{
		return mPosition;
	}


	void GridTextureComponent::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;

		XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, translation);
	}

	void GridTextureComponent::SetPosition(float x, float y, float z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;

		XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, translation);
	}

	void GridTextureComponent::SetTextureTransform(XMMATRIX texMat)
	{
		XMStoreFloat4x4(&mTexMatrix, texMat);
	}

	void GridTextureComponent::Initialize()
	{

		mMaterial = new BasicMaterialTexture;
		mMaterial->Initialize(mApp->Direct3DDevice(), mApp->WindowHandle());

		Model *grid = Model::CreateGrid((float)mWidth, (float)mDepth, mRows, mCols);

		Mesh* mesh = grid->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mApp->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(mApp->Direct3DDevice(), &mIndexBuffer);
		mIndexCount = (UINT)mesh->Indices().size();

		D3D11_RASTERIZER_DESC rsDesc;
		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.DepthClipEnable = true;

		HRESULT hr = mApp->Direct3DDevice()->CreateRasterizerState(&rsDesc, &mRasterizerState);

		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateRasterizerState() failed", hr);
		}

		// Load the texture
		/*
		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		if (FAILED(hr = CreateWICTextureFromFile(mApp->Direct3DDevice(), mApp->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mTextureView)))
		{
			throw D3DAppException("CreateWICTextureFromFile() failed.", hr);
		}
		*/

		///*
		//Checkerboard.dds
		std::wstring textureName = L"Content\\Textures\\Checkerboard.dds";
		//hr = CreateDDSTextureFromFile(mApp->Direct3DDevice(), textureName.c_str(), nullptr, &mTextureView);
		hr = CreateDDSTextureFromFileEx(mApp->Direct3DDevice(), textureName.c_str(),
									0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS, true, nullptr, &mTextureView);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateDDSTextureFromFile() failed.", hr);
		}
		//*/


	}

	void GridTextureComponent::Draw(const Timer &timer)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mApp->Direct3DDeviceContext();

		direct3DDeviceContext->RSSetState(mRasterizerState);
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = (UINT)mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = world * mCamera->ViewProjectionMatrix();//mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->Update(direct3DDeviceContext, wvp, XMLoadFloat4x4(&mTexMatrix), mTextureView);
	
		mMaterial->Apply(direct3DDeviceContext);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}


}