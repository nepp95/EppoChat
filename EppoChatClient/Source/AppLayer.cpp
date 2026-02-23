#include "AppLayer.h"

#include "EppoChatCommon/DataPacket.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/BufferReader.h>
#include <EppoCore/Core/BufferWriter.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <steam/isteamnetworkingutils.h>

#include <regex>

ClientAppLayer* ClientAppLayer::s_Instance = nullptr;

static auto StaticOnConnectionChanged(SteamNetConnectionStatusChangedCallback_t* info) -> void
{
    ClientAppLayer::Get()->OnConnectionStatusChanged(info);
}

static auto DebugCallback(const ESteamNetworkingSocketsDebugOutputType type, const char* message) -> void
{
    switch (type)
    {
        case k_ESteamNetworkingSocketsDebugOutputType_None:
            break;

        case k_ESteamNetworkingSocketsDebugOutputType_Bug:
        case k_ESteamNetworkingSocketsDebugOutputType_Error:
        {
            Log::Error("{}", message);
            break;
        }

        case k_ESteamNetworkingSocketsDebugOutputType_Important:
        case k_ESteamNetworkingSocketsDebugOutputType_Warning:
        {
            Log::Warn("{}", message);
            break;
        }

        case k_ESteamNetworkingSocketsDebugOutputType_Msg:
        {
            Log::Info("{}", message);
            break;
        }

        case k_ESteamNetworkingSocketsDebugOutputType_Verbose:
        case k_ESteamNetworkingSocketsDebugOutputType_Debug:
        case k_ESteamNetworkingSocketsDebugOutputType_Everything:
        case k_ESteamNetworkingSocketsDebugOutputType__Force32Bit:
        {
            Log::Trace("{}", message);
            break;
        }
    }
}

ClientAppLayer::ClientAppLayer()
{
    EP_ASSERT(!s_Instance);
    s_Instance = this;

    // Initialize scratch buffer
    m_ScratchBuffer.Allocate(MAX_PAYLOAD_SIZE);
}

auto ClientAppLayer::OnAttach() -> void
{
    // Initialize networking
    if (SteamDatagramErrMsg errMsg; !GameNetworkingSockets_Init(nullptr, errMsg))
        Log::Error("Failed to initialize GameNetworkingSockets: {}", errMsg);

#ifdef EP_DEBUG
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugCallback);
#else
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugCallback);
#endif

    m_Socket = SteamNetworkingSockets();
}

auto ClientAppLayer::OnDetach() -> void
{
    m_ScratchBuffer.Release();
    GameNetworkingSockets_Kill();
}

auto ClientAppLayer::OnUpdate(float timestep) -> void
{
    if (m_ConnectionStatus != ConnectionStatus::Connected)
        return;

    // Poll messages
    PollIncomingMessages();

    // Poll connection state changes
    m_Socket->RunCallbacks();
}

auto ClientAppLayer::OnEvent(Event& e) -> void
{
}

auto ClientAppLayer::PreUIRender() -> void
{
}

auto ClientAppLayer::OnUIRender() -> void
{
    // Derived from ImGui demo
    static bool s_Open = true;
    static constexpr bool s_Fullscreen = true;
    static constexpr bool s_Padding = false;
    static ImGuiDockNodeFlags s_DockspaceFlags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (s_Fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        s_DockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (s_DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!s_Padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockspace", &s_Open, windowFlags);

    if (!s_Padding)
        ImGui::PopStyleVar();
    if (s_Fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the dockspace
    if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        const ImGuiID dockspaceId = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), s_DockspaceFlags);
    }

    // Main window
    if (m_ConnectionStatus != ConnectionStatus::Connected && !m_ConnectionPopupOpen)
    {
        ImGui::OpenPopup("Connect To Server");
    }

    m_ConnectionPopupOpen = ImGui::BeginPopupModal("Connect To Server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (m_ConnectionPopupOpen)
    {
        ImGui::Text("Username");
        ImGui::InputText("##username", &m_Username);

        ImGui::Text("IP Address (IP:PORT)");
        ImGui::InputText("##ipaddr", &m_IPAddress);

        if (ImGui::Button("Connect"))
        {
            if (IsValidIP(m_IPAddress))
            {
                // Connect to server
                SteamNetworkingConfigValue_t options{};
                options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)StaticOnConnectionChanged);

                Log::Info("Connecting to {}", m_IPAddress);
                m_Connection = m_Socket->ConnectByIPAddress(m_IPAddrSteam, 1, &options);
                if (m_Connection == k_HSteamNetConnection_Invalid)
                    Log::Error("Failed to connect!");
                else
                {
                    m_ConnectionStatus = ConnectionStatus::Connected;
                }
            }
        }

        if (m_ConnectionStatus == ConnectionStatus::Connected)
        {
            // Send connection request
            BufferWriter writer;
            writer.WriteRaw<PacketType>(PacketType::ConnectionRequest);
            writer.WriteString(m_Username);

            const auto buffer = writer.GetBuffer();
            SendMessage(buffer);

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Begin("Users");

    for (const auto& [clientId, clientInfo] : m_ConnectedClients)
        ImGui::Text(clientInfo.Username.c_str());

    ImGui::End(); // Users

    ImGui::Begin("Chat");
    RenderChat();
    ImGui::End(); // Chat

    ImGui::End(); // Dockspace
}

auto ClientAppLayer::PostUIRender() -> void
{
}

auto ClientAppLayer::PollIncomingMessages() -> void
{
    while (true)
    {
        // Poll messages
        ISteamNetworkingMessage* incomingMessage = nullptr;
        const int32_t numMessages = m_Socket->ReceiveMessagesOnConnection(m_Connection, &incomingMessage, 1);

        if (numMessages == 0)
            break;

        if (numMessages == -1)
        {
            Log::Error("Failed polling incoming messages!");
            break;
        }

        // Copy incoming data to scratch buffer
        if (m_ScratchBuffer.Size < incomingMessage->m_cbSize && incomingMessage->m_cbSize < UINT64_MAX)
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
            case PacketType::ConnectionRequest:
            {
                bool acceptedConnection;
                reader.ReadRaw<bool>(acceptedConnection);

                if (!acceptedConnection)
                    Log::Error("Failed to connect because of invalid username");

                break;
            }

            case PacketType::ClientList:
            {
                std::map<ClientID, ClientInfo> clients;
                reader.ReadMap<ClientID, ClientInfo>(clients);

                m_ConnectedClients = clients;

                break;
            }

            case PacketType::Message:
            {
                std::map<int64_t, MessageData> messages;
                reader.ReadMap<int64_t, MessageData>(messages);
                EP_ASSERT(!messages.empty());

                if (messages.size() > 1)
                    m_Messages = messages;
                else
                {
                    const auto message = messages.begin();
                    m_Messages.insert_or_assign(message->first, message->second);
                }

                break;
            }
        }
    }
}

