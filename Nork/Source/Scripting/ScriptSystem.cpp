#include "ScriptSystem.h"
#include "Core/Engine.h"
#include <Windows.h>

namespace Nork {
static ScriptSystem* instance;
ScriptSystem::ScriptSystem() {
    instance = this;
}
ScriptSystem& ScriptSystem::Instance() {
    return *instance;
}

void ScriptSystem::Update() {
	for (auto [ent, comp] : Engine::Get().scene.registry.view<ScriptComponent>().each()) {
        if (comp.script) {
		    comp.script->Run();
        }
	}
}

void ScriptSystem::LoadDLL(const fs::path& path)
{
    HMODULE hModule = LoadLibrary(path.c_str());

    if (hModule == NULL) {
        std::unreachable();
    }
    GetFactoryFunc getFactory = (GetFactoryFunc)GetProcAddress(hModule, "GetFactory");
    if (getFactory == nullptr) {
        std::unreachable();
    }

    scriptFactory = getFactory();

    // FreeLibrary(hModule);
}

}