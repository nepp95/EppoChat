#include "AppLayer.h"

#include <EppoCore/Core/BufferReader.h>
#include <EppoCore/Core/BufferWriter.h>

ServerAppLayer* ServerAppLayer::s_Instance = nullptr;

static auto StaticOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) -> void
{
    ServerAppLayer::Get()->OnConnectionStatusChanged(info);
}

ServerAppLayer::ServerAppLayer()
{
    EP_ASSERT(!s_Instance);
    s_Instance = this;

    // Initialize scratch buffer
    m_ScratchBuffer.Allocate(MAX_PAYLOAD_SIZE);
}

void ServerAppLayer::OnAttach()
{
    if (SteamDatagramErrMsg errMsg; !GameNetworkingSockets_Init(nullptr, errMsg))
        Log::Error("Failed to initialize GameNetworkingSockets: {}", errMsg);

    m_Socket = SteamNetworkingSockets();

    SteamNetworkingIPAddr ipAddr{};
    ipAddr.m_port = 598;

    SteamNetworkingConfigValue_t options{};
    options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)StaticOnConnectionStatusChanged);

    Log::Info("Creating listening socket on port: {}", ipAddr.m_port);
    m_ListenSocket = m_Socket->CreateListenSocketIP(ipAddr, 1, &options);

    if (m_ListenSocket == k_HSteamListenSocket_Invalid)
        Log::Error("Failed to create listening socket on port: {}", ipAddr.m_port);

    m_PollGroup = m_Socket->CreatePollGroup();
    if (m_PollGroup == k_HSteamNetPollGroup_Invalid)
        Log::Error("Failed to create poll group on: {}", ipAddr.m_port);

    Log::Info("Listening on: {}", ipAddr.m_port);
}

void ServerAppLayer::OnDetach()
{
    // Close connections
    for (const auto& [connection, clientInfo] : m_Clients)
    {
        Log::Info("Closing connection of user '{}'", clientInfo.Username);
        m_Socket->CloseConnection(connection, static_cast<int>(DisconnectReason::ServerClosed), nullptr, true);
    }

    m_Socket->CloseListenSocket(m_ListenSocket);
    m_ListenSocket = k_HSteamListenSocket_Invalid;

    m_Socket->DestroyPollGroup(m_PollGroup);
    m_PollGroup = k_HSteamNetPollGroup_Invalid;

    GameNetworkingSockets_Kill();
    m_ScratchBuffer.Release();
}

void ServerAppLayer::OnUpdate(float timestep)
{
    if (m_ListenSocket == k_HSteamListenSocket_Invalid || m_PollGroup == k_HSteamNetPollGroup_Invalid)
        return;

    // Poll messages
    PollIncomingMessages();

    // Poll connection state changes
    m_Socket->RunCallbacks();

    // Update clients
    for (const auto& clientId : m_Clients | std::views::keys)
    {
        // Is typing updates
        /*BufferWriter writer;
        writer.WriteRaw<PacketType>(PacketType::ClientTyping);
        writer.WriteRaw<uint32_t>(clientId);
        writer.WriteRaw<bool>(isTyping);
        // or do a send to all?*/
    }
}

