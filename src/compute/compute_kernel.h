#ifndef VOXEL_COMPUTE_KERNEL_H
#define VOXEL_COMPUTE_KERNEL_H

#include <cstdint>
#include <map>
#include <vector>

#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include "gpu_resource.h"

namespace godot
{

// A compute pipeline built from an *imported* .glsl shader (RDShaderFile), plus its uniform bindings.
//
// - Shaders are never compiled at runtime: the SPIR-V produced by Godot's importer is used directly,
//   so this also works in exported projects. Use the `#[versions]` section of the shader for variants.
// - The kernel never owns the resources bound to it; bind RIDs owned elsewhere (e.g. by a GpuResource).
// - Uniform sets are rebuilt lazily after a binding changes, and old sets are freed.
class ComputeKernel
{
  public:
    ComputeKernel(RenderingDevice *rd, const String &shader_path, const StringName &version = StringName());
    ~ComputeKernel();

    ComputeKernel(const ComputeKernel &) = delete;
    ComputeKernel &operator=(const ComputeKernel &) = delete;

    bool is_valid() const
    {
        return _pipeline.is_valid();
    }

    // Adds or replaces the binding at (set, binding).
    void bind(uint32_t set, uint32_t binding, RenderingDevice::UniformType type, const RID &rid);
    void bind_storage_buffer(uint32_t set, uint32_t binding, const RID &rid)
    {
        bind(set, binding, RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER, rid);
    }
    void bind_image(uint32_t set, uint32_t binding, const RID &rid)
    {
        bind(set, binding, RenderingDevice::UNIFORM_TYPE_IMAGE, rid);
    }

    // Push constants are padded to a multiple of 16 bytes as Vulkan requires.
    void set_push_constant(const PackedByteArray &data);
    template <typename T> void set_push_constant(const T &value)
    {
        set_push_constant(to_bytes(value));
    }

    // Records a dispatch into its own compute list. Safe to call on the main RenderingDevice.
    void dispatch(const Vector3i &groups);
    // Records a dispatch into an already open compute list.
    void dispatch(int64_t compute_list, const Vector3i &groups);

    // Number of work groups needed to cover `threads` with groups of `group_size`.
    static Vector3i group_count(const Vector3i &threads, const Vector3i &group_size);

  private:
    struct Binding
    {
        RenderingDevice::UniformType type;
        RID rid;
    };

    bool _prepare();
    void _free_uniform_sets();

    RenderingDevice *_rd = nullptr;
    String _path;
    RID _shader;
    RID _pipeline;
    std::map<uint32_t, std::map<uint32_t, Binding>> _bindings; // set -> binding -> resource
    std::map<uint32_t, RID> _uniform_sets;
    PackedByteArray _push_constant;
    bool _dirty = true;
};

} // namespace godot

#endif // VOXEL_COMPUTE_KERNEL_H
