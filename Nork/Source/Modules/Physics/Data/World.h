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

		std::vector<Object> objs; // objects with active colliders should be sorted ahead, so SAP can iteratre easily 
		std::vector<ObjectHandle> handles;
		std::unordered_map<ObjectHandle, int> handleObjIdxMap;

		ObjectHandle AddObject(const Object& obj)
		{
			static uint64_t counter = 0;
			objs.push_back(obj);
			handles.push_back(ObjectHandle{ .handle = counter++ });
			handleObjIdxMap[handles.back()] = objs.size() - 1;

			return handles.back();
		}
		void RemoveObject(const ObjectHandle& handle)
		{
			auto idxReplace = handleObjIdxMap[handle];
			if (idxReplace < objs.size() - 1)
			{
				objs[idxReplace] = objs.back();
				handles[idxReplace] = handles.back();
				handleObjIdxMap[handles[idxReplace]] = idxReplace;
			}
			objs.pop_back();
			handles.pop_back();
			handleObjIdxMap.erase(handle);
		}
	};
}