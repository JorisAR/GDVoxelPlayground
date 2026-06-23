#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"

float distanceToBoundary(vec3 pos, vec3 halfBoxSize) {
    vec3 d = abs(pos - halfBoxSize) - halfBoxSize;
    float dist = length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);

    return saturate(0.01 * (1 - dist));
}

float terrainDensity(ivec3 pos) {
    if (!isValidPos(pos)) return 0;
    uint brick_index = getBrickIndex(pos);
    uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos);     

    // Calculate the distance from the center of the sphere
    vec3 world_pos = pos * voxelWorldProperties.scale;

    // Set the voxel data based on the distance
    float noise = fbm(world_pos * 0.1);
    float dist_to_boundary = distanceToBoundary(pos, voxelWorldProperties.grid_size.xyz * 0.5); 
    float top = 200.f;
    dist_to_boundary = min(pow((top - pos.y) / top, 3.0) * top, dist_to_boundary);

    if (pos.y < top && 0.4 < noise * dist_to_boundary) {
        return 1.0f;
    }
    return 0.0f;
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    if (!isValidPos(pos)) return;

    uint brick_index = getBrickIndex(pos);
    uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos);     

    float v0density = terrainDensity(pos);
    float v1density = terrainDensity(pos + ivec3(0,2,0));

    if (v0density > 0.5) { //  
        if(v1density > 0.5)
            setBothVoxelBuffers(voxel_index, createRockVoxel(pos));
        else
            setBothVoxelBuffers(voxel_index, createGrassVoxel(pos));

        // atomicAdd(voxelBricks[brick_index].occupancy_count, 1);
    } 
    else if(pos.y > 200) {
        // Create a sky voxel at the top of the world
        // setBothVoxelBuffers(voxel_index, createWaterVoxel());
    }
    else {        
        setBothVoxelBuffers(voxel_index, createAirVoxel());
    }
}