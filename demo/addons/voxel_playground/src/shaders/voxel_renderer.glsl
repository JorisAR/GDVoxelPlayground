#[compute]
#version 460

#include "utility.glsl"
#include "voxel_world.glsl"

// ----------------------------------- STRUCTS -----------------------------------

struct Light {
    vec4 position;
    vec4 color;
};

// ----------------------------------- GENERAL STORAGE -----------------------------------

layout(set = 1, binding = 0, rgba8) restrict uniform writeonly image2D outputImage;
layout(set = 1, binding = 1, r32f) restrict uniform writeonly image2D depthBuffer;

layout(std430, set = 1, binding = 2) restrict buffer Params {
    vec4 background; //rgb, brightness
    int width;
    int height;
    float fov;
} params;

layout(std430, set = 1, binding = 3) restrict buffer Camera {
    mat4 view_projection;
    mat4 inv_view_projection;
    vec4 position;
    uint frame_index;
    float near;
    float far;
} camera;


// ----------------------------------- FUNCTIONS -----------------------------------

vec3 blinnPhongShading(vec3 baseColor, vec3 normal, vec3 lightDir, vec3 lightColor, vec3 viewDir, float shadow) {
    float NdotL = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = NdotL * baseColor;

    vec3 specular = vec3(0.0);
    vec3 H = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, H), 0.0);
    specular = pow(NdotH, 10.0) * lightColor;

    vec3 ambient = baseColor;

    vec3 result = 0.25 * shadow * specular;
    result += (shadow * 0.5 + 0.5) * diffuse;
    result += 0.2 * ambient;
    return result;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= params.width || pos.y >= params.height) return;

    vec2 screen_uv = vec2(pos + 0.5) / vec2(params.width, params.height);

    vec4 ndc = vec4(screen_uv * 2.0 - 1.0, 0.0, 1.0);
    ndc.y = -ndc.y;

    vec4 world_pos = inverse(camera.view_projection) * ndc;
    world_pos /= world_pos.w;
    vec3 ray_origin = camera.position.xyz;
    vec3 ray_dir = normalize(world_pos.xyz - ray_origin);
    ivec3 grid_position;
    vec3 normal;
    int step_count = 0;
    float t;
    vec3 color = vec3(0.0);

    Voxel voxel;

    if (voxelTraceWorld(ray_origin, ray_dir, vec2(camera.near, camera.far), voxel, t, grid_position, normal, step_count)) {
        vec3 hitPos = ray_origin + t * ray_dir;
        normal = normalize(normal);
        vec3 voxel_pos = vec3(grid_position) * voxelWorldProperties.scale;// + 0.5;
        vec3 baseColor = vec3(grid_position) / voxelWorldProperties.grid_size.xyz;
        float emission = getVoxelEmission(voxel);
        color = getVoxelColor(voxel, grid_position) * (1 + emission);
        if(isVoxelLiquid(voxel))
        {
            color += vec3(0.05 * sin(0.0167 * voxelWorldProperties.frame + 0.2 * (grid_position.x + grid_position.y + grid_position.z)));
            color += vec3(((voxel.data & 0xFu) > 0) ? 0.5 : 0);
        }

        // color *= 0.05 * dot(normal, vec3(0.5, 1.0, 0.0)) + 0.95; //discolor different faces slightly.
        vec3 voxel_view_dir = normalize(camera.position.xyz - voxel_pos);

        // direct illumination
        if(emission < 1) {
            float shadow = computeShadow(hitPos, normal, voxelWorldProperties.sun_direction.xyz);
            float ao = computeAmbientOcclusion(hitPos, grid_position, normal) * 0.7 + 0.3;
            color = ao * blinnPhongShading(color, normal, normalize(voxelWorldProperties.sun_direction.xyz), voxelWorldProperties.sun_color.rgb, voxel_view_dir, shadow);
        }
    } else {
        color = sampleSkyColor(ray_dir);
    }
    // depth = camera.far / (camera.far - camera.near) * (1.0 - camera.near / depth);
    //visualize steps
    // if(step_count < 1000)
    //     color = vec3(step_count * 0.001);
    // else
    //     color = vec3(1,0,0);


    float depth = 0.0f;
    imageStore(outputImage, pos, vec4(color, 1.0));
    imageStore(depthBuffer, pos, vec4(depth, 0.0, 0.0, 0.0));
}