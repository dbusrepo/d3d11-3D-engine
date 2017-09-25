///////////////////////////////////////////////////////////////////////////////
// Filename: cpuclass.cpp
///////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "D3DApp.h"
#include "Utility.h"
#include "CpuUsageComponent.h"

namespace Library
{

	RTTI_DEFINITIONS(CpuUsageComponent)

	CpuUsageComponent::CpuUsageComponent(D3DApp& app, XMVECTORF32 color)
		: DrawableComponent(app),
		mSpriteBatch(nullptr),
		mSpriteFont(nullptr),
		mTextPosition(0.0f, 15.0f),
		m_canReadCpu(false),
		m_cpuUsage(0),
		m_queryHandle(NULL),
		m_counterHandle(0),
		mTimeElapsed(0.0f),
		mColor(color)
	{
	}


	CpuUsageComponent::CpuUsageComponent(const CpuUsageComponent& other)
	{
	}


	CpuUsageComponent::~CpuUsageComponent()
	{
		if (m_canReadCpu)
		{
			PdhCloseQuery(m_queryHandle);
		}

		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
	}


	void CpuUsageComponent::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		mSpriteBatch = new SpriteBatch(mApp->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mApp->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		initCpuCounter();
	}

	void CpuUsageComponent::initCpuCounter()
	{
		PDH_STATUS status;

		// Initialize the flag indicating whether this object can read the system cpu usage or not.
		m_canReadCpu = true;

		// Create a query object to poll cpu usage.
		status = PdhOpenQuery(NULL, 0, &m_queryHandle);
		if (status != ERROR_SUCCESS)
		{
			m_canReadCpu = false;
		}

		// Set query object to poll all cpus in the system.
		status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
		if (status != ERROR_SUCCESS)
		{
			m_canReadCpu = false;
		}

		m_cpuUsage = 0;
	}

	void CpuUsageComponent::Update(const Timer &timer)
	{
		PDH_FMT_COUNTERVALUE value;

		if (m_canReadCpu)
		{
			if ((timer.TotalTime() - mTimeElapsed) >= 1.0f)
			{
				PdhCollectQueryData(m_queryHandle);

				PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);

				m_cpuUsage = value.longValue;

				mTimeElapsed += 1.0f;

			}
		}

	}


	XMFLOAT2 & CpuUsageComponent::TextPosition()
	{
		return mTextPosition;
	}

	int CpuUsageComponent::GetCpuPercentage() const
	{
		int usage;

		if (m_canReadCpu)
		{
			usage = (int)m_cpuUsage;
		}
		else
		{
			usage = 0;
		}

		return usage;
	}

	void CpuUsageComponent::Draw(const Timer &timer)
	{
		mSpriteBatch->Begin();

		std::wostringstream fpsLabel;
		fpsLabel.precision(6);

		fpsLabel << L"Cpu usage: " << m_cpuUsage << "%";
		mSpriteFont->DrawString(mSpriteBatch, fpsLabel.str().c_str(), mTextPosition, XMLoadFloat4(&mColor));

		mSpriteBatch->End();
	}
}