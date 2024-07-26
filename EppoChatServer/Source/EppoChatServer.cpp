#include "AppLayer.h"

#include <EppoCore.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoChatServer : public Application
{
public:
    EppoChatServer(const ApplicationSpecification& specification)
        : Application(specification)
    {
        std::shared_ptr<ServerAppLayer> layer = std::make_shared<ServerAppLayer>();

        PushLayer(layer);
    }

    ~EppoChatServer() = default;
};

Application* Eppo::CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
	spec.CommandLineArgs = args;
	spec.Title = "EppoChatServer";

    return new EppoChatServer(spec);
}