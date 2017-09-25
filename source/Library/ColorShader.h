#pragma once

#include "Common.h"

namespace Library
{

	class ColorShader
	{
		//RTTI_DECLARATIONS(ColorShader, Shader)

	public:
		ColorShader();
		~ColorShader();

		void Initialize(ID3D11Device* device, HWND hwnd);
		void Apply(ID3D11DeviceContext* deviceContext);
		void SetParameters(ID3D11DeviceContext* deviceContext, XMMATRIX wvp);
		
		//void CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements, ID3D11InputLayout **inputLayout);
		//void Apply(UINT flags, ID3D11DeviceContext* context);

	private:
		void InitializeShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&);
		void Shutdown();

		struct MatrixBufferType
		{
			XMFLOAT4X4 wvp;
		};

		ColorShader(const ColorShader& rhs);
		ColorShader& operator=(const ColorShader& rhs);

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mMatrixBuffer;
				
	};
}