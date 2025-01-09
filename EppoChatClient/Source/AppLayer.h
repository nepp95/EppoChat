#pragma once

#include <EppoCore/Core/Application.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

using namespace Eppo;

class ClientAppLayer : public Layer
{
public:
    ClientAppLayer();
    ~ClientAppLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float timestep) override;
    void OnUIRender() override;

    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

    static ClientAppLayer* Get() { return s_Instance; }

private:
    void ConnectToServer();

private:
    ISteamNetworkingSockets* m_Socket;
    HSteamNetConnection m_Connection;

    bool m_IsConnected = false;

    static ClientAppLayer* s_Instance;
};
