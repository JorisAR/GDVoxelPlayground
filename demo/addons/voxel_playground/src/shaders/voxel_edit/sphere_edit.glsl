#[compute]
#version 460

#include "../utility.glsl"
#include "../voxel_world.glsl"

layout(std430, set = 1, binding = 0) restrict buffer Params {
    vec4 camera_origin;
    vec4 camera_direction;
    vec4 hit_position;
    float near;
    float far;
    float radius;  
    uint value;
} params;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 world_pos = ivec3(params.hit_position.xyz) + pos - ivec3(params.radius);
    if (!isValidPos(world_pos) || params.hit_position.w < 0) return;

    vec3 center = params.hit_position.xyz;
    float d = length(vec3(world_pos) - center);

    if (d < params.radius) {
        uint brick_index = getBrickIndex(world_pos);
        uint voxel_index = voxelBricks[brick_index].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(world_pos);     
        bool isAir = isVoxelAir(getVoxel(voxel_index));

        Voxel voxel = createAirVoxel();
        if(params.value == 1)
            voxel = createRockVoxel(world_pos);
        if(params.value == 2)
            voxel = createSandVoxel(world_pos);
        if (params.value == 3)
            voxel = createWaterVoxel(world_pos);
        if (params.value == 4)
            voxel = createLavaVoxel(world_pos); 

        if(isAir ^^ isVoxelAir(voxel))
            setBothVoxelBuffers(voxel_index, voxel);
    }
}
