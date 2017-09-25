#pragma once
#include "DrawableComponent.h"

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Library
{
	class FpsComponent : public DrawableComponent
	{
		RTTI_DECLARATIONS(FpsComponent, DrawableComponent)

	public:
		FpsComponent(D3DApp& app, XMVECTORF32 color);
		~FpsComponent();

		XMFLOAT2& TextPosition();
		float FrameRate() const;

		virtual void Initialize() override;
		virtual void Update(const Timer &timer) override;
		virtual void Draw(const Timer &timer) override;

	private:
		FpsComponent();
		FpsComponent(const FpsComponent& rhs);
		FpsComponent& operator=(const FpsComponent& rhs);

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
		XMFLOAT4 mColor;
		
		size_t mFrameCount;
		float mFrameRate;
		float mMspf;
		float mTimeElapsed;
	};
}
