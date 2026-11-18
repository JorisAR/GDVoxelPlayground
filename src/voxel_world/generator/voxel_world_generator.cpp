#include "voxel_world_generator.h"
#include "voxel_world/shader_paths.h"

using namespace godot;

void VoxelWorldGenerator::initialize_brick_grid(RenderingDevice *rd, VoxelWorldRIDs &voxel_world_rids,
                                               const VoxelWorldProperties &properties)
{
    ComputeKernel kernel(rd, VoxelShaders::INITIALIZE_BRICK_GRID);
    voxel_world_rids.add_voxel_buffers(kernel);

    const Vector3i brick_grid_size(properties.brick_grid_size.x, properties.brick_grid_size.y,
                                   properties.brick_grid_size.z);
    kernel.dispatch(ComputeKernel::group_count(brick_grid_size, Vector3i(8, 8, 8)));
}
