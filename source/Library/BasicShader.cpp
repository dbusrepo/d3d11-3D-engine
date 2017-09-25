#include "D3DAppException.h"
#include "Utility.h"
#include "BasicShader.h"
#include "D3Dcompiler.h"

Library::BSPEngine::BasicShader::BasicShader()
	: mName("BasicShader")
{
	mVertexShader = nullptr;
	mPixelShader = nullptr;
	mInputLayout = nullptr;
	mConstantBuffer = nullptr;
}

Library::BSPEngine::BasicShader::~BasicShader()
{
	ReleaseObject(mConstantBuffer);
	ReleaseObject(mInputLayout);
	ReleaseObject(mVertexShader);
	ReleaseObject(mPixelShader);
}

void Library::BSPEngine::BasicShader::Initialize(ID3D11Device * device, HWND hwnd)
{
	SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

	InitializeShader(device, hwnd, L"Content\\Shaders\\BasicVS.cso", L"Content\\Shaders\\BasicPS.cso");
}

void Library::BSPEngine::BasicShader::Apply(ID3D11DeviceContext * deviceContext)
{
	deviceContext->IASetInputLayout(mInputLayout);
	deviceContext->VSSetShader(mVertexShader, nullptr, 0);
	deviceContext->PSSetShader(mPixelShader, nullptr, 0);
}

void Library::BSPEngine::BasicShader::SetParameters(ID3D11DeviceContext * deviceContext, XMMATRIX wvp, XMVECTOR color)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	hr = deviceContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11DeviceContext::Map() failed.", hr);
	}

	ConstantBufferType* dataPtr;

	// Get a pointer to the data in the constant buffer.
	dataPtr = (ConstantBufferType*)mappedResource.pData;

	// Copy the matrix into the constant buffer.
	XMStoreFloat4x4(&dataPtr->wvp, XMMatrixTranspose(wvp));
	XMStoreFloat4(&dataPtr->color, color);

	// Unlock the constant buffer.
	deviceContext->Unmap(mConstantBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	UINT bufferSlotNumber = 0;

	// Finally set the constant buffer in the shaders with the updated values.
	deviceContext->VSSetConstantBuffers(bufferSlotNumber, 1, &mConstantBuffer);
	deviceContext->PSSetConstantBuffers(bufferSlotNumber, 1, &mConstantBuffer);
}

void Library::BSPEngine::BasicShader::InitializeShader(ID3D11Device *device, HWND hwnd, const std::wstring &fileNameVS, const std::wstring &fileNamePS)
{
	HRESULT hr;

	std::vector<char> compiledVS;
	Utility::LoadBinaryFile(fileNameVS, compiledVS);

	// Create the vertex shader from the buffer.
	hr = device->CreateVertexShader(&compiledVS[0], compiledVS.size(), nullptr, &mVertexShader);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateVertexShader() failed", hr);
	}

	std::vector<char> compiledPS;
	Utility::LoadBinaryFile(fileNamePS, compiledPS);

	// Create the pixel shader from the buffer.
	hr = device->CreatePixelShader(&compiledPS[0], compiledPS.size(), nullptr, &mPixelShader);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreatePixelShader() failed", hr);
	}

	D3D11_INPUT_ELEMENT_DESC inputLayout[1];

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType structure in the ModelClass and in the shader.
	inputLayout[0].SemanticName = "POSITION";
	inputLayout[0].SemanticIndex = 0;
	inputLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayout[0].InputSlot = 0;
	inputLayout[0].AlignedByteOffset = 0;
	inputLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[0].InstanceDataStepRate = 0;

	hr = device->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout), &compiledVS[0], compiledVS.size(), &mInputLayout);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateInputLayout() failed.", hr);
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = device->CreateBuffer(&constantBufferDesc, nullptr, &mConstantBuffer);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateBuffer() failed.", hr);
	}
}
