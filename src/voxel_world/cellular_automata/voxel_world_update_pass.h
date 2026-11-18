#ifndef VOXEL_WORLD_UPDATE_PASS_H
#define VOXEL_WORLD_UPDATE_PASS_H

#include <godot_cpp/classes/rendering_device.hpp>

#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelWorldUpdatePass
{
  public:
    VoxelWorldUpdatePass(const String &liquid_shader_path, RenderingDevice *rd,
                         const VoxelWorldRIDs &voxel_world_rids, const Vector3i size);

    void update(float delta);

  private:
    ComputeKernel _liquid;
    ComputeKernel _freeze_lava;
    ComputeKernel _cleanup;
    Vector3i _size;
};

#endif // VOXEL_WORLD_UPDATE_PASS_H
