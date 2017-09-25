#include "Component.h"

namespace Library
{
	RTTI_DEFINITIONS(Component)

		Component::Component()
		: mApp(nullptr), mEnabled(true)
	{
	}

	Component::Component(D3DApp& app)
		: mApp(&app), mEnabled(true)
	{
	}

	Component::~Component()
	{
	}

	D3DApp* Component::GetGame()
	{
		return mApp;
	}

	void Component::SetGame(D3DApp& app)
	{
		mApp = &app;
	}

	bool Component::Enabled() const
	{
		return mEnabled;
	}

	void Component::SetEnabled(bool enabled)
	{
		mEnabled = enabled;
	}

	void Component::Initialize()
	{
	}

	void Component::Update(const Timer &timer)
	{

	}
}