void ServerAppLayer::PollIncomingMessages()
{
    // Poll messages
    ISteamNetworkingMessage* incomingMessage = nullptr;
    while (const int32_t numMessages = m_Socket->ReceiveMessagesOnPollGroup(m_PollGroup, &incomingMessage, 1))
    {
        if (numMessages < 0)
        {
            Log::Error("Failed polling incoming messages!");
            break;
        }

        // Get associated client
        EP_ASSERT(incomingMessage && numMessages == 1);
        const ClientID clientId = incomingMessage->m_conn;

        // Copy incoming data to scratch buffer
        if (std::cmp_less(m_ScratchBuffer.Size, incomingMessage->m_cbSize))
            m_ScratchBuffer.Allocate(incomingMessage->m_cbSize);
        std::memset(m_ScratchBuffer.Data, 0, m_ScratchBuffer.Size);
        std::memcpy(m_ScratchBuffer.Data, incomingMessage->m_pData, incomingMessage->m_cbSize);
        incomingMessage->Release();

        // Process incoming data
        BufferReader reader(m_ScratchBuffer);

        // Get packet type
        PacketType type;
        const bool success = reader.ReadRaw<PacketType>(type);
        EP_ASSERT(success);

        // Process packet payload
        switch (type)
        {
            case PacketType::ClientList:
            case PacketType::ClientConnected:
            case PacketType::ClientQuit:
            case PacketType::Error:
            {
                const std::string errorMsg =
                    std::format("Received packet type '{}' from client is not allowed!", static_cast<uint16_t>(type));

                BufferWriter writer;
                writer.WriteRaw<PacketType>(PacketType::Error);
                writer.WriteString(errorMsg);

                SendMessage(clientId, writer.GetBuffer());

                break;
            }

            case PacketType::ClientTyping:
            {
                if (m_Clients.contains(clientId))
                {
                    bool isTyping;
                    reader.ReadRaw<bool>(isTyping);

                    m_Clients.at(clientId).IsTyping = isTyping;
                }
                else
                {
                    const std::string errorMsg =
                        std::format("Received packet type '{}' from client is only allowed when connected!", static_cast<uint16_t>(type));

                    BufferWriter writer;
                    writer.WriteRaw<PacketType>(PacketType::Error);
                    writer.WriteString(errorMsg);

                    SendMessage(clientId, writer.GetBuffer());
                }

                break;
            }

            case PacketType::ConnectionRequest:
            {
                std::string username;
                reader.ReadString(username);

                const bool validUsername = IsValidUsername(username);

                BufferWriter writer;
                writer.WriteRaw<PacketType>(PacketType::ConnectionRequest);
                writer.WriteRaw<bool>(validUsername);
                SendMessage(clientId, writer.GetBuffer());

                if (validUsername)
                {
                    // Update client info
                    auto& clientInfo = m_Clients[clientId];
                    clientInfo.Username = username;

                    // Send connected users to everyone
                    for (const auto& id : m_Clients | std::views::keys)
                        SendClientList(id);

                    // Send welcome message
                    SendWelcomeMessage(clientId);
                }
                else
                {
                    // Disconnect client
                    m_Socket->CloseConnection(clientId, 0, "Username rejected", true);
                }

                break;
            }

            case PacketType::Message:
            {
                const auto& clientInfo = m_Clients.at(clientId);

                MessageData messageData;
                reader.ReadObject<MessageData>(messageData);

                // In case user tries to spoof their id, overwrite with what we know is the sender
                messageData.UserID = clientId;
                messageData.Username = clientInfo.Username;

                Log::Info("Message received: {}", messageData.Message);

                m_Messages.emplace_back(messageData);

                // Send to clients
                BufferWriter writer;
                writer.WriteRaw<PacketType>(PacketType::Message);
                writer.WriteObject<MessageData>(messageData);

                for (const auto& id : m_Clients | std::views::keys)
                {
                    if (id == clientId)
                        continue;

                    SendMessage(id, writer.GetBuffer());
                }

                break;
            }
        }
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
            EP_ASSERT(!m_Clients.contains(info->m_hConn));
            Log::Info("Client connecting from {}", info->m_info.m_szConnectionDescription);

            // Try accept connection
            if (m_Socket->AcceptConnection(info->m_hConn) != k_EResultOK)
            {
                // Failed connection
                m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);
                Log::Error("Failed to accept connection!");
                break;
            }

            // Assign poll group
            if (!m_Socket->SetConnectionPollGroup(info->m_hConn, m_PollGroup))
            {
                m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);
                Log::Error("Failed to set poll group!");
                break;
            }

            break;
        }

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            Log::Info("Client {} disconnected from server!", info->m_hConn);

            // Ignore if client was not previously connected
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                const auto client = m_Clients.find(info->m_hConn);
                EP_ASSERT(client != m_Clients.end());

                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
                    Log::Warn("Problem detected on client! ({})", info->m_hConn);
                else
                    Log::Warn("Connection closed by peer! ({})", info->m_hConn);

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

auto ServerAppLayer::SendMessage(const ClientID clientId, const Buffer buffer) const -> void
{
    EP_ASSERT(buffer.Size <= UINT32_MAX); // Below method only accepts up to the max size of 32 bit uint as size
    auto result = m_Socket->SendMessageToConnection(clientId, buffer.Data, static_cast<uint32_t>(buffer.Size), 0, nullptr);
}

auto ServerAppLayer::SendClientList(const ClientID clientId) const -> void
{
    BufferWriter writer;
    writer.WriteRaw<PacketType>(PacketType::ClientList);
    writer.WriteMap<ClientID, ClientInfo>(m_Clients);

    SendMessage(clientId, writer.GetBuffer());
}

auto ServerAppLayer::SendWelcomeMessage(const ClientID clientId) const -> void
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const int64_t tp = time_point_cast<milliseconds>(now).time_since_epoch().count();

    const MessageData message{
        .UserID = 0,
        .Timestamp = tp,
        .Username = "Admin",
        .Message = "Welcome to the server!",
    };

    // Messages always get sent as a map to allow multiple messages in one packet
    BufferWriter writer;
    writer.WriteRaw<PacketType>(PacketType::Message);
    writer.WriteObject<MessageData>(message);

    SendMessage(clientId, writer.GetBuffer());
}

auto ServerAppLayer::IsValidUsername(const std::string& username) -> bool
{
    // We do not allow names with different case sizes but further identical
    std::string requestedName = username;

    std::ranges::transform(
        requestedName, requestedName.begin(),
        [](const unsigned char c) -> char
        {
            return static_cast<char>(std::tolower(c));
        }
    );

    // Reserved names
    for (constexpr std::array reservedNames = { "admin", "moderator", "root" }; const auto& reservedName : reservedNames)
    {
        if (requestedName == reservedName)
            return false;
    }

    // Existing names
    for (const auto& clientInfo : m_Clients | std::views::values)
    {
        if (clientInfo.Username == username)
            return false;
    }

    return true;
}
