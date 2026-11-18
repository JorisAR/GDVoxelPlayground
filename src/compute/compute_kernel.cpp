#include "compute_kernel.h"

#include <godot_cpp/classes/rd_shader_file.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ComputeKernel::ComputeKernel(RenderingDevice *rd, const String &shader_path, const StringName &version)
    : _rd(rd), _path(shader_path)
{
    if (_rd == nullptr)
    {
        UtilityFunctions::printerr("ComputeKernel: no RenderingDevice available for ", shader_path);
        return;
    }

    Ref<RDShaderFile> shader_file = ResourceLoader::get_singleton()->load(shader_path);
    if (shader_file.is_null())
    {
        UtilityFunctions::printerr("ComputeKernel: failed to load imported shader ", shader_path);
        return;
    }

    Ref<RDShaderSPIRV> spirv = shader_file->get_spirv(version);
    if (spirv.is_null())
    {
        UtilityFunctions::printerr("ComputeKernel: shader ", shader_path, " has no version '", version, "'");
        return;
    }

    const String error = spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE);
    if (!error.is_empty())
    {
        UtilityFunctions::printerr("ComputeKernel: ", shader_path, " failed to compile:\n", error);
        return;
    }

    _shader = _rd->shader_create_from_spirv(spirv, shader_path.get_file());
    if (!_shader.is_valid())
    {
        UtilityFunctions::printerr("ComputeKernel: failed to create shader from ", shader_path);
        return;
    }

    _pipeline = _rd->compute_pipeline_create(_shader);
    if (!_pipeline.is_valid())
        UtilityFunctions::printerr("ComputeKernel: failed to create compute pipeline for ", shader_path);
}

ComputeKernel::~ComputeKernel()
{
    if (_rd == nullptr)
        return;
    _free_uniform_sets();
    // Freeing the shader also frees the pipeline that depends on it.
    if (_pipeline.is_valid())
        _rd->free_rid(_pipeline);
    if (_shader.is_valid())
        _rd->free_rid(_shader);
}

void ComputeKernel::bind(uint32_t set, uint32_t binding, RenderingDevice::UniformType type, const RID &rid)
{
    Binding &b = _bindings[set][binding];
    if (b.rid == rid && b.type == type)
        return;
    b.type = type;
    b.rid = rid;
    _dirty = true;
}

void ComputeKernel::set_push_constant(const PackedByteArray &data)
{
    _push_constant = data;
    const int64_t padded = (data.size() + 15) & ~int64_t(15);
    if (padded != data.size())
    {
        _push_constant.resize(padded);
        std::memset(_push_constant.ptrw() + data.size(), 0, padded - data.size());
    }
}

void ComputeKernel::_free_uniform_sets()
{
    for (auto &pair : _uniform_sets)
    {
        // A set is freed automatically by Godot when one of its resources is freed, so check first.
        if (pair.second.is_valid() && _rd->uniform_set_is_valid(pair.second))
            _rd->free_rid(pair.second);
    }
    _uniform_sets.clear();
}

bool ComputeKernel::_prepare()
{
    if (!is_valid())
        return false;

    // Rebuild if a binding changed, or if Godot invalidated a set because a bound resource was freed.
    if (!_dirty)
    {
        for (const auto &pair : _uniform_sets)
        {
            if (!_rd->uniform_set_is_valid(pair.second))
            {
                _dirty = true;
                break;
            }
        }
    }
    if (!_dirty)
        return true;

    _free_uniform_sets();
    for (const auto &set_pair : _bindings)
    {
        TypedArray<RDUniform> uniforms;
        for (const auto &binding_pair : set_pair.second)
        {
            if (!binding_pair.second.rid.is_valid())
            {
                UtilityFunctions::printerr("ComputeKernel: ", _path, " has an invalid resource at set ",
                                           set_pair.first, " binding ", binding_pair.first);
                return false;
            }
            Ref<RDUniform> uniform;
            uniform.instantiate();
            uniform->set_binding(binding_pair.first);
            uniform->set_uniform_type(binding_pair.second.type);
            uniform->add_id(binding_pair.second.rid);
            uniforms.push_back(uniform);
        }

        RID set_rid = _rd->uniform_set_create(uniforms, _shader, set_pair.first);
        if (!set_rid.is_valid())
        {
            UtilityFunctions::printerr("ComputeKernel: failed to create uniform set ", set_pair.first, " for ",
                                       _path);
            _free_uniform_sets();
            return false;
        }
        _uniform_sets[set_pair.first] = set_rid;
    }
    _dirty = false;
    return true;
}

void ComputeKernel::dispatch(const Vector3i &groups)
{
    if (!_prepare())
        return;
    const int64_t list = _rd->compute_list_begin();
    dispatch(list, groups);
    _rd->compute_list_end();
}

void ComputeKernel::dispatch(int64_t compute_list, const Vector3i &groups)
{
    if (!_prepare())
        return;
    if (groups.x <= 0 || groups.y <= 0 || groups.z <= 0)
        return;

    _rd->compute_list_bind_compute_pipeline(compute_list, _pipeline);
    for (const auto &pair : _uniform_sets)
        _rd->compute_list_bind_uniform_set(compute_list, pair.second, pair.first);
    if (!_push_constant.is_empty())
        _rd->compute_list_set_push_constant(compute_list, _push_constant,
                                            static_cast<uint32_t>(_push_constant.size()));
    _rd->compute_list_dispatch(compute_list, groups.x, groups.y, groups.z);
}

Vector3i ComputeKernel::group_count(const Vector3i &threads, const Vector3i &group_size)
{
    auto div = [](int32_t a, int32_t b) { return a <= 0 ? 1 : (a + b - 1) / b; };
    return Vector3i(div(threads.x, group_size.x), div(threads.y, group_size.y), div(threads.z, group_size.z));
}
