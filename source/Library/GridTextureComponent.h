#pragma once

#include "Common.h"
#include "DrawableComponent.h"

namespace Library
{
	class CameraComponent;
	class Model;
	class BasicMaterialTexture;

	class GridTextureComponent : public DrawableComponent
	{
		RTTI_DECLARATIONS(GridTextureComponent, DrawableComponent)

	public:
		GridTextureComponent(D3DApp& app, CameraComponent& camera);
		GridTextureComponent(D3DApp& app, CameraComponent& camera, UINT size, UINT scale, XMFLOAT4 color);
		~GridTextureComponent();

		const XMFLOAT3& Position() const;

		void SetPosition(const XMFLOAT3& position);
		void SetPosition(float x, float y, float z);
		void SetTextureTransform(XMMATRIX texMat);

		virtual void Initialize() override;
		virtual void Draw(const Timer &timer) override;

	private:
		GridTextureComponent();
		GridTextureComponent(const GridTextureComponent& rhs);
		GridTextureComponent& operator=(const GridTextureComponent& rhs);

		static const UINT DefaultWidth;
		static const UINT DefaultDepth;
		static const UINT DefaultNumRows;
		static const UINT DefaultNumCols;

		ID3D11RasterizerState* mRasterizerState;
		BasicMaterialTexture *mMaterial;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		//ID3D11Texture2D* mTexture;
		ID3D11ShaderResourceView* mTextureView;

		XMFLOAT3 mPosition;
		UINT mWidth, mDepth;
		UINT mRows, mCols;

		XMFLOAT4X4 mWorldMatrix;
		XMFLOAT4X4 mTexMatrix;
	};
}