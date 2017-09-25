#pragma once

#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		class DoorShader
		{

		public:
			DoorShader();
			~DoorShader();

			void Initialize(ID3D11Device* device, HWND hwnd);
			void Apply(ID3D11DeviceContext* deviceContext);
			void SetCBufferVS(ID3D11DeviceContext* deviceContext, XMMATRIX wvp);
			void SetCBufferPS(ID3D11DeviceContext* deviceContext, XMFLOAT3 worldCamPos, XMFLOAT2 clusterMapSize);
			void SetDiffuseAndNormalTextureArrayViews(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* textureArrView, ID3D11ShaderResourceView* textureArrNormalView);
			void SetLightmapClusterMapsTextureArrayView(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* lightMapTexArrayView, ID3D11ShaderResourceView* clusterMapTexArrayView);
			void SetUseLighting(bool useLighting);
		private:
			void InitializeShader(ID3D11Device*, HWND);

			struct glDataVShader
			{
				XMFLOAT4X4 wvp;
			};

			struct glDataPShader
			{
				XMFLOAT3 worldCameraPos;
				float padding;
				XMFLOAT2 clusterMapSize;
				XMFLOAT2 padding2;
			};

			DoorShader(const DoorShader& rhs);
			DoorShader& operator=(const DoorShader& rhs);

			std::string mName;

			bool mUseLighting;

			ID3D11VertexShader* mShaderLightVertexShader;
			ID3D11PixelShader* mShaderLightPixelShader;
			ID3D11InputLayout* mShaderLightInputLayout;

			ID3D11VertexShader* mShaderNoLightVertexShader;
			ID3D11PixelShader* mShaderNoLightPixelShader;
			ID3D11InputLayout* mShaderNoLightInputLayout;
			ID3D11Buffer* mCBufferVS;
			ID3D11Buffer* mCBufferPS;

			ID3D11SamplerState* mTexSampler;
			ID3D11SamplerState* mLmSampler;
		};
	}
}