auto ClientAppLayer::OnConnectionStatusChanged(const SteamNetConnectionStatusChangedCallback_t* info) -> void
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_None:
        case k_ESteamNetworkingConnectionState_Connecting:
            break;

        case k_ESteamNetworkingConnectionState_Connected:
        {
            break;
        }

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                // Timed out? Rejected connection? Lots of possibilites
            }
            else if (info->m_eOldState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                // Local problem
            }

            m_Socket->CloseConnection(info->m_hConn, 0, nullptr, false);
            m_Connection = k_HSteamNetConnection_Invalid;
            m_ConnectionStatus = ConnectionStatus::NotConnected;

            break;
        }

        case k_ESteamNetworkingConnectionState_FindingRoute:
        case k_ESteamNetworkingConnectionState_FinWait:
        case k_ESteamNetworkingConnectionState_Linger:
        case k_ESteamNetworkingConnectionState_Dead:
        case k_ESteamNetworkingConnectionState__Force32Bit:
            break;
    }
}

auto ClientAppLayer::SendMessage(const Buffer buffer) const -> void
{
    EP_ASSERT(buffer.Size <= UINT32_MAX); // Below method only accepts up to the max size of 32 bit uint as size
    auto result = m_Socket->SendMessageToConnection(m_Connection, buffer.Data, static_cast<uint32_t>(buffer.Size), 0, nullptr);
}

auto ClientAppLayer::RenderChat() -> void
{
    static bool s_AutoScroll = true;
    static std::string s_InputMessage;

    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &s_AutoScroll);
        ImGui::EndPopup();
    }

    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");

    ImGui::Separator();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));

    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_None;
    if (ImGui::BeginTable("##Messages", 2, tableFlags))
    {
        ImGui::TableSetupColumn("#1st", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("#2nd", ImGuiTableColumnFlags_WidthStretch, 4.0f);

        for (const auto& [timestamp, message] : m_Messages)
        {
            using namespace std::chrono;
            const sys_time utcTime{ milliseconds(timestamp) };
            auto localSeconds = time_point_cast<seconds>(current_zone()->to_local(utcTime));
            zoned_time localTime{ current_zone(), localSeconds };
            const std::string timestampStr = std::format("{:%H:%M:%S}", localTime);

            ImGui::TableNextColumn();
            ImGui::Text("%s", message.Username.c_str());
            ImGui::Text(timestampStr.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(message.Message.c_str());
            ImGui::TableNextRow();
        }

        ImGui::EndTable();
    }

    if (s_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::PopStyleVar();
    ImGui::Separator();

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("Input", &s_InputMessage, inputFlags))
    {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const int64_t tp = time_point_cast<milliseconds>(now).time_since_epoch().count();

        BufferWriter writer;
        writer.WriteRaw<PacketType>(PacketType::Message);
        writer.WriteRaw<int64_t>(tp);
        writer.WriteString(s_InputMessage);

        SendMessage(writer.GetBuffer());

        MessageData message{
            .UserID = m_Connection,
            .Timestamp = tp,
            .Username = m_Username,
            .Message = s_InputMessage,
        };

        m_Messages.insert_or_assign(tp, message);
        s_InputMessage.clear();
    }

    ImGui::SetItemDefaultFocus();
}

auto ClientAppLayer::IsValidIP(const std::string& ipStr) -> bool
{
    const std::regex ipRegexPattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}):([0-9]{1,5})$)");

    // Extract IP and port separately
    std::smatch matches;
    std::regex_match(ipStr, matches, ipRegexPattern);

    const uint8_t i1 = static_cast<uint8_t>(std::stoi(matches[1].str()));
    const uint8_t i2 = static_cast<uint8_t>(std::stoi(matches[2].str()));
    const uint8_t i3 = static_cast<uint8_t>(std::stoi(matches[3].str()));
    const uint8_t i4 = static_cast<uint8_t>(std::stoi(matches[4].str()));
    const uint16_t port = static_cast<uint16_t>(std::stoi(matches[5].str()));

    if (i1 > UINT8_MAX || i2 > UINT8_MAX || i3 > UINT8_MAX || i4 > UINT8_MAX || port > UINT16_MAX)
    {
        Log::Error("Invalid IP address entered!");
        return false;
    }

    const uint32_t ip = i1 << 24 | i2 << 16 | i3 << 8 | i4;
    m_IPAddrSteam.SetIPv4(ip, port);

    return true;
}
