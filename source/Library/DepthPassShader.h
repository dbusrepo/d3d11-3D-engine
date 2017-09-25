#pragma once

#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		class DepthPassShader
		{

		public:
			DepthPassShader();
			~DepthPassShader();

			void Initialize(ID3D11Device* device, HWND hwnd);
			void Apply(ID3D11DeviceContext* deviceContext);
			void SetParameters(ID3D11DeviceContext* deviceContext, XMMATRIX wvp);

		private:
			void InitializeShader(ID3D11Device*, HWND, const std::wstring&);
			
			struct ConstantBufferType
			{
				XMFLOAT4X4 wvp;
			};

			DepthPassShader(const DepthPassShader& rhs);
			DepthPassShader& operator=(const DepthPassShader& rhs);

			std::string mName;

			ID3D11VertexShader* mVertexShader;
			ID3D11InputLayout* mInputLayout;
			ID3D11Buffer* mConstantBuffer;

		};
	}
}
