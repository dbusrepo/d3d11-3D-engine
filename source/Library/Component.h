#pragma once

#pragma once

#include "Common.h"

namespace Library
{
	class D3DApp;
	class Timer;

	class Component : public RTTI
	{
		RTTI_DECLARATIONS(Component, RTTI)

	public:
		Component();
		Component(D3DApp& app);
		virtual ~Component();

		D3DApp* GetGame();
		void SetGame(D3DApp& app);
		bool Enabled() const;
		void SetEnabled(bool enabled);

		virtual void Initialize();
		virtual void Update(const Timer &timer);

	protected:
		D3DApp* mApp;
		bool mEnabled;

	private:
		Component(const Component& rhs);
		Component& operator=(const Component& rhs);
	};
}
