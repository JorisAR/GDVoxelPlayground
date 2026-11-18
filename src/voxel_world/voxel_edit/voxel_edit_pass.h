#ifndef VOXEL_WORLD_EDIT_PASS_H
#define VOXEL_WORLD_EDIT_PASS_H

#include <godot_cpp/classes/rendering_device.hpp>

#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelEditPass
{
    struct VoxelEditParams // matches Params in raycast.glsl / sphere_edit.glsl
    {
        Vector4 camera_origin;
        Vector4 camera_direction;
        Vector4 hit_position;
        float near;
        float far;
        float radius;
        unsigned int value;
    };

  public:
    VoxelEditPass(const String &edit_shader_path, RenderingDevice *rd, const VoxelWorldRIDs &voxel_world_rids,
                  const Vector3i size);

    void edit_using_raycast(const Vector3 &camera_origin, const Vector3 &camera_direction, const float radius,
                            const float range, const int value);

  private:
    ComputeKernel _ray_cast;
    ComputeKernel _edit;
    Vector3i _size;

    VoxelEditParams _edit_params;
    GpuResource _edit_params_buffer;
};

#endif // VOXEL_WORLD_EDIT_PASS_H
