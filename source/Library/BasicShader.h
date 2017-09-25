#pragma once

#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		class BasicShader
		{

		public:
			BasicShader();
			~BasicShader();

			void Initialize(ID3D11Device* device, HWND hwnd);
			void Apply(ID3D11DeviceContext* deviceContext);
			void SetParameters(ID3D11DeviceContext* deviceContext, XMMATRIX wvp, XMVECTOR color);

		private:
			void InitializeShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&);
			
			struct ConstantBufferType
			{
				XMFLOAT4X4 wvp;		
				XMFLOAT4 color;
			};

			BasicShader(const BasicShader& rhs);
			BasicShader& operator=(const BasicShader& rhs);

			std::string mName;

			ID3D11VertexShader* mVertexShader;
			ID3D11PixelShader* mPixelShader;
			ID3D11InputLayout* mInputLayout;
			ID3D11Buffer* mConstantBuffer;

		};
	}
}
