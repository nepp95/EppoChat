#include "AppLayer.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/BufferWriter.h>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <GLFW/glfw3.h>

#include <regex>

ClientAppLayer* ClientAppLayer::s_Instance = nullptr;

namespace
{
    Buffer s_ScratchBuffer(1024);

    void StaticOnConnectionChanged(SteamNetConnectionStatusChangedCallback_t* info)
    {
        ClientAppLayer::Get()->OnConnectionStatusChanged(info);
    }

    void DebugCallback(ESteamNetworkingSocketsDebugOutputType type, const char* message)
    {
        switch (type)
        {
            case k_ESteamNetworkingSocketsDebugOutputType_None:
                break;

            case k_ESteamNetworkingSocketsDebugOutputType_Bug:
            case k_ESteamNetworkingSocketsDebugOutputType_Error:
            {
                EPPO_ERROR(message);
                break;
            }

            case k_ESteamNetworkingSocketsDebugOutputType_Important:
            case k_ESteamNetworkingSocketsDebugOutputType_Warning:
            {
                EPPO_WARN(message);
                break;
            }

            case k_ESteamNetworkingSocketsDebugOutputType_Msg:
            {
                EPPO_INFO(message);
                break;
            }

            case k_ESteamNetworkingSocketsDebugOutputType_Verbose:
            case k_ESteamNetworkingSocketsDebugOutputType_Debug:
            case k_ESteamNetworkingSocketsDebugOutputType_Everything:
            case k_ESteamNetworkingSocketsDebugOutputType__Force32Bit:
            {
                EPPO_TRACE(message);
            }
        }
    }

    SteamNetworkingIPAddr ParseIPString(const std::string& ipStr)
    {
        const std::regex ipRegexPattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}):([0-9]{1,5})$)");

        // Extract IP and port separately
        std::smatch matches;
        std::regex_match(ipStr, matches, ipRegexPattern);

        // Extract 4 parts of the IP
        const uint8_t i1 = static_cast<uint8_t>(std::stoi(matches[1].str()));
        const uint8_t i2 = static_cast<uint8_t>(std::stoi(matches[2].str()));
        const uint8_t i3 = static_cast<uint8_t>(std::stoi(matches[3].str()));
        const uint8_t i4 = static_cast<uint8_t>(std::stoi(matches[4].str()));
        const uint16_t port = static_cast<uint16_t>(std::stoi(matches[5].str()));

        if (i1 > UINT8_MAX || i2 > UINT8_MAX || i3 > UINT8_MAX || i4 > UINT8_MAX || port > UINT16_MAX)
        {
            EPPO_ERROR("Invalid IP address entered!");
            return {};
        }

        const uint32_t ip = i1 << 24 | i2 << 16 | i3 << 8 | i4;

        SteamNetworkingIPAddr ipAddr{};
        ipAddr.SetIPv4(ip, port);

        return ipAddr;
    }
}

ClientAppLayer::ClientAppLayer()
{
    EPPO_ASSERT(!s_Instance)
    s_Instance = this;
}

void ClientAppLayer::OnAttach()
{
    if (SteamDatagramErrMsg errMsg; !GameNetworkingSockets_Init(nullptr, errMsg))
        EPPO_ERROR("Failed to initialize GameNetworkingSockets: {}", errMsg);

    #ifdef EPPO_DEBUG
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugCallback);
    #else
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugCallback);
    #endif

    m_Socket = SteamNetworkingSockets();
}

void ClientAppLayer::OnDetach()
{
    s_ScratchBuffer.Release();
    GameNetworkingSockets_Kill();
}

void ClientAppLayer::OnUpdate(float timestep)
{
    if (!m_IsConnected)
        return;

    // Poll messages
    PollIncomingMessages();

    // Poll connection state changes
    m_Socket->RunCallbacks();

    // Poll user input
    PollUserInput();
}

void ClientAppLayer::OnUIRender()
{
    // From ImGui example
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    const bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Dockspace", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    const ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    const float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    style.WindowMinSize.x = minWinSizeX;

    int width, height;
    glfwGetWindowSize(Application::Get().GetWindow().GetNativeWindow(), &width, &height);
    int xPos, yPos;
    glfwGetWindowPos(Application::Get().GetWindow().GetNativeWindow(), &xPos, &yPos);

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) / 2.0f + xPos, static_cast<float>(height) / 2.0f + yPos), 0, ImVec2(0.5f, 0.5f));
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    ImGui::Begin("Connect", &dockspaceOpen, flags);

    ImGui::BeginGroup();
    ImGui::Text("IP Address (IP:PORT)");
    ImGui::InputText("##ipaddr", s_ScratchBuffer.As<char>(), 100);
    if (ImGui::Button("Connect"))
        ConnectToServer();
    ImGui::EndGroup();

    ImGui::End(); // Login

    ImGui::End(); // Dockspace
}

void ClientAppLayer::PollIncomingMessages() const
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
            EPPO_ERROR("Failed polling incoming messages!");
            break;
        }

        EPPO_INFO("Message received of size {}", incomingMessage->m_cbSize);
        incomingMessage->Release();
    }
}

void ClientAppLayer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_None:
        case k_ESteamNetworkingConnectionState_Connecting:
            break;

        case k_ESteamNetworkingConnectionState_Connected:
        {
            EPPO_INFO("Connected to server!");
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
            m_IsConnected = false;

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

void ClientAppLayer::PollUserInput()
{
    static bool sent = false;

    if (!sent)
    {
        const std::string str = "Hello from client!";

        BufferWriter writer(s_ScratchBuffer);
        writer.Write(const_cast<char*>(str.c_str()), str.size());

        m_Socket->SendMessageToConnection(m_Connection, s_ScratchBuffer.Data, writer.BytesWritten(), k_nSteamNetworkingSend_Reliable,
                                          nullptr);

        sent = true;
    }
}

void ClientAppLayer::ConnectToServer()
{
    auto str = std::string(s_ScratchBuffer.As<char>(), 100);
    if (const size_t pos = str.find_last_not_of('\0'); pos != std::string::npos)
        str.resize(pos + 1);

    if (const std::regex ipRegexPattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}):([0-9]{1,5})$)");
        !std::regex_match(str, ipRegexPattern))
    {
        // TODO: Visual feedback
        EPPO_ERROR("Entered address does not match criteria!");
        return;
    }

    const SteamNetworkingIPAddr ipAddr = ParseIPString(str);

    SteamNetworkingConfigValue_t options{};
    options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)StaticOnConnectionChanged);

    EPPO_INFO("Connecting to {}", str);
    m_Connection = m_Socket->ConnectByIPAddress(ipAddr, 1, &options);
    if (m_Connection == k_HSteamNetConnection_Invalid)
        EPPO_ERROR("Failed to connect!");
    else
        m_IsConnected = true;
}
