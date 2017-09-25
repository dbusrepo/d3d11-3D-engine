#include "GridComponent.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "CameraComponent.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "Utility.h"
#include "ColorHelper.h"
#include "BasicMaterialFixedColor.h"
#include "Mesh.h"
#include "Model.h"

namespace Library
{
	RTTI_DEFINITIONS(GridComponent)

	const UINT GridComponent::DefaultWidth = 64;
	const UINT GridComponent::DefaultDepth = 64;
	const UINT GridComponent::DefaultNumRows = 32;
	const UINT GridComponent::DefaultNumCols = 32;
	
	const XMFLOAT4 GridComponent::DefaultColor = XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Wheat));

	GridComponent::GridComponent(D3DApp& app, CameraComponent& camera)
		: DrawableComponent(app, camera), mGrid(nullptr), mWireframeRS(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mMaterial(nullptr),
		mPosition(Vector3Helper::Zero), mWidth(DefaultWidth), mDepth(DefaultDepth), mRows(DefaultNumRows), mCols(DefaultNumCols), mColor(DefaultColor), mWorldMatrix(MatrixHelper::Identity)
	{
	}

	GridComponent::GridComponent(D3DApp& app, CameraComponent& camera, UINT size, UINT scale, XMFLOAT4 color)
		: DrawableComponent(app, camera), mGrid(nullptr), mWireframeRS(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mMaterial(nullptr),
		mPosition(Vector3Helper::Zero), mWidth(DefaultWidth), mDepth(DefaultDepth), mRows(DefaultNumRows), mCols(DefaultNumCols), mColor(color), mWorldMatrix(MatrixHelper::Identity)
	{
	}

	GridComponent::~GridComponent()
	{
		ReleaseObject(mWireframeRS);
		ReleaseObject(mIndexBuffer);
		ReleaseObject(mVertexBuffer);
		DeleteObject(mGrid);
		DeleteObject(mMaterial);
	}

	const XMFLOAT3& GridComponent::Position() const
	{
		return mPosition;
	}

	const XMFLOAT4& GridComponent::Color() const
	{
		return mColor;
	}



	void GridComponent::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;

		XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, translation);
	}

	void GridComponent::SetPosition(float x, float y, float z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;

		XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, translation);
	}

	void GridComponent::SetColor(const XMFLOAT4& color)
	{
		mColor = color;
	}

	void GridComponent::Initialize()
	{
		InitializeGrid();

		mMaterial = new BasicMaterialFixedColor;
		mMaterial->Initialize(mApp->Direct3DDevice(), mApp->WindowHandle());

		Mesh* mesh = mGrid->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mApp->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(mApp->Direct3DDevice(), &mIndexBuffer);
		mIndexCount = (UINT)mesh->Indices().size();

		D3D11_RASTERIZER_DESC wireframeDesc;
		ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
		wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireframeDesc.CullMode = D3D11_CULL_NONE;
		wireframeDesc.FrontCounterClockwise = false;
		wireframeDesc.DepthClipEnable = true;

		HRESULT hr = mApp->Direct3DDevice()->CreateRasterizerState(&wireframeDesc, &mWireframeRS);

		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateRasterizerState() failed", hr);
		}
	}

	void GridComponent::Draw(const Timer &timer)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mApp->Direct3DDeviceContext();
		
		direct3DDeviceContext->RSSetState(mWireframeRS);
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		UINT stride = (UINT)mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = world * mCamera->ViewProjectionMatrix();//mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->UpdateMVP(direct3DDeviceContext, wvp);
		mMaterial->UpdateColor(direct3DDeviceContext, XMLoadFloat4(&mColor));

		mMaterial->Apply(direct3DDeviceContext);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void GridComponent::InitializeGrid()
	{
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
		if (mGrid) delete mGrid;
		
		ID3D11Device* direct3DDevice = GetGame()->Direct3DDevice();

		// create the model
		mGrid = Model::CreateGrid((float)mWidth, (float)mDepth, mRows, mCols);
	}
}