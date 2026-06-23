#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"

layout(local_size_x = 4, local_size_y = 2, local_size_z = 4) in;

shared uint localOccupancy[32];

void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz) * ivec3(2, 4, 2);
    
    uint brick_index = getBrickIndex(pos);
    uint id = gl_LocalInvocationIndex;      
    uint occupied = 0;
    
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 4; ++y) {
            for (int z = 0; z < 2; ++z) {
                ivec3 world_pos = pos + ivec3(x, y, z);
                if (!isValidPos(world_pos)) continue;       
                
                uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME
                                    + getVoxelIndexInBrick(world_pos); 
                
                if(isVoxelDynamic(getPreviousVoxel(voxel_index))) {
                    setPreviousVoxel(voxel_index, createAirVoxel());
                }
                
                occupied += isVoxelAir(getVoxel(voxel_index)) ? 0 : 1;
            }
        }
    }  

    localOccupancy[id] = occupied;
    barrier();
    
    if (id == 0u) {
        uint count = 0;
        for (uint i = 0u; i < 32u; ++i) {
            count += localOccupancy[i];
        }

        voxelBricks[brick_index].occupancy_count = count;
    }
}
