#include "Scripting/Nork.h"
#include "Scripting/TemplateScriptFactory.h"
#include "Components/Common.h"

class Script: public Nork::Script {
public:
	using Nork::Script::Script;
	const std::string& Id() override {
		static auto id = Nork::MakeId<Script>();
		return id;
	}
	void Run() override {
		Nork::Components::Transform& tr = Entity().GetComponent<Nork::Components::Transform>();
		tr.localPosition = {10, 10, 10};
		//auto& kinem = Entity().GetComponent<Nork::Components::Physics>().Kinem();
		//kinem.velocity.y += 1;
	}
public:
};

using ScriptFactory = Nork::TemplateScriptFactory<Script>;

extern "C" __declspec(dllexport) std::shared_ptr<Nork::ScriptFactory> GetFactory()
{
	return std::make_shared<ScriptFactory>();
}
