#include "FpsComponent.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "D3DApp.h"
#include "Utility.h"


namespace Library
{
	RTTI_DEFINITIONS(FpsComponent)

		FpsComponent::FpsComponent(D3DApp& app, XMVECTORF32 color)
		: DrawableComponent(app),
		mSpriteBatch(nullptr),
		mSpriteFont(nullptr),
		mTextPosition(0.0f, 15.0f),
		mFrameCount(0),
		mFrameRate(0.0f),
		mMspf(0.0f),
		mTimeElapsed(0.0f),
		mColor(color)
	{
	}

	FpsComponent::~FpsComponent()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
	}

	XMFLOAT2& FpsComponent::TextPosition()
	{
		return mTextPosition;
	}

	float FpsComponent::FrameRate() const
	{
		return mFrameRate;
	}

	void FpsComponent::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		mSpriteBatch = new SpriteBatch(mApp->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mApp->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void FpsComponent::Update(const Timer &timer)
	{
		// Compute averages over one second period.
		if ((timer.TotalTime() - mTimeElapsed) >= 1.0f)
		{
			mFrameRate = (float)mFrameCount; // mFrameRate = mFrameCount / 1
			mMspf = 1000.0f / mFrameRate;

			mFrameCount = 0;
			mTimeElapsed += 1.0f;
		}

		++mFrameCount;
	}

	void FpsComponent::Draw(const Timer &timer)
	{
		mSpriteBatch->Begin();
		XMFLOAT2 textPosition = mTextPosition;
		std::wostringstream fpsLabel;
		std::wostringstream fmtLabel;
		fpsLabel.precision(6);
		fmtLabel.precision(6);
		
		fpsLabel << L"Frame Rate: " << mFrameRate;
		mSpriteFont->DrawString(mSpriteBatch, fpsLabel.str().c_str(), textPosition, XMLoadFloat4(&mColor));

		fmtLabel << L"Frame Time: " << mMspf << L" (ms)";
		textPosition.y += 16;
		mSpriteFont->DrawString(mSpriteBatch, fmtLabel.str().c_str(), textPosition, XMLoadFloat4(&mColor));
		
		mSpriteBatch->End();
	}
}