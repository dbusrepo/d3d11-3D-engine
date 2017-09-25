#pragma once

#include "Common.h"

namespace Library
{

	class FixedColorShader
	{
		//RTTI_DECLARATIONS(FixedColorShader, Shader)

	public:
		FixedColorShader();
		~FixedColorShader();

		void Initialize(ID3D11Device* device, HWND hwnd);
		void Apply(ID3D11DeviceContext* deviceContext);
		void SetParameters(ID3D11DeviceContext* deviceContext, XMMATRIX wvp);
		void SetParameters(ID3D11DeviceContext* deviceContext, XMVECTOR color);

		//void CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements, ID3D11InputLayout **inputLayout);
		//void Apply(UINT flags, ID3D11DeviceContext* context);

	private:
		void InitializeShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&);
		void Shutdown();

		struct MatrixBufferType
		{
			XMFLOAT4X4 wvp;
		};

		struct ColorBufferType
		{
			XMFLOAT4 color;
		};

		FixedColorShader(const FixedColorShader& rhs);
		FixedColorShader& operator=(const FixedColorShader& rhs);

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mMatrixBuffer;
		ID3D11Buffer* mColorBuffer;

	};
}