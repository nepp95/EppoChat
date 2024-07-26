#pragma once

#include <EppoCore.h>
#include "Network.h"

using namespace Eppo;

class ClientAppLayer : public Layer
{
public:
	ClientAppLayer() = default;
	~ClientAppLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Event& e) override;
	void OnUpdate(float timestep) override;
	void OnUIRender() override;

private:
	std::shared_ptr<Network> m_Network;
};
