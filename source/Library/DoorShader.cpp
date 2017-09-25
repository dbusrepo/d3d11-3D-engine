#include "D3DAppException.h"
#include "Utility.h"
#include "DoorShader.h"
#include "D3Dcompiler.h"

namespace Library {

	namespace BSPEngine {

		DoorShader::DoorShader()
			: mName("DoorShader")
		{
			mShaderLightVertexShader = nullptr;
			mShaderLightPixelShader = nullptr;
			mShaderNoLightVertexShader = nullptr;
			mShaderNoLightPixelShader = nullptr;
			mShaderLightInputLayout = nullptr;
			mShaderNoLightInputLayout = nullptr;
			mCBufferVS = nullptr;
			mCBufferPS = nullptr;
			mTexSampler = nullptr;
			mLmSampler = nullptr;
			mUseLighting = false;

		}

		DoorShader::~DoorShader()
		{
			ReleaseObject(mCBufferVS);
			ReleaseObject(mCBufferPS);
			ReleaseObject(mShaderLightInputLayout);
			ReleaseObject(mShaderNoLightInputLayout);
			ReleaseObject(mShaderLightVertexShader);
			ReleaseObject(mShaderLightPixelShader);
			ReleaseObject(mShaderNoLightVertexShader);
			ReleaseObject(mShaderNoLightPixelShader);
			ReleaseObject(mTexSampler);
			ReleaseObject(mLmSampler);
		}

		void DoorShader::Initialize(ID3D11Device *device, HWND hwnd)
		{
			SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

			InitializeShader(device, hwnd);
		}

		void DoorShader::Apply(ID3D11DeviceContext* deviceContext)
		{
			if (mUseLighting) {
				deviceContext->IASetInputLayout(mShaderLightInputLayout);
				deviceContext->VSSetShader(mShaderLightVertexShader, nullptr, 0);
				deviceContext->PSSetShader(mShaderLightPixelShader, nullptr, 0);
				deviceContext->PSSetSamplers(0, 1, &mTexSampler);
				deviceContext->PSSetSamplers(1, 1, &mLmSampler);
			}
			else {
				deviceContext->IASetInputLayout(mShaderNoLightInputLayout);
				deviceContext->VSSetShader(mShaderNoLightVertexShader, nullptr, 0);
				deviceContext->PSSetShader(mShaderNoLightPixelShader, nullptr, 0);
				deviceContext->PSSetSamplers(0, 1, &mTexSampler);
			}
		}


