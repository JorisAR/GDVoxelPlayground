#ifndef VOXEL_GPU_RESOURCE_H
#define VOXEL_GPU_RESOURCE_H

#include <cstring>
#include <type_traits>

#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot
{

// Copies a trivially-copyable struct into a byte array for upload to the GPU.
template <typename T> inline PackedByteArray to_bytes(const T &value)
{
    static_assert(std::is_trivially_copyable<T>::value, "to_bytes requires a trivially copyable type");
    PackedByteArray bytes;
    bytes.resize(sizeof(T));
    std::memcpy(bytes.ptrw(), &value, sizeof(T));
    return bytes;
}

// Owns a single RenderingDevice RID (buffer, texture, sampler, ...) and frees it on destruction.
// Move-only, so ownership is always explicit.
class GpuResource
{
  public:
    GpuResource() = default;
    GpuResource(RenderingDevice *rd, const RID &rid) : _rd(rd), _rid(rid)
    {
    }
    ~GpuResource()
    {
        reset();
    }

    GpuResource(const GpuResource &) = delete;
    GpuResource &operator=(const GpuResource &) = delete;

    GpuResource(GpuResource &&other) noexcept : _rd(other._rd), _rid(other._rid)
    {
        other._rd = nullptr;
        other._rid = RID();
    }
    GpuResource &operator=(GpuResource &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            _rd = other._rd;
            _rid = other._rid;
            other._rd = nullptr;
            other._rid = RID();
        }
        return *this;
    }

    void reset()
    {
        if (_rd != nullptr && _rid.is_valid())
            _rd->free_rid(_rid);
        _rid = RID();
        _rd = nullptr;
    }

    const RID &rid() const
    {
        return _rid;
    }
    bool is_valid() const
    {
        return _rd != nullptr && _rid.is_valid();
    }

    // Creates a zero-initialised storage buffer without allocating a CPU-side copy.
    static GpuResource storage_buffer(RenderingDevice *rd, uint32_t size_bytes)
    {
        RID rid = rd->storage_buffer_create(size_bytes);
        if (rid.is_valid())
            rd->buffer_clear(rid, 0, size_bytes);
        return GpuResource(rd, rid);
    }

    static GpuResource storage_buffer(RenderingDevice *rd, const PackedByteArray &data)
    {
        return GpuResource(rd, rd->storage_buffer_create(static_cast<uint32_t>(data.size()), data));
    }

    template <typename T> static GpuResource storage_buffer_from(RenderingDevice *rd, const T &value)
    {
        return storage_buffer(rd, to_bytes(value));
    }

    template <typename T> void update(const T &value)
    {
        update_bytes(to_bytes(value));
    }
    void update_bytes(const PackedByteArray &data, uint32_t offset = 0)
    {
        if (is_valid())
            _rd->buffer_update(_rid, offset, static_cast<uint32_t>(data.size()), data);
    }

  private:
    RenderingDevice *_rd = nullptr;
    RID _rid;
};

} // namespace godot

#endif // VOXEL_GPU_RESOURCE_H
