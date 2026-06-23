#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"

layout(std430, set = 1, binding = 0) restrict buffer Params {
    ivec4 bounds_min; //min corner in voxels
    ivec4 bounds_size; //size in voxels
} params;

layout(std430, set = 1, binding = 1) restrict buffer Result {
    uint data[];
} result;

uint posToResultIndex(ivec3 pos) {
    return uint(pos.x + pos.y * params.bounds_size.x + pos.z * params.bounds_size.x * params.bounds_size.y);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if(pos.x >= params.bounds_size.x || pos.y >= params.bounds_size.y || pos.z >= params.bounds_size.z) {
        return; //out of bounds
    }
    ivec3 world_pos = params.bounds_min.xyz + pos;

    uint index = posToResultIndex(pos);
    uint bit_index = index % 32;
    uint data_index = index / 32;

    Voxel voxel_value = getVoxel(posToIndex(world_pos));

    if (!isValidPos(world_pos) || !isVoxelSolid(voxel_value)) {
        atomicAnd(result.data[data_index], ~(1u << bit_index));
    } else {
        atomicOr(result.data[data_index], 1u << bit_index);
    }

}
