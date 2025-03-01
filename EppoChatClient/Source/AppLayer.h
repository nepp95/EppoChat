#pragma once

#include <EppoChatCommon/DataPacket.h>
#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Buffer.h>

#include <steam/steamnetworkingsockets.h>

class ClientAppLayer : public Eppo::Layer
{
public:
    ClientAppLayer();
    ~ClientAppLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float timestep) override;
    void OnUIRender() override;

    void PollIncomingMessages() const;
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
    void PollUserInput();

    static ClientAppLayer* Get() { return s_Instance; }

private:
    void ConnectToServer();

private:
    ISteamNetworkingSockets* m_Socket{};
    HSteamNetConnection m_Connection{};

    enum class Layout : uint8_t
    {
        Connect,
        Chat
    };

    Layout m_DesiredLayout = Layout::Connect;
    Layout m_CurrentLayout = Layout::Connect;

    std::array<Eppo::ScopedBuffer, 2> m_LayoutBuffer;

    bool m_IsConnected = false;

    static ClientAppLayer* s_Instance;
};
