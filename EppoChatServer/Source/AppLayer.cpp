#include "AppLayer.h"

#include <EppoCore/Core/BufferReader.h>

ServerAppLayer* ServerAppLayer::s_Instance = nullptr;

namespace
{
    Buffer s_ScratchBuffer(1024);

    void StaticOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
    {
        ServerAppLayer::Get()->OnConnectionStatusChanged(info);
    }
}

ServerAppLayer::ServerAppLayer()
{
    EPPO_ASSERT(!s_Instance)
    s_Instance = this;
}

void ServerAppLayer::OnAttach()
{
    if (SteamDatagramErrMsg errMsg; !GameNetworkingSockets_Init(nullptr, errMsg))
        EPPO_ERROR("Failed to initialize GameNetworkingSockets: {}", errMsg);

    m_Socket = SteamNetworkingSockets();

    const auto& spec = Application::Get().GetSpecification();

    SteamNetworkingIPAddr ipAddr{};
    ipAddr.m_port = 8192;

    SteamNetworkingConfigValue_t options{};
    options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)StaticOnConnectionStatusChanged);

    EPPO_INFO("Creating listening socket on port: {}", ipAddr.m_port);
    m_ListenSocket = m_Socket->CreateListenSocketIP(ipAddr, 1, &options);

    if (m_ListenSocket == k_HSteamListenSocket_Invalid)
        EPPO_ERROR("Failed to create listening socket on port: {}", ipAddr.m_port);

    m_PollGroup = m_Socket->CreatePollGroup();
    if (m_PollGroup == k_HSteamNetPollGroup_Invalid)
        EPPO_ERROR("Failed to create poll group on: {}", ipAddr.m_port);

    EPPO_INFO("Listening on: {}", ipAddr.m_port);
}

void ServerAppLayer::OnDetach()
{
    // Close connections
    // m_Socket->CloseConnection

    s_ScratchBuffer.Release();

    m_Socket->CloseListenSocket(m_ListenSocket);
    m_ListenSocket = k_HSteamListenSocket_Invalid;

    m_Socket->DestroyPollGroup(m_PollGroup);
    m_PollGroup = k_HSteamNetPollGroup_Invalid;

    GameNetworkingSockets_Kill();
}

void ServerAppLayer::OnUpdate(float timestep)
{
    if (m_ListenSocket == k_HSteamListenSocket_Invalid || m_PollGroup == k_HSteamNetPollGroup_Invalid)
        return;

    // Poll messages
    PollIncomingMessages();

    // Poll connection state changes
    m_Socket->RunCallbacks();

    // Poll user input

}

void ServerAppLayer::PollIncomingMessages()
{
    // Poll messages
    ISteamNetworkingMessage* incomingMessage = nullptr;
    while (const int32_t numMessages = m_Socket->ReceiveMessagesOnPollGroup(m_PollGroup, &incomingMessage, 1))
    {
        if (numMessages < 0)
        {
            EPPO_ERROR("Failed polling incoming messages!");
            break;
        }

         // Get associated client
        EPPO_ASSERT(incomingMessage && numMessages == 1)
        auto client = m_Clients.find(incomingMessage->m_conn);
        EPPO_ASSERT(client != m_Clients.end());

        // Copy incoming data to scratch buffer
        EPPO_ASSERT(static_cast<int>(s_ScratchBuffer.Size) >= incomingMessage->m_cbSize)
        s_ScratchBuffer = Buffer::Copy(incomingMessage->m_pData, incomingMessage->m_cbSize);
        incomingMessage->Release();
    }
}

void ServerAppLayer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            EPPO_ASSERT(!m_Clients.contains(info->m_hConn));
            EPPO_INFO("Client connecting from {}", info->m_info.m_szConnectionDescription);

            // Try accept connection
            if (m_Socket->AcceptConnection(info->m_hConn) != k_EResultOK)
            {
                // Failed connection
                m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);
                EPPO_ERROR("Failed to accept connection!");
                break;
            }

            // Assign poll group
            if (!m_Socket->SetConnectionPollGroup(info->m_hConn, m_PollGroup))
            {
                m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);
                EPPO_ERROR("Failed to set poll group!");
                break;
            }

            // Connection succeeded
            ClientInfo& clientInfo = m_Clients[info->m_hConn];
            clientInfo.IPAddress = info->m_info.m_identityRemote.m_ip;

            EPPO_INFO("Client {} connected to server!", info->m_hConn);

            break;
        }

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            EPPO_INFO("Client {} disconnected from server!", info->m_hConn);

            // Ignore if client was not previously connected
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                const auto client = m_Clients.find(info->m_hConn);
                EPPO_ASSERT(client != m_Clients.end())

                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
                    EPPO_WARN("Problem detected on client! ({})", info->m_hConn);
                else
                    EPPO_WARN("Connection closed by peer! ({})", info->m_hConn);

                m_Clients.erase(client);
            }

            m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);

            break;
        }

        case k_ESteamNetworkingConnectionState_FindingRoute:
        case k_ESteamNetworkingConnectionState_Connected:
        case k_ESteamNetworkingConnectionState_FinWait:
        case k_ESteamNetworkingConnectionState_Linger:
        case k_ESteamNetworkingConnectionState_Dead:
        case k_ESteamNetworkingConnectionState__Force32Bit:
            break;
    }
}
