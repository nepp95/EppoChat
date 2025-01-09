#pragma once

#include <EppoCore/Core/Application.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

using namespace Eppo;

struct ClientInfo
{
    SteamNetworkingIPAddr IPAddress;
    std::chrono::time_point<std::chrono::system_clock> TimeConnected = std::chrono::system_clock::now();
};

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


private:
    ISteamNetworkingSockets* m_Socket;

    HSteamListenSocket m_ListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup m_PollGroup = k_HSteamNetPollGroup_Invalid;

    std::map<HSteamNetConnection, ClientInfo> m_Clients;

    static ServerAppLayer* s_Instance;
};
