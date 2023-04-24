#pragma once

#include "ScriptSystem.h"

namespace Nork {

template<class T> static std::string MakeId() {
	return typeid(T).name();
}

template<class... ScriptTypes>
class TemplateScriptFactory : public ScriptFactory {
public:
	TemplateScriptFactory() : ids(CreateIds()) {}
	const std::vector<std::string>& Ids() const override {
		return ids;
	}
	std::shared_ptr<Script> Create(const std::shared_ptr<SceneNode>& node, const std::string& id) override {
		return Create<ScriptTypes...>(node, id);
	}
private:
	template<class T, class... R>
	std::shared_ptr<Script> Create(const std::shared_ptr<SceneNode>& node, const std::string& id) {
		if (MakeId<T>() == id) {
			return std::make_shared<T>(node);
		}
		if constexpr (sizeof...(R) > 0) {
			return Create<R...>(node, id);
		}
		throw std::runtime_error("failed to find script with id: " + id);
	}
public:
	std::vector<std::string> ids;
private:
	static std::vector<std::string> CreateIds() {
		std::vector<std::string> ids;
		CreateIds<ScriptTypes...>(ids);
		return ids;
	}
	template<class T, class... R> static void CreateIds(std::vector<std::string>& ids) {
		ids.push_back(MakeId<T>());
		if constexpr (sizeof...(R) > 0) {
			CreateIds<R...>(ids);
		}
	}
};
}