#pragma once

#include "EppoChatCommon/Client.h"
#include "EppoChatCommon/DataPacket.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Buffer.h>

#include <steam/steamnetworkingsockets.h>

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
    auto SendMessage(ClientID clientId, Buffer buffer) -> void;
    auto SendClientList(ClientID clientId) -> void;
    auto SendMessageHistory(ClientID clientId) -> void;
    auto SendWelcomeMessage(ClientID clientId) -> void;
    auto IsValidUsername(std::string_view username) -> bool;

private:
    ISteamNetworkingSockets* m_Socket;
    HSteamListenSocket m_ListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup m_PollGroup = k_HSteamNetPollGroup_Invalid;

    std::map<ClientID, ClientInfo> m_Clients;
    std::map<int64_t, MessageData> m_Messages;
    Buffer m_ScratchBuffer;

    static ServerAppLayer* s_Instance;
};
