#pragma once

#include "Common.h"
#include "../Utils.h"

namespace std {
	template <>
	class hash<Nork::Physics::ObjectHandle>
	{
	public:
		size_t operator()(const Nork::Physics::ObjectHandle& val) const
		{
			return std::hash<uint64_t>()(val.handle);
		}
	};
}

namespace Nork::Physics
{
	class World
	{
	public:
		World();
		// std::vector<KinematicData> kinems;
		// std::vector<Collider> colliders;

		std::vector<Object> objs; // objects with active colliders should be sorted ahead, so SAP can iteratre easily 
		std::vector<ObjectHandle> handles;
		std::unordered_map<ObjectHandle, int> handleObjIdxMap;

		ObjectHandle AddObject(const Object& obj)
		{
			objs.push_back(obj);
			handles.push_back(ObjectHandle{ .handle = objs.size() - 1});
			handleObjIdxMap[handles.back()] = objs.size() - 1;

			return handles.back();
		}
	};
}