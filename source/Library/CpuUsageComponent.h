#pragma once
#include <pdh.h>
#include "DrawableComponent.h"

#pragma comment(lib, "pdh.lib")

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Library
{
	class CpuUsageComponent : public DrawableComponent
	{
		RTTI_DECLARATIONS(CpuUsageComponent, DrawableComponent)

	public:
		CpuUsageComponent(D3DApp& app, XMVECTORF32 color);
		~CpuUsageComponent();

		XMFLOAT2& TextPosition();
		int GetCpuPercentage() const;

		virtual void Initialize() override;
		virtual void Update(const Timer &timer) override;
		virtual void Draw(const Timer &timer) override;

	private:
		CpuUsageComponent();
		CpuUsageComponent(const CpuUsageComponent& rhs);
		CpuUsageComponent& operator=(const CpuUsageComponent& rhs);

		void initCpuCounter();

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
		XMFLOAT4 mColor;

		float mTimeElapsed;
		bool m_canReadCpu;
		HQUERY m_queryHandle;
		HCOUNTER m_counterHandle;
		long m_cpuUsage;
	};
}
