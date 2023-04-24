#pragma once

#include "Scene/Scene.h"

namespace Nork {

class Script {
public:
	virtual void Run() = 0;
	virtual const std::string& Id() = 0;
public:
	Script(const std::shared_ptr<SceneNode>& node) : node(node) {}
	Entity& Entity() {
		return node->GetEntity();
	}
	SceneNode& Node() {
		return *node;
	}
public:
	std::shared_ptr<SceneNode> node;
};

class ScriptFactory {
public:
	virtual const std::vector<std::string>& Ids() const = 0;
	virtual std::shared_ptr<Script> Create(const std::shared_ptr<SceneNode>& node, const std::string& id) = 0;
};

struct ScriptComponent {
	std::shared_ptr<Script> script;
};

class ScriptSystem
{
public:
	ScriptSystem();
	void LoadDLL(const fs::path&);
	void Update();
	static ScriptSystem& Instance();
public:
	std::shared_ptr<ScriptFactory> scriptFactory;
};

typedef std::shared_ptr<ScriptFactory> (*GetFactoryFunc)();
}

