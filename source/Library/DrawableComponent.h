#pragma once

#include "Component.h"

namespace Library
{
	class CameraComponent;

	class DrawableComponent : public Component
	{
		RTTI_DECLARATIONS(DrawableComponent, Component)

	public:
		DrawableComponent();
		DrawableComponent(D3DApp& app);
		DrawableComponent(D3DApp& app, CameraComponent& camera);
		virtual ~DrawableComponent();

		bool Visible() const;
		void SetVisible(bool visible);

		CameraComponent* GetCamera();
		void SetCamera(CameraComponent* camera);

		virtual void Draw(const Timer &timer);

	protected:
		bool mVisible;
		CameraComponent* mCamera;

	private:
		DrawableComponent(const DrawableComponent& rhs);
		DrawableComponent& operator=(const DrawableComponent& rhs);
	};
}