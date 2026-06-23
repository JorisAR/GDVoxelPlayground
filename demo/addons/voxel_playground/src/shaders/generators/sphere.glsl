#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"


layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if (!isValidPos(pos)) return;

    // Calculate the distance from the center of the sphere
    vec3 center = voxelWorldProperties.grid_size.xyz * 0.5;
    float radius = min(voxelWorldProperties.grid_size.x, min(voxelWorldProperties.grid_size.y, voxelWorldProperties.grid_size.z)) * 0.5f;
    float d = length(vec3(pos) - center);

    // Set the voxel data based on the distance
    uint brick_index = getBrickIndex(pos);
    uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos);     

    if (d < radius) { // Inside the sphere
        atomicAdd(voxelBricks[brick_index].occupancy_count, 1);
        voxelData[voxel_index].data = 1; 
    } else { // Outside the sphere
        voxelData[voxel_index].data = 0; 
    }
}