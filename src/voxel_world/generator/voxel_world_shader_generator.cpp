#include "voxel_world_shader_generator.h"

using namespace godot;

void VoxelWorldShaderGenerator::generate(RenderingDevice *rd, VoxelWorldRIDs &voxel_world_rids,
                                         const VoxelWorldProperties &properties)
{
    ComputeKernel kernel(rd, shader_path);
    if (!kernel.is_valid())
        return;
    voxel_world_rids.add_voxel_buffers(kernel);

    const Vector3i size(properties.grid_size.x, properties.grid_size.y, properties.grid_size.z);
    kernel.dispatch(ComputeKernel::group_count(size, Vector3i(8, 8, 8)));
}
