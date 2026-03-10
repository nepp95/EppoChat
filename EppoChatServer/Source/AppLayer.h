#pragma once

#include "EppoChatCommon/Client.h"
#include "EppoChatCommon/DataPacket.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Buffer.h>

#include <steam/steamnetworkingsockets.h>

#include <deque>

using namespace Eppo;

enum class DisconnectReason
{
    ServerClosed = 0,
    Kicked = 1,
};

class ServerAppLayer : public Layer
{
public:
    ServerAppLayer();
    ~ServerAppLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float timestep) override;

    void PollIncomingMessages();
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

    static auto Get() -> ServerAppLayer* { return s_Instance; }

private:
    auto SendMessage(ClientID clientId, Buffer buffer) const -> void;
    auto SendClientList(ClientID clientId) const -> void;
    auto SendWelcomeMessage(ClientID clientId) const -> void;
    auto IsValidUsername(const std::string& username) -> bool;

private:
    ISteamNetworkingSockets* m_Socket;
    HSteamListenSocket m_ListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup m_PollGroup = k_HSteamNetPollGroup_Invalid;

    std::map<ClientID, ClientInfo> m_Clients;
    std::vector<MessageData> m_Messages;
    Buffer m_ScratchBuffer;

    static ServerAppLayer* s_Instance;
};