		void DoorShader::InitializeShader(ID3D11Device *device, HWND hwnd)
		{
			HRESULT hr;
			std::vector<char> compiledVS;
			std::vector<char> compiledPS;

			/**************************************************************************/
			/*
			Utility::LoadBinaryFile(L"Content\\Shaders\\DoorLightVS.cso", compiledVS);

			// Create the vertex shader from the buffer.
			hr = device->CreateVertexShader(&compiledVS[0], compiledVS.size(), nullptr, &mShaderLightVertexShader);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateVertexShader() failed", hr);
			}

			Utility::LoadBinaryFile(L"Content\\Shaders\\DoorLightPS.cso", compiledPS);

			// Create the pixel shader from the buffer.
			hr = device->CreatePixelShader(&compiledPS[0], compiledPS.size(), nullptr, &mShaderLightPixelShader);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreatePixelShader() failed", hr);
			}
			*/
			/**************************************************************************/

			// shader light input layout definition
			/*
			D3D11_INPUT_ELEMENT_DESC inputLayoutLightShader[6];

			// Now setup the layout of the data that goes into the shader.
			// This setup needs to match the VertexType structure in the buffer.
			inputLayoutLightShader[0].SemanticName = "POSITION";
			inputLayoutLightShader[0].SemanticIndex = 0;
			inputLayoutLightShader[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputLayoutLightShader[0].InputSlot = 0;
			inputLayoutLightShader[0].AlignedByteOffset = 0;
			inputLayoutLightShader[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[0].InstanceDataStepRate = 0;

			inputLayoutLightShader[1].SemanticName = "TEXCOORD";
			inputLayoutLightShader[1].SemanticIndex = 0;
			inputLayoutLightShader[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputLayoutLightShader[1].InputSlot = 0;
			inputLayoutLightShader[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutLightShader[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[1].InstanceDataStepRate = 0;

			inputLayoutLightShader[2].SemanticName = "TEXCOORD";
			inputLayoutLightShader[2].SemanticIndex = 1;
			inputLayoutLightShader[2].Format = DXGI_FORMAT_R32G32_FLOAT;
			inputLayoutLightShader[2].InputSlot = 0;
			inputLayoutLightShader[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutLightShader[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[2].InstanceDataStepRate = 0;

			inputLayoutLightShader[3].SemanticName = "NORMAL";
			inputLayoutLightShader[3].SemanticIndex = 0;
			inputLayoutLightShader[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputLayoutLightShader[3].InputSlot = 0;
			inputLayoutLightShader[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutLightShader[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[3].InstanceDataStepRate = 0;

			inputLayoutLightShader[4].SemanticName = "TANGENT";
			inputLayoutLightShader[4].SemanticIndex = 0;
			inputLayoutLightShader[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			inputLayoutLightShader[4].InputSlot = 0;
			inputLayoutLightShader[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutLightShader[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[4].InstanceDataStepRate = 0;

			inputLayoutLightShader[5].SemanticName = "TEXCOORD";
			inputLayoutLightShader[5].SemanticIndex = 2;
			inputLayoutLightShader[5].Format = DXGI_FORMAT_R32_UINT;
			inputLayoutLightShader[5].InputSlot = 0;
			inputLayoutLightShader[5].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutLightShader[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutLightShader[5].InstanceDataStepRate = 0;

			hr = device->CreateInputLayout(inputLayoutLightShader, ARRAYSIZE(inputLayoutLightShader), &compiledVS[0], compiledVS.size(), &mShaderLightInputLayout);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateinputLayoutLightShader() failed.", hr);
			}
			*/

			/**************************************************************************/

			compiledVS.clear();
			compiledPS.clear();

			Utility::LoadBinaryFile(L"Content\\Shaders\\DoorNoLightVS.cso", compiledVS);
			// Create the vertex shader from the buffer.
			hr = device->CreateVertexShader(&compiledVS[0], compiledVS.size(), nullptr, &mShaderNoLightVertexShader);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateVertexShader() failed", hr);
			}

			Utility::LoadBinaryFile(L"Content\\Shaders\\DoorNoLightPS.cso", compiledPS);
			// Create the pixel shader from the buffer.
			hr = device->CreatePixelShader(&compiledPS[0], compiledPS.size(), nullptr, &mShaderNoLightPixelShader);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreatePixelShader() failed", hr);
			}

			/**************************************************************************/

			D3D11_INPUT_ELEMENT_DESC inputLayoutNoLightShader[2];

			inputLayoutNoLightShader[0].SemanticName = "POSITION";
			inputLayoutNoLightShader[0].SemanticIndex = 0;
			inputLayoutNoLightShader[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputLayoutNoLightShader[0].InputSlot = 0;
			inputLayoutNoLightShader[0].AlignedByteOffset = 0;
			inputLayoutNoLightShader[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutNoLightShader[0].InstanceDataStepRate = 0;

			inputLayoutNoLightShader[1].SemanticName = "TEXCOORD";
			inputLayoutNoLightShader[1].SemanticIndex = 0;
			inputLayoutNoLightShader[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputLayoutNoLightShader[1].InputSlot = 0;
			inputLayoutNoLightShader[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputLayoutNoLightShader[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputLayoutNoLightShader[1].InstanceDataStepRate = 0;

			hr = device->CreateInputLayout(inputLayoutNoLightShader, ARRAYSIZE(inputLayoutNoLightShader), &compiledVS[0], compiledVS.size(), &mShaderNoLightInputLayout);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateinputLayoutLightShader() failed.", hr);
			}

			/**************************************************************************/

			// Setup the description of the dynamic constant buffer that is in the vertex shader.
			D3D11_BUFFER_DESC cbufferDescVS;
			cbufferDescVS.Usage = D3D11_USAGE_DYNAMIC;
			cbufferDescVS.ByteWidth = sizeof(glDataVShader);
			cbufferDescVS.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbufferDescVS.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbufferDescVS.MiscFlags = 0;
			cbufferDescVS.StructureByteStride = 0;

			// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
			hr = device->CreateBuffer(&cbufferDescVS, nullptr, &mCBufferVS);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateBuffer() failed.", hr);
			}

			/**************************************************************************/

			D3D11_SAMPLER_DESC samplerDesc;

			// Create a texture sampler state description.
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 4;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; //D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0.0f;
			samplerDesc.BorderColor[1] = 0.0f;
			samplerDesc.BorderColor[2] = 0.0f;
			samplerDesc.BorderColor[3] = 0.0f;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = device->CreateSamplerState(&samplerDesc, &mTexSampler);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateSamplerState() failed.", hr);
			}

			/**************************************************************************/

			D3D11_SAMPLER_DESC lmSamplerDesc;
			// Create a lightmap sampler state description.
			lmSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;//D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			lmSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			lmSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			lmSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			lmSamplerDesc.MipLODBias = 0.0f;
			lmSamplerDesc.MaxAnisotropy = 1;
			lmSamplerDesc.ComparisonFunc = (D3D11_COMPARISON_FUNC)0;// D3D11_COMPARISON_NEVER; //D3D11_COMPARISON_ALWAYS;
			lmSamplerDesc.BorderColor[0] = 0.0f;
			lmSamplerDesc.BorderColor[1] = 0.0f;
			lmSamplerDesc.BorderColor[2] = 0.0f;
			lmSamplerDesc.BorderColor[3] = 0.0f;
			lmSamplerDesc.MinLOD = 0;
			lmSamplerDesc.MaxLOD = 0;

			hr = device->CreateSamplerState(&lmSamplerDesc, &mLmSampler);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11Device::CreateSamplerState() failed.", hr);
			}

		}


