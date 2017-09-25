#include "D3DAppException.h"
#include "Utility.h"
#include "ColorShader.h"
#include "D3Dcompiler.h"

namespace Library {
	

	ColorShader::ColorShader()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mMatrixBuffer = nullptr;
	}

	ColorShader::~ColorShader()
	{
		Shutdown();
	}

	void ColorShader::Initialize(ID3D11Device *device, HWND hwnd)
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize the vertex and pixel shaders.
		InitializeShader(device, hwnd, L"Content\\Shaders\\ColorVS.cso", L"Content\\Shaders\\ColorPS.cso");
	}

	void ColorShader::Apply(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(mInputLayout);
		deviceContext->VSSetShader(mVertexShader, nullptr, 0);
		deviceContext->PSSetShader(mPixelShader, nullptr, 0);
	}


	void ColorShader::InitializeShader(ID3D11Device *device, HWND hwnd, const std::wstring &fileNameVS, const std::wstring &fileNamePS)
	{
		HRESULT hr;

		std::vector<char> compiledVS;
		Utility::LoadBinaryFile(fileNameVS, compiledVS);

		//ID3D10Blob* compiledShader = nullptr;

		//hr = D3DReadFileToBlob(fileNameVS.c_str(), &compiledShader);

		/*
		to compile shader see
		https://msdn.microsoft.com/it-it/library/windows/desktop/hh968107(v=vs.85).aspx

		ID3D10Blob* compiledShader = nullptr;
		ID3D10Blob* errorMessages = nullptr;
		hr = D3DCompileFromFile(L"Content\\Shaders\\ColorVS.hlsl",
			nullptr, nullptr, "main",
			"vs_5_0", 0, 0,
			&compiledShader, &errorMessages);

		if (FAILED(hr))
		{
			throw D3DAppException("D3DCompileFromFile failed", hr);
		}
		*/

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

		inputLayout[1].SemanticName = "COLOR";
		inputLayout[1].SemanticIndex = 0;
		inputLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

	}


	void ColorShader::SetParameters(ID3D11DeviceContext *deviceContext, XMMATRIX wvp)
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

		// Unlock the constant buffer.
		deviceContext->Unmap(mMatrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		UINT bufferSlotNumber = 0;

		// Finally set the constant buffer in the vertex shader with the updated values.
		deviceContext->VSSetConstantBuffers(bufferSlotNumber, 1, &mMatrixBuffer);

	}



	void ColorShader::Shutdown()
	{
		ReleaseObject(mMatrixBuffer);
		ReleaseObject(mInputLayout);
		ReleaseObject(mVertexShader);
		ReleaseObject(mPixelShader);
	}

}