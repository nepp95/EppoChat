#include "AppLayer.h"

#include <EppoCore.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoChat : public Application
{
public:
	EppoChat(const ApplicationSpecification& specification)
		: Application(specification)
	{
		std::shared_ptr<AppLayer> layer = std::make_shared<AppLayer>();

		PushLayer(layer);
	}

	~EppoChat() = default;
};

Application* Eppo::CreateApplication()
{
	ApplicationSpecification spec;
	spec.Title = "EppoChat";

	return new EppoChat(spec);
}
