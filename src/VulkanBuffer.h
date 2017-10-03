#ifndef VULKAN_BUFFER_H_
#define VULKAN_BUFFER_H_

#include <vulkan/vulkan.h>
#include "VulkanAllocation.h"

namespace Vulkan {

	enum BufferMapType {
		Read,
		Write
	};

	enum BufferUsage {
		Static,
		Dynamic
	};

	class VulkanBuffer {
	public:
		VulkanBuffer();
		virtual ~VulkanBuffer();
		int GetSize() const { return mSize; }
		int GetAllocatedSize() const { return((mSize & ~MappedBit) + 15) & ~15; }
		int GetOffset() const { return mOffset & ~OwnerBit; }
		VkBuffer Buffer() const { return hBuffer; }
	protected:
		bool IsOwner() const { return (mOffset & OwnerBit) != 0; }
		bool IsMapped() const { return (mSize & MappedBit) != 0; }
		void SetMapped() const { const_cast<int &>(mSize) |= MappedBit; }
		void SetUnmapped() const { const_cast<int &>(mSize) &= ~MappedBit; }
		VkBuffer hBuffer;
		int mSize;
		int mOffset;
		BufferUsage mUsage;
		VulkanAllocation mAllocation;

		static const int MappedBit = 1 << (4 * 8 - 1);
		static const int OwnerBit = 1 << (4 * 8 - 1);
	};

	class VertexBuffer : public VulkanBuffer {

	public:
		VertexBuffer();
		~VertexBuffer();
		bool AllocateBufferObject(const void* data, int allocSize, BufferUsage usage);
		void FreeBufferObject();
		void Reference(const VertexBuffer& other);
		void Reference(const VertexBuffer& other, int refOffset, int refSize);
		void Update(const void* data, int size, int offset = 0) const;
		void* Map();
		void Unmap();
	private:
		void ClearWithoutFreeing();
	};

	class IndexBuffer : public VulkanBuffer
	{
	public: 
		IndexBuffer();
		~IndexBuffer();
		bool AllocateBufferObject(const void* data, int allocSize, BufferUsage usage);
		void FreeBufferObject();
		void Reference(const IndexBuffer& other);
		void Reference(const IndexBuffer& other, int refOffset, int refSize);
		void Update(const void* data, int size, int offset = 0) const;
		void* Map();
		void Unmap();
	private:
		void ClearWithoutFreeing();
	};

	class UniformBuffer : public VulkanBuffer {
	public:
		UniformBuffer();
		~UniformBuffer();
		bool AllocateBufferObject(const void* data, int allocSize, BufferUsage usage);
		void FreeBufferObject();
		void Reference(const UniformBuffer& other);
		void Reference(const UniformBuffer& other, int refOffset, int refSize);
		void Update(const void* data, int size, int offset = 0) const;
		void* Map();
		void Unmap();
	private:
		void ClearWithoutFreeing();
	};
}
#endif
