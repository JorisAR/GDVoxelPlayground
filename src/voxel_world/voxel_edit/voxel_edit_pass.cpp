#include "voxel_edit_pass.h"
#include "voxel_world/shader_paths.h"

using namespace godot;

VoxelEditPass::VoxelEditPass(const String &edit_shader_path, RenderingDevice *rd,
                             const VoxelWorldRIDs &voxel_world_rids, const Vector3i size)
    : _ray_cast(rd, VoxelShaders::EDIT_RAYCAST), _edit(rd, edit_shader_path), _size(size)
{
    _edit_params = {
        Vector4(1, 1, 1, 1),  // camera_origin
        Vector4(0, 0, -1, 0), // camera_direction
        Vector4(0, 0, 0, -1), // hit_position (w < 0 means no hit)
        0.1f,                 // near
        100.0f,               // far
        1.0f,                 // radius
        0                     // value
    };
    _edit_params_buffer = GpuResource::storage_buffer_from(rd, _edit_params);

    voxel_world_rids.add_voxel_buffers(_ray_cast);
    _ray_cast.bind_storage_buffer(1, 0, _edit_params_buffer.rid());

    voxel_world_rids.add_voxel_buffers(_edit);
    _edit.bind_storage_buffer(1, 0, _edit_params_buffer.rid());
}

void VoxelEditPass::edit_using_raycast(const Vector3 &camera_origin, const Vector3 &camera_direction,
                                       const float radius, const float range, const int value)
{
    if (!_ray_cast.is_valid() || !_edit.is_valid() || radius <= 0.0f)
        return;

    _edit_params.camera_origin = Vector4(camera_origin.x, camera_origin.y, camera_origin.z, 1.0f);
    _edit_params.camera_direction =
        Vector4(camera_direction.x, camera_direction.y, camera_direction.z, 0.0f).normalized();
    _edit_params.hit_position = Vector4(0, 0, 0, -1);
    _edit_params.near = 0.1f;
    _edit_params.far = range;
    _edit_params.radius = radius;
    _edit_params.value = static_cast<unsigned int>(value);
    _edit_params_buffer.update(_edit_params);

    // The ray cast writes the hit position into the params buffer, and the edit shader reads it from there,
    // so everything stays on the GPU: no readback / stall.
    _ray_cast.dispatch(Vector3i(1, 1, 1));

    const int extent = static_cast<int>(std::ceil(2.0f * radius));
    _edit.dispatch(ComputeKernel::group_count(Vector3i(extent, extent, extent), Vector3i(8, 8, 8)));
}
