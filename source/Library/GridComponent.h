#pragma once

#include "Common.h"
#include "DrawableComponent.h"

namespace Library
{
	class CameraComponent;
	class Model;
	class BasicMaterialFixedColor;

	class GridComponent : public DrawableComponent
	{
		RTTI_DECLARATIONS(GridComponent, DrawableComponent)

	public:
		GridComponent(D3DApp& app, CameraComponent& camera);
		GridComponent(D3DApp& app, CameraComponent& camera, UINT size, UINT scale, XMFLOAT4 color);
		~GridComponent();

		const XMFLOAT3& Position() const;
		const XMFLOAT4& Color() const;

		void SetPosition(const XMFLOAT3& position);
		void SetPosition(float x, float y, float z);
		void SetColor(const XMFLOAT4& color);

		virtual void Initialize() override;
		virtual void Draw(const Timer &timer) override;

	private:
		GridComponent();
		GridComponent(const GridComponent& rhs);
		GridComponent& operator=(const GridComponent& rhs);

		void InitializeGrid();

		static const UINT DefaultWidth;
		static const UINT DefaultDepth;
		static const UINT DefaultNumRows;
		static const UINT DefaultNumCols;

		static const XMFLOAT4 DefaultColor;

		
		ID3D11RasterizerState* mWireframeRS;
		Model *mGrid;
		BasicMaterialFixedColor *mMaterial;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMFLOAT3 mPosition;
		UINT mWidth, mDepth;
		UINT mRows, mCols;
		XMFLOAT4 mColor;
		XMFLOAT4X4 mWorldMatrix;
	};
}