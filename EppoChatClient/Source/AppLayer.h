#pragma once

#include "EppoChatCommon/Client.h"
#include "EppoChatCommon/DataPacket.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Buffer.h>

#include <steam/steamnetworkingsockets.h>

using namespace Eppo;

enum class ConnectionStatus
{
    NotConnected,
    Connecting,
    Connected,
    FailedToConnect,
};

class ClientAppLayer : public Layer
{
public:
    ClientAppLayer();
    ~ClientAppLayer() override = default;

    auto OnAttach() -> void override;
    auto OnDetach() -> void override;

    auto OnUpdate(float timestep) -> void override;
    auto OnEvent(Event& e) -> void override;
    auto PreUIRender() -> void override;
    auto OnUIRender() -> void override;
    auto PostUIRender() -> void override;

    auto PollIncomingMessages() -> void;
    auto OnConnectionStatusChanged(const SteamNetConnectionStatusChangedCallback_t* info) -> void;

    static auto Get() -> ClientAppLayer* { return s_Instance; }

private:
    auto SendMessage(Buffer buffer) const -> void;
    auto RenderChat() -> void;
    auto IsValidIP(const std::string& ipStr) -> bool;

private:
    // Socket
    ISteamNetworkingSockets* m_Socket{};
    HSteamNetConnection m_Connection{};

    // Connection info and connection data buffer
    ConnectionStatus m_ConnectionStatus = ConnectionStatus::NotConnected;
    bool m_ConnectionPopupOpen = false;
    Buffer m_ScratchBuffer;

    // Client and server info
    std::string m_Username;
    std::string m_IPAddress;
    SteamNetworkingIPAddr m_IPAddrSteam;

    // Chat info
    std::map<ClientID, ClientInfo> m_ConnectedClients;
    std::vector<MessageData> m_Messages;
    int64_t m_LastTypeTime;

    static ClientAppLayer* s_Instance;
};
