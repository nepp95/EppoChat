#include "AppLayer.h"

#include <EppoCore.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoApp : public Application
{
public:
	EppoApp(const ApplicationSpecification& specification)
		: Application(specification)
	{
		std::shared_ptr<AppLayer> layer = std::make_shared<AppLayer>();

		PushLayer(layer);
	}

	~EppoApp() = default;
};

Application* Eppo::CreateApplication()
{
	ApplicationSpecification spec;
	spec.Title = "EppoApp";

	return new EppoApp(spec);
}
