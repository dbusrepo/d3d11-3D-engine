#pragma once

#include "Common.h"
#include "DrawableComponent.h"

namespace Library
{
	class CameraComponent;
	class SkyboxMaterial;

	class SkyboxComponent : public DrawableComponent
	{
		RTTI_DECLARATIONS(SkyboxComponent, DrawableComponent)

	public:
		SkyboxComponent(D3DApp& app, CameraComponent& camera, const std::wstring& cubeMapFileName, float scale);
		~SkyboxComponent();

		virtual void Initialize() override;
		virtual void Draw(const Timer &timer) override;
		virtual void Update(const Timer &timer) override;

	private:
		SkyboxComponent();
		SkyboxComponent(const SkyboxComponent& rhs);
		SkyboxComponent& operator=(const SkyboxComponent& rhs);

		std::wstring mCubeMapFileName;
		float mSphereRadius;

		SkyboxMaterial* mMaterial;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11ShaderResourceView* mCubeMapShaderResourceView;
		UINT mIndexCount;
		
		ID3D11RasterizerState* mRasterizerState;
		ID3D11DepthStencilState* mDepthStencilState;

		XMFLOAT4X4 mWorldMatrix;

	};
}