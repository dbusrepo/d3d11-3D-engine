#include "D3DAppException.h"
#include "Utility.h"
#include "FixedColorShader.h"
#include "D3Dcompiler.h"

namespace Library {

	FixedColorShader::FixedColorShader()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mMatrixBuffer = nullptr;
		mColorBuffer = nullptr;
	}

	FixedColorShader::~FixedColorShader()
	{
		Shutdown();
	}

	void FixedColorShader::Initialize(ID3D11Device *device, HWND hwnd)
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		InitializeShader(device, hwnd, L"Content\\Shaders\\FixedColorVS.cso", L"Content\\Shaders\\FixedColorPS.cso");
	}

	void FixedColorShader::Apply(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(mInputLayout);
		deviceContext->VSSetShader(mVertexShader, nullptr, 0);
		deviceContext->PSSetShader(mPixelShader, nullptr, 0);
	}


	void FixedColorShader::InitializeShader(ID3D11Device *device, HWND hwnd, const std::wstring &fileNameVS, const std::wstring &fileNamePS)
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
		inputLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

		// Setup the description of the dynamic color constant buffer
		D3D11_BUFFER_DESC colorBufferDesc;
		colorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		colorBufferDesc.ByteWidth = sizeof(ColorBufferType);
		colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		colorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		colorBufferDesc.MiscFlags = 0;
		colorBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer
		hr = device->CreateBuffer(&colorBufferDesc, nullptr, &mColorBuffer);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11Device::CreateBuffer() failed.", hr);
		}

	}


	void FixedColorShader::SetParameters(ID3D11DeviceContext *deviceContext, XMMATRIX wvp)
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

	void FixedColorShader::SetParameters(ID3D11DeviceContext * deviceContext, XMVECTOR color)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// Lock the constant buffer so it can be written to.
		hr = deviceContext->Map(mColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			throw D3DAppException("ID3D11DeviceContext::Map() failed.", hr);
		}

		ColorBufferType* dataPtr;

		// Get a pointer to the data in the constant buffer.
		dataPtr = (ColorBufferType*)mappedResource.pData;

		// Copy the matrix into the constant buffer.
		XMStoreFloat4(&dataPtr->color, color);

		// Unlock the constant buffer.
		deviceContext->Unmap(mColorBuffer, 0);
		
		// Finally set the constant buffer in the vertex shader with the updated values.
		deviceContext->PSSetConstantBuffers(0, 1, &mColorBuffer);

	}



	void FixedColorShader::Shutdown()
	{
		ReleaseObject(mMatrixBuffer);
		ReleaseObject(mColorBuffer);
		ReleaseObject(mInputLayout);
		ReleaseObject(mVertexShader);
		ReleaseObject(mPixelShader);
	}

}