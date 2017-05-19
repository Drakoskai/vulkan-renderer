#ifndef VKCOM_H_
#define VKCOM_H_
#include <vulkan/vulkan.h>
#include <functional>

namespace Vulkan {
	template <typename T>
	class VkCom {
	public:
		VkCom();

		VkCom(std::function<void(T, VkAllocationCallbacks*)> deletef);

		VkCom(const VkCom<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef);

		VkCom(const VkCom<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef);

		~VkCom();

		const T* operator &() const;

		T* replace();

		operator T() const;

		void operator=(T rhs);

		template<typename V>
		bool operator==(V rhs);

	private:
		T object{ VK_NULL_HANDLE };
		std::function<void(T)> deleter;

		void cleanup();
	};

	template <typename T>
	VkCom<T>::VkCom() : VkCom([](T, VkAllocationCallbacks*) {
	}) { }

	template <typename T>
	VkCom<T>::VkCom(std::function<void(T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [=](T obj) { deletef(obj, nullptr); };
	}

	template <typename T>
	VkCom<T>::VkCom(const VkCom<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
	}

	template <typename T>
	VkCom<T>::VkCom(const VkCom<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef) {
		this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
	}

	template <typename T>
	VkCom<T>::~VkCom() {
		cleanup();
	}

	template <typename T>
	const T* VkCom<T>::operator&() const {
		return &object;
	}

	template <typename T>
	T* VkCom<T>::replace() {
		cleanup();
		return &object;
	}

	template <typename T>
	VkCom<T>::operator T() const {
		return object;
	}

	template <typename T>
	void VkCom<T>::operator=(T rhs) {
		if(rhs != object) {
			cleanup();
			object = rhs;
		}
	}

	template <typename T>
	template <typename V>
	bool VkCom<T>::operator==(V rhs) {
		return object == T(rhs);
	}

	template <typename T>
	void VkCom<T>::cleanup() {
		if(object != VK_NULL_HANDLE) {
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
}
#endif
