#include "AppLayer.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoChatServer : public Application
{
public:
	EppoChatServer(ApplicationSpecification specification)
		: Application(std::move(specification))
    {
	    const auto layer = std::make_shared<ServerAppLayer>();

        PushLayer(layer);
    }

    ~EppoChatServer() = default;
};

Application* Eppo::CreateApplication(const ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.CommandLineArgs = args;
	spec.Title = "EppoChatServer";

    return new EppoChatServer(spec);
}
