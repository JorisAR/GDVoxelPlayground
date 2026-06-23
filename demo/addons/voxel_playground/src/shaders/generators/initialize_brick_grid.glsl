#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"


layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if (pos.x >= voxelWorldProperties.brick_grid_size.x || pos.y >= voxelWorldProperties.brick_grid_size.y || pos.z >= voxelWorldProperties.brick_grid_size.z) return;

    int brick_index = pos.x + pos.y * voxelWorldProperties.brick_grid_size.x + pos.z * voxelWorldProperties.brick_grid_size.x * voxelWorldProperties.brick_grid_size.y;
    voxelBricks[brick_index].voxel_data_pointer = brick_index;
    voxelBricks[brick_index].occupancy_count = 0;
}