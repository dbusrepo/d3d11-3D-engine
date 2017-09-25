#pragma once

#include "Common.h"

namespace Library
{

	class SkyboxShader
	{
		//RTTI_DECLARATIONS(SkyboxShader, Shader)
	public:
		SkyboxShader();
		~SkyboxShader();

		void Initialize(ID3D11Device* device, HWND hwnd);
		void Apply(ID3D11DeviceContext* deviceContext);
		void SetParameters(ID3D11DeviceContext* deviceContext,
			XMMATRIX wvp,
			ID3D11ShaderResourceView* textureView);


	private:
		void InitializeShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&);
		void Shutdown();

		struct MatrixBufferType
		{
			XMFLOAT4X4 wvp;
		};

		SkyboxShader(const SkyboxShader& rhs);
		SkyboxShader& operator=(const SkyboxShader& rhs);

		std::string mName;

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mMatrixBuffer;
		ID3D11SamplerState* mSamplerState;
	};
}