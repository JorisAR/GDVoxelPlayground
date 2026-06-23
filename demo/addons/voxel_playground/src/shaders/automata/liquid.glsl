#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"


ivec3 directions[4] = ivec3[](
    ivec3(-1, 0, 0), // left
    ivec3(0, 0, 1),  // forward
    ivec3(1, 0, 0),  // right
    ivec3(0, 0, -1)  // backward
);

//last 3 bits store direction, we just check all 4 bits for now
uint getVoxelDirectionID(Voxel voxel) {
    return voxel.data & 0xFu;
}

void setVoxelDirectionID(inout Voxel voxel, uint directionID) {
    voxel.data = (voxel.data & ~0xFu) | (directionID & 0xFu);
}

bool move_water(ivec3 pos, ivec3 dir, uint brick_index, uint voxel_index, uint new_voxel_data, bool swap_liquids) {
    ivec3 newPos = pos + dir;
    if (isValidPos(newPos)) {
        uint new_brick_index = getBrickIndex(newPos);
        uint new_voxel_index = voxelBricks[new_brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(newPos); 
        Voxel previous_voxel = getPreviousVoxel(new_voxel_index);
        if (isVoxelAir(previous_voxel) || (swap_liquids && isVoxelLiquid(previous_voxel))) {
            uint expected = previous_voxel.data;
            uint original;
            if (voxelWorldProperties.frame % 2 == 0)
                original = atomicCompSwap(voxelData[new_voxel_index].data, expected, new_voxel_data);
            else
                original = atomicCompSwap(voxelData2[new_voxel_index].data, expected, new_voxel_data);
            if (original == expected) {
                setVoxel(voxel_index, swap_liquids ? previous_voxel : createAirVoxel());
                return true;
            }
        }
        return false;
    }
    return true;
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if (!isValidPos(pos)) return;
    
    uint brick_index = getBrickIndex(pos);
    uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos); 

    Voxel voxel_value = getPreviousVoxel(voxel_index);
    if(isVoxelLiquid(voxel_value)) {
        if(!move_water(pos, ivec3(0, -1, 0), brick_index, voxel_index, voxel_value.data, false))
        {
            uvec4 random_value = hash(uvec4(pos, voxelWorldProperties.frame));
            uint randVal = random_value.x;  
            uint randPercent = randVal % 100u;  
            uint index = 0;
            uint previous_index = getVoxelDirectionID(voxel_value);

            if( previous_index == 0) {
                index = randVal % 4u;
                setVoxelDirectionID(voxel_value, index + 1);
            }
            else {
                --previous_index;
                if (randPercent < 80u) {
                    index = previous_index;
                } else if (randPercent < 90u) {
                    index = (previous_index + 1u) % 4u;
                } else {
                    index = (previous_index + 3u) % 4u;
                }
            }            

            ivec3 dir = directions[index];

            if(!move_water(pos, dir, brick_index, voxel_index, voxel_value.data, false)) {
                // setVoxelDirectionID(voxel_value, previous_index);
                setVoxelDirectionID(voxel_value, 0);
                setVoxel(voxel_index, voxel_value);
            }
                
        }
    }
    if(isVoxelType(voxel_value, VOXEL_TYPE_SAND)) {
        if(!move_water(pos, ivec3(0, -1, 0), brick_index, voxel_index, voxel_value.data, true)){
            uvec4 random_value = hash(uvec4(pos, voxelWorldProperties.frame));            
            ivec3 dir = directions[random_value.x % 4u] + ivec3(0, -1, 0);

            if(!move_water(pos, dir, brick_index, voxel_index, voxel_value.data, true)) {
                setVoxel(voxel_index, voxel_value);
            }
        }
            
    }
}
