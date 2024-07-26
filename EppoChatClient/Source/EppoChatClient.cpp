#include "AppLayer.h"

#include <EppoCore.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoChatClient : public Application
{
public:
    EppoChatClient(const ApplicationSpecification& specification)
        : Application(specification)
    {
        std::shared_ptr<ClientAppLayer> layer = std::make_shared<ClientAppLayer>();

        PushLayer(layer);
    }

    ~EppoChatClient() = default;
};

Application* Eppo::CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
	spec.CommandLineArgs = args;
	spec.Title = "EppoChatClient";

    return new EppoChatClient(spec);
}