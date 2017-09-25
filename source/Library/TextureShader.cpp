#include "D3DAppException.h"
#include "Utility.h"
#include "TextureShader.h"
#include "D3Dcompiler.h"

namespace Library {

	TextureShader::TextureShader()
		: mName()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mMatrixBuffer = nullptr;
		mSamplerState = nullptr;
	}

	TextureShader::~TextureShader()
	{
		Shutdown();
	}

	void TextureShader::Initialize(ID3D11Device *device, HWND hwnd)
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		InitializeShader(device, hwnd, L"Content\\Shaders\\TextureVS.cso", L"Content\\Shaders\\TexturePS.cso");
	}

	void TextureShader::Apply(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(mInputLayout);
		deviceContext->VSSetShader(mVertexShader, nullptr, 0);
		deviceContext->PSSetShader(mPixelShader, nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, &mSamplerState);
	}


	void TextureShader::InitializeShader(ID3D11Device *device, HWND hwnd, const std::wstring &fileNameVS, const std::wstring &fileNamePS)
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

		D3D11_INPUT_ELEMENT_DESC inputLayout[2];

		// Now setup the layout of the data that goes into the shader.
		// This setup needs to match the VertexType structure in the ModelClass and in the shader.
		inputLayout[0].SemanticName = "POSITION";
		inputLayout[0].SemanticIndex = 0;
		inputLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputLayout[0].InputSlot = 0;
		inputLayout[0].AlignedByteOffset = 0;
		inputLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayout[0].InstanceDataStepRate = 0;

		inputLayout[1].SemanticName = "TEXCOORD";
		inputLayout[1].SemanticIndex = 0;
		inputLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputLayout[1].InputSlot = 0;
		inputLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayout[1].InstanceDataStepRate = 0;

		hr = device->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout), &compiledVS[0], compiledVS.size(), &mInputLayout);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateInputLayout() failed.", hr);
		}

		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		hr = device->CreateBuffer(&matrixBufferDesc, nullptr, &mMatrixBuffer);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateBuffer() failed.", hr);
		}

		D3D11_SAMPLER_DESC samplerDesc;

		// Create a texture sampler state description.
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; //D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = device->CreateSamplerState(&samplerDesc, &mSamplerState);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateSamplerState() failed.", hr);
		}

	}


	void TextureShader::SetParameters(ID3D11DeviceContext *deviceContext, XMMATRIX wvp, XMMATRIX texMat, ID3D11ShaderResourceView* textureView)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// Lock the constant buffer so it can be written to.
		hr = deviceContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11DeviceContext::Map() failed.", hr);
		}

		MatrixBufferType* dataPtr;

		// Get a pointer to the data in the constant buffer.
		dataPtr = (MatrixBufferType*)mappedResource.pData;

		// Copy the matrix into the constant buffer.
		XMStoreFloat4x4(&dataPtr->wvp, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&dataPtr->texMat, XMMatrixTranspose(texMat));

		// Unlock the constant buffer.
		deviceContext->Unmap(mMatrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		UINT bufferSlotNumber = 0;

		// Finally set the constant buffer in the vertex shader with the updated values.
		deviceContext->VSSetConstantBuffers(bufferSlotNumber, 1, &mMatrixBuffer);

		// Set shader texture resource in the pixel shader.
		deviceContext->PSSetShaderResources(0, 1, &textureView);

	}


	void TextureShader::Shutdown()
	{
		ReleaseObject(mMatrixBuffer);
		ReleaseObject(mInputLayout);
		ReleaseObject(mVertexShader);
		ReleaseObject(mPixelShader);
		ReleaseObject(mSamplerState);
	}

}