		void DoorShader::SetCBufferVS(ID3D11DeviceContext *deviceContext, XMMATRIX wvp)
		{
			HRESULT hr;
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			// Lock the constant buffer so it can be written to.
			hr = deviceContext->Map(mCBufferVS, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11DeviceContext::Map() failed.", hr);
			}

			glDataVShader* dataPtr;

			// Get a pointer to the data in the constant buffer.
			dataPtr = (glDataVShader*)mappedResource.pData;

			// Copy the matrix into the constant buffer.
			XMStoreFloat4x4(&dataPtr->wvp, XMMatrixTranspose(wvp));

			// Unlock the constant buffer.
			deviceContext->Unmap(mCBufferVS, 0);

			// Finally set the constant buffer in the vertex shader with the updated values.
			deviceContext->VSSetConstantBuffers(0, 1, &mCBufferVS);

		}

		void DoorShader::SetCBufferPS(ID3D11DeviceContext * deviceContext, XMFLOAT3 worldCamPos, XMFLOAT2 clusterMapSize)
		{
			if (!mUseLighting) return;

			HRESULT hr;
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			// Lock the constant buffer so it can be written to.
			hr = deviceContext->Map(mCBufferPS, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				throw D3DAppException("ID3D11DeviceContext::Map() failed.", hr);
			}

			glDataPShader* dataPtr;

			// Get a pointer to the data in the constant buffer.
			dataPtr = (glDataPShader*)mappedResource.pData;
			dataPtr->worldCameraPos = worldCamPos;
			dataPtr->clusterMapSize = clusterMapSize;

			// Unlock the constant buffer.
			deviceContext->Unmap(mCBufferPS, 0);

			// Finally set the constant buffer in the pixel shader with the updated values.
			deviceContext->PSSetConstantBuffers(0, 1, &mCBufferPS);

		}

		void DoorShader::SetLightmapClusterMapsTextureArrayView(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * lightMapTexArrayView, ID3D11ShaderResourceView * clusterMapTexArrayView)
		{
			if (!mUseLighting) return;
			// Set shader texture resource in the pixel shader.
			deviceContext->PSSetShaderResources(0, 1, &lightMapTexArrayView);
			deviceContext->PSSetShaderResources(1, 1, &clusterMapTexArrayView);
		}

		void DoorShader::SetUseLighting(bool useLighting)
		{
			mUseLighting = useLighting;
		}

		void DoorShader::SetDiffuseAndNormalTextureArrayViews(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * textureArrView, ID3D11ShaderResourceView* textureArrNormalView)
		{
			// Set shader texture resource in the pixel shader.
			if (mUseLighting) {
				deviceContext->PSSetShaderResources(2, 1, &textureArrView);
				deviceContext->PSSetShaderResources(3, 1, &textureArrNormalView);
			}
			else {
				deviceContext->PSSetShaderResources(0, 1, &textureArrView);
			}
		}

	}
}