#include "voxel_camera.h"
#include "utility/utils.h"
#include "voxel_world/shader_paths.h"
#include <godot_cpp/classes/viewport.hpp>

void VoxelCamera::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_fov"), &VoxelCamera::get_fov);
    ClassDB::bind_method(D_METHOD("set_fov", "value"), &VoxelCamera::set_fov);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fov"), "set_fov", "get_fov");

    ClassDB::bind_method(D_METHOD("get_voxel_world"), &VoxelCamera::get_voxel_world);
    ClassDB::bind_method(D_METHOD("set_voxel_world", "value"), &VoxelCamera::set_voxel_world);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "voxel_world", PROPERTY_HINT_NODE_TYPE, "VoxelWorld"),
                 "set_voxel_world", "get_voxel_world");

    ClassDB::bind_method(D_METHOD("get_output_texture"), &VoxelCamera::get_output_texture);
    ClassDB::bind_method(D_METHOD("set_output_texture", "value"), &VoxelCamera::set_output_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "output_texture", PROPERTY_HINT_NODE_TYPE, "TextureRect"),
                 "set_output_texture", "get_output_texture");
}

void VoxelCamera::_notification(int p_what)
{
    if (godot::Engine::get_singleton()->is_editor_hint())
    {
        return;
    }
    switch (p_what)
    {
    case NOTIFICATION_ENTER_TREE: {
        set_process_internal(true);

        break;
    }
    case NOTIFICATION_EXIT_TREE: {
        set_process_internal(false);

        break;
    }
    case NOTIFICATION_PREDELETE: {
        release();
        break;
    }
    case NOTIFICATION_INTERNAL_PROCESS: {
        render();
        break;
    }
    }
}

float VoxelCamera::get_fov() const
{
    return fov;
}

void VoxelCamera::set_fov(float value)
{
    fov = value;
}

// int VoxelCamera::get_num_bounces() const
// {
//     return num_bounces;
// }

// void VoxelCamera::set_num_bounces(int value)
// {
//     num_bounces = value;
// }

TextureRect *VoxelCamera::get_output_texture() const
{
    return Object::cast_to<TextureRect>(ObjectDB::get_instance(_output_texture_rect_id));
}

void VoxelCamera::set_output_texture(TextureRect *value)
{
    _output_texture_rect_id = value ? ObjectID(value->get_instance_id()) : ObjectID();
}

VoxelWorld *VoxelCamera::get_voxel_world() const
{
    return Object::cast_to<VoxelWorld>(ObjectDB::get_instance(_voxel_world_id));
}

void VoxelCamera::set_voxel_world(VoxelWorld *value)
{
    _voxel_world_id = value ? ObjectID(value->get_instance_id()) : ObjectID();
}

VoxelCamera::~VoxelCamera()
{
    release();
}

bool VoxelCamera::init(VoxelWorld *world)
{
    release();

    _rd = RenderingServer::get_singleton()->get_rendering_device();
    if (_rd == nullptr)
    {
        UtilityFunctions::printerr("VoxelCamera: requires a RenderingDevice-based renderer (Forward+ or Mobile).");
        return false;
    }
    if (get_output_texture() == nullptr)
    {
        UtilityFunctions::printerr("VoxelCamera: No output texture set.");
        return false;
    }

    _kernel = std::make_unique<ComputeKernel>(_rd, VoxelShaders::RENDERER);
    if (!_kernel->is_valid())
    {
        _kernel.reset();
        return false;
    }

    _render_parameters_buffer = GpuResource::storage_buffer_from(_rd, render_parameters);
    _camera_parameters_buffer = GpuResource::storage_buffer_from(_rd, camera_parameters);
    _kernel->bind_storage_buffer(1, 2, _render_parameters_buffer.rid());
    _kernel->bind_storage_buffer(1, 3, _camera_parameters_buffer.rid());

    output_texture.instantiate();
    get_output_texture()->set_texture(output_texture);
    camera_parameters.frame_index = 0;
    return true;
}

void VoxelCamera::release()
{
    if (output_texture.is_valid())
        output_texture->set_texture_rd_rid(RID());
    _kernel.reset();
    _output_texture.reset();
    _depth_texture.reset();
    _render_parameters_buffer.reset();
    _camera_parameters_buffer.reset();
    _resolution = Vector2i();
}

bool VoxelCamera::resize_targets(const Vector2i &resolution)
{
    Ref<RDTextureView> view;
    view.instantiate();

    Ref<RDTextureFormat> format;
    format.instantiate();
    format->set_width(resolution.x);
    format->set_height(resolution.y);

    // Must match the `rgba8` image declared in voxel_renderer.glsl.
    format->set_format(RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM);
    format->set_usage_bits(RenderingDevice::TEXTURE_USAGE_STORAGE_BIT | RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT);
    GpuResource color(_rd, _rd->texture_create(format, view));

    // Must match the `r32f` image declared in voxel_renderer.glsl.
    format->set_format(RenderingDevice::DATA_FORMAT_R32_SFLOAT);
    GpuResource depth(_rd, _rd->texture_create(format, view));

    if (!color.is_valid() || !depth.is_valid())
    {
        UtilityFunctions::printerr("VoxelCamera: failed to create render targets of size ", resolution);
        return false;
    }

    // Point the displayed texture at the new target before the old one is freed.
    output_texture->set_texture_rd_rid(color.rid());
    _output_texture = std::move(color);
    _depth_texture = std::move(depth);
    _kernel->bind_image(1, 0, _output_texture.rid());
    _kernel->bind_image(1, 1, _depth_texture.rid());

    render_parameters.width = resolution.x;
    render_parameters.height = resolution.y;
    render_parameters.fov = fov;
    _render_parameters_buffer.update(render_parameters);

    _resolution = resolution;
    return true;
}

void VoxelCamera::render()
{
    VoxelWorld *world = get_voxel_world();
    if (world == nullptr || !world->is_initialized())
        return; // the world may still be generating, or may have been freed

    if (!_kernel)
    {
        if (_init_failed)
            return;
        if (!init(world))
        {
            _init_failed = true; // don't retry (and spam errors) every frame
            return;
        }
    }

    // Rebinding is a no-op unless the world recreated its buffers.
    world->get_voxel_world_rids().add_voxel_buffers(*_kernel);

    const Vector2i resolution = get_viewport()->get_visible_rect().size;
    if (resolution.x <= 0 || resolution.y <= 0)
        return; // minimised
    if (resolution != _resolution && !resize_targets(resolution))
        return;

    const Projection projection = Projection::create_perspective(
        fov, static_cast<real_t>(resolution.x) / resolution.y, near_plane, far_plane, false);
    const Transform3D transform = get_global_transform();
    const Projection VP = projection * Projection(transform.affine_inverse());
    const Vector3 camera_position = transform.get_origin();

    Utils::projection_to_float(camera_parameters.vp, VP);
    Utils::projection_to_float(camera_parameters.ivp, VP.inverse());
    camera_parameters.cameraPosition = Vector4(camera_position.x, camera_position.y, camera_position.z, 1.0f);
    camera_parameters.frame_index++;
    camera_parameters.nearPlane = near_plane;
    camera_parameters.farPlane = far_plane;
    _camera_parameters_buffer.update(camera_parameters);

    // Must match local_size in voxel_renderer.glsl.
    _kernel->dispatch(ComputeKernel::group_count(Vector3i(resolution.x, resolution.y, 1), Vector3i(8, 8, 1)));
}
