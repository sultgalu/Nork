#pragma once

namespace Nork {
struct CallbackStorage {
	virtual void removeCallbackReference(uint32_t cbId) = 0;
};
struct Disposable {
	CallbackStorage* cbStorage;
	uint32_t cbId;
	void Invalidate() {
		cbStorage = nullptr;
	}
	~Disposable() {
		if (cbStorage) {
			cbStorage->removeCallbackReference(cbId);
		}
	}
};
struct LifeCycle {
	std::vector<std::shared_ptr<Disposable>> disposables;
};
}