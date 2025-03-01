#include "AppLayer.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoChatClient : public Application
{
public:
    explicit EppoChatClient(const ApplicationSpecification& specification)
        : Application(specification)
    {
        const auto layer = std::make_shared<ClientAppLayer>();

        PushLayer(layer);
    }

    ~EppoChatClient() = default;
};

Application* Eppo::CreateApplication(const ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
	spec.CommandLineArgs = args;
	spec.Title = "EppoChatClient";

    return new EppoChatClient(spec);
}
