#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if (!isValidPos(pos)) return;
    
    uint brick_index = getBrickIndex(pos);
    uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos); 
    
    Voxel voxel_value = getVoxel(voxel_index);
    if(isVoxelType(voxel_value, VOXEL_TYPE_LAVA)) {
        if( isVoxelType(getVoxel(posToIndex(pos + ivec3(0,1,0))), VOXEL_TYPE_WATER)
        || isVoxelType(getVoxel(posToIndex(pos + ivec3(0,0,1))), VOXEL_TYPE_WATER)
        || isVoxelType(getVoxel(posToIndex(pos + ivec3(0,0,-1))), VOXEL_TYPE_WATER)
        || isVoxelType(getVoxel(posToIndex(pos + ivec3(1,0,0))), VOXEL_TYPE_WATER)
        || isVoxelType(getVoxel(posToIndex(pos + ivec3(-1,0,0))), VOXEL_TYPE_WATER)
        || isVoxelType(getVoxel(posToIndex(pos + ivec3(0,-1,0))), VOXEL_TYPE_WATER))
        {
            setBothVoxelBuffers(voxel_index, createRockVoxel(pos));
        }
    }
}
