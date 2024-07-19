#pragma once

#include <EppoCore.h>

using namespace Eppo;

class AppLayer : public Layer
{
public:
	AppLayer() = default;
	~AppLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Event& e) override;
	void OnUpdate(float timestep) override;
	void OnUIRender() override;
};
