#include "DrawableComponent.h"

namespace Library
{
	RTTI_DEFINITIONS(DrawableComponent)

		DrawableComponent::DrawableComponent()
		: Component(), mVisible(true), mCamera(nullptr)
	{
	}

	DrawableComponent::DrawableComponent(D3DApp& app)
		: Component(app), mVisible(true), mCamera(nullptr)
	{
	}

	DrawableComponent::DrawableComponent(D3DApp& app, CameraComponent& camera)
		: Component(app), mVisible(true), mCamera(&camera)
	{
	}

	DrawableComponent::~DrawableComponent()
	{
	}

	bool DrawableComponent::Visible() const
	{
		return mVisible;
	}

	void DrawableComponent::SetVisible(bool visible)
	{
		mVisible = visible;
	}

	CameraComponent* DrawableComponent::GetCamera()
	{
		return mCamera;
	}

	void DrawableComponent::SetCamera(CameraComponent* camera)
	{
		mCamera = camera;
	}

	void DrawableComponent::Draw(const Timer &timer)
	{
	}
}