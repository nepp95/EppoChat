#pragma once

#include <EppoCore/Core/Application.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

using namespace Eppo;

class ServerAppLayer : public Layer
{
public:
	ServerAppLayer();
	~ServerAppLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float timestep) override;

	void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

	static ServerAppLayer* Get() { return s_Instance; }

private:
	ISteamNetworkingSockets* m_Socket;

	HSteamListenSocket m_ListenSocket;
    HSteamNetPollGroup m_PollGroup;

	static ServerAppLayer* s_Instance;
};
