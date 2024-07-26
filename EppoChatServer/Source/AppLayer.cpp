#include "AppLayer.h"

#include <EppoCore.h>
#include <imgui/imgui.h>

ServerAppLayer* ServerAppLayer::s_Instance = nullptr;

static void SteamOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    ServerAppLayer::Get()->OnConnectionStatusChanged(info);
}

void ServerAppLayer::OnAttach()
{
	SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
    {
        EPPO_ERROR("Failed to initialize GameNetworkingSockets: {}", errMsg);
    }

	m_Socket = SteamNetworkingSockets();

	const ApplicationSpecification& spec = Application::Get().GetSpecification();

	SteamNetworkingIPAddr ipAddr;
    ipAddr.Clear();
    ipAddr.m_port = atoi(spec.CommandLineArgs[1]);

	SteamNetworkingConfigValue_t options;
    options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamOnConnectionStatusChanged);

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

	m_Socket->CloseListenSocket(m_ListenSocket);
    m_ListenSocket = k_HSteamListenSocket_Invalid;

    m_Socket->DestroyPollGroup(m_PollGroup);
    m_PollGroup = k_HSteamNetPollGroup_Invalid;
}

void ServerAppLayer::OnEvent(Event& e)
{

}

void ServerAppLayer::OnUpdate(float timestep)
{
	// Poll messages
	// Poll connection
	m_Socket->RunCallbacks();
	// Poll user input
	// Sleep because we don't want 100% CPU usage
}

void ServerAppLayer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info)
{
	
}
