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
template<class T> class LiveData : public CallbackStorage {
public:
	const T& get() const { return value; }
	void set(const T& newValue) {
		auto oldValue = value;
		value = newValue;
		for (auto& [_, sub] : subscribers) {
			sub(oldValue, newValue);
		}
	}
	const T* operator->() {
		return &value;
	}
	const T& operator*() {
		return value;
	}
	LiveData() = default;
	LiveData(const T& v) {
		value = v;
	}
	LiveData(T&& v) {
		value = v;
	}
	LiveData<T>& operator=(const T& v) { set(v); return *this; }
	LiveData<T>& operator=(T&& v) { set(v); return *this; }

	~LiveData() {
		for (auto& d : disposables) {
			if (auto disp = d.lock()) {
				disp->Invalidate();
			}
		}
	}
	void subscribe(std::function<void(const T&, const T&)> cb, LifeCycle& lifeCycle) {
		subscribers[currId] = cb;
		auto disposable = std::make_shared<Disposable>(this, currId);
		disposables.push_back(disposable);
		lifeCycle.disposables.push_back(disposable);
		currId++;
	}
	void removeCallbackReference(uint32_t cbId) override {
		subscribers.erase(cbId + 1);
	}
private:
	T value;
	std::unordered_map<uint32_t, std::function<void(const T&, const T&)>> subscribers;
	std::vector<std::weak_ptr<Disposable>> disposables;
	uint32_t currId = 0;
};
}