#ifndef VOXEL_WORLD_GLSL
#define VOXEL_WORLD_GLSL



#define MAX_RAY_STEPS 1000
#define BRICK_EDGE_LENGTH 8
#define BRICK_VOLUME 512 

struct Brick { // we may add color to this for simple LOD
    uint occupancy_count;      // mask for voxels in the brick; 0 means the brick is empty
    uint voxel_data_pointer;  // index of the first voxel in the brick (voxels stored in Morton order)
};

struct Voxel {
    uint data;
};

layout(std430, set = 0, binding = 0) buffer VoxelWorldProperties {
    ivec4 grid_size;
    ivec4 brick_grid_size;
    vec4 sky_color;
    vec4 ground_color;
    vec4 sun_color;
    vec4 sun_direction;
    float scale;
    int frame;
} voxelWorldProperties;

layout(std430, set = 0, binding = 1) buffer VoxelWorldBricks {
    Brick voxelBricks[];
};

layout(std430, set = 0, binding = 2) buffer VoxelWorldData {
    Voxel voxelData[];
};

layout(std430, set = 0, binding = 3) buffer VoxelWorldData2 {
    Voxel voxelData2[];
};



// -------------------------------------- VOXEL DATA --------------------------------------

const uint VOXEL_TYPE_AIR = 0;
const uint VOXEL_TYPE_SOLID = 1;
const uint VOXEL_TYPE_WATER = 2;
const uint VOXEL_TYPE_LAVA = 3;
const uint VOXEL_TYPE_SAND = 4;
const vec3 DEFAULT_WATER_COLOR = vec3(0.1, 0.3, 0.8);
const vec3 DEFAULT_LAVA_COLOR = vec3(4.0, 0.6, 0.1);

Voxel createVoxel(uint type, vec3 color) {
    Voxel voxel;
    voxel.data = (type & 0xFF) << 24; // Store type in the highest byte
    voxel.data |= (compress_color16(color) & 0xFFFF) << 8; //store color in the next 2 bytes
    return voxel;
}

Voxel createAirVoxel() {
    return createVoxel(VOXEL_TYPE_AIR, vec3(0.0));
}

Voxel createWaterVoxel(ivec3 pos) {
    vec3 color = randomizedColor(DEFAULT_WATER_COLOR, pos); 
    Voxel voxel = createVoxel(VOXEL_TYPE_WATER, color);
    // voxel.data |= 127;
    return voxel;
}

Voxel createLavaVoxel(ivec3 pos) {
    vec3 color = randomizedColor(DEFAULT_LAVA_COLOR, pos); 
    Voxel voxel = createVoxel(VOXEL_TYPE_LAVA, color);
    // voxel.data |= 127;
    return voxel;
}

Voxel createGrassVoxel(ivec3 pos) {
   vec3 color = randomizedColor(vec3(.2, .9, .45), pos);    
    return createVoxel(VOXEL_TYPE_SOLID, color);
}

Voxel createSandVoxel(ivec3 pos) {
    vec3 color = randomizedColor(vec3(.91, .82, .52), pos);
    return createVoxel(VOXEL_TYPE_SAND, color);
}

Voxel createRockVoxel(ivec3 pos) {
    vec3 color = randomizedColor(vec3(.24, .25, .32), pos);
    return createVoxel(VOXEL_TYPE_SOLID, color);
}

bool isVoxelType(Voxel voxel, uint type) {
    return ((voxel.data >> 24) & 0xFF) == (type & 0xFF);
}

bool equalsVoxelType(Voxel a, Voxel b) {
    return ((a.data >> 24) & 0xFF) == ((b.data >> 24) & 0xFF);
}

bool isVoxelAir(Voxel voxel) {
    return isVoxelType(voxel, VOXEL_TYPE_AIR);
}

bool isVoxelLiquid(Voxel voxel) {
    return isVoxelType(voxel, VOXEL_TYPE_WATER) || isVoxelType(voxel, VOXEL_TYPE_LAVA);
}

bool isVoxelSolid(Voxel voxel) {
    return !isVoxelType(voxel, VOXEL_TYPE_AIR) && !isVoxelLiquid(voxel);
}

bool isVoxelDynamic(Voxel voxel) {
    return isVoxelLiquid(voxel) || isVoxelType(voxel, VOXEL_TYPE_SAND);
}

vec3 getVoxelColor(Voxel voxel, ivec3 pos) {
    // ivec3 liquid_pos = pos + ivec3(10 * sin(voxelWorldProperties.frame * 0.0167));
    // if(isVoxelType(voxel, VOXEL_TYPE_WATER))
    //     return randomizedColor(DEFAULT_WATER_COLOR, liquid_pos); 
    // if(isVoxelType(voxel, VOXEL_TYPE_LAVA))
    //     return randomizedColor(DEFAULT_LAVA_COLOR, liquid_pos); 

    uint color = (voxel.data >> 8) & 0xFFFF;
    return decompress_color16(color);
}

//0 is base value
float getVoxelEmission(Voxel voxel) {
     return isVoxelType(voxel, VOXEL_TYPE_LAVA) ? 1 : 0;
}

Voxel getPreviousVoxel(uint index)
{
    return voxelWorldProperties.frame % 2 == 0 ? voxelData2[index] : voxelData[index];
}

Voxel getVoxel(uint index)
{
    return voxelWorldProperties.frame % 2 == 0 ? voxelData[index] : voxelData2[index];
}

void setVoxel(uint index, Voxel voxel) {
    if (voxelWorldProperties.frame % 2 == 0)
        voxelData[index] = voxel;
    else 
        voxelData2[index] = voxel;    
}

void setPreviousVoxel(uint index, Voxel voxel) {
    if (voxelWorldProperties.frame % 2 == 0)
        voxelData2[index] = voxel;
    else 
        voxelData[index] = voxel;    
}


void setBothVoxelBuffers(uint index, Voxel voxel)
{
    voxelData[index] = voxel;
    voxelData2[index] = voxel;
}

// -------------------------------------- UTILS --------------------------------------
bool isValidPos(ivec3 pos) {
    return pos.x >= 0 && pos.x < voxelWorldProperties.grid_size.x &&
           pos.y >= 0 && pos.y < voxelWorldProperties.grid_size.y &&
           pos.z >= 0 && pos.z < voxelWorldProperties.grid_size.z;
}

uint getBrickIndex(ivec3 pos) {
    ivec3 brickCoord = pos / BRICK_EDGE_LENGTH;
    return brickCoord.x + brickCoord.y * voxelWorldProperties.brick_grid_size.x + brickCoord.z * voxelWorldProperties.brick_grid_size.x * voxelWorldProperties.brick_grid_size.y;
}


#define USE_MORTON_ORDER
uint getVoxelIndexInBrick(ivec3 pos) {
    ivec3 localPos = pos % BRICK_EDGE_LENGTH;
#ifdef USE_MORTON_ORDER
    uint morton = 0u;
    morton |= ((uint(localPos.x) >> 0) & 1u) << 0;
    morton |= ((uint(localPos.y) >> 0) & 1u) << 1;
    morton |= ((uint(localPos.z) >> 0) & 1u) << 2;
    morton |= ((uint(localPos.x) >> 1) & 1u) << 3;
    morton |= ((uint(localPos.y) >> 1) & 1u) << 4;
    morton |= ((uint(localPos.z) >> 1) & 1u) << 5;
    morton |= ((uint(localPos.x) >> 2) & 1u) << 6;
    morton |= ((uint(localPos.y) >> 2) & 1u) << 7;
    morton |= ((uint(localPos.z) >> 2) & 1u) << 8;
    return morton;
#endif
    return uint(localPos.x +
           (localPos.y * BRICK_EDGE_LENGTH) +
           (localPos.z * BRICK_EDGE_LENGTH * BRICK_EDGE_LENGTH));
}

uint posToIndex(ivec3 pos) {
    if (!isValidPos(pos)) return 0;
    return voxelBricks[getBrickIndex(pos)].voxel_data_pointer * BRICK_VOLUME + getVoxelIndexInBrick(pos);
}

ivec3 worldToGrid(vec3 pos) {
    return ivec3(pos / voxelWorldProperties.scale);
}

// -------------------------------------- RAYCASTING --------------------------------------
bool voxelTraceBrick(vec3 origin, vec3 direction, uint voxel_data_pointer, out uint voxelIndex, inout int step_count, inout vec3 normal, out ivec3 grid_position, out float t) {
    origin = clamp(origin, vec3(0.001), vec3(7.999));
    grid_position = ivec3(floor(origin));

    ivec3 step_dir   = ivec3(sign(direction));
    vec3 invAbsDir   = 1.0 / max(abs(direction), vec3(1e-4));
    vec3 factor      = step(vec3(0.0), direction);
    t = 0.0;

    vec3 lowerDistance = (origin - vec3(grid_position));
    vec3 upperDistance = (((vec3(grid_position) + vec3(1.0))) - origin);
    vec3 tDelta      = invAbsDir;
    vec3 tMax        = vec3(t) + mix(lowerDistance, upperDistance, factor) * invAbsDir;

    while (all(greaterThanEqual(grid_position, ivec3(0))) &&
           all(lessThanEqual(grid_position, ivec3(7)))) {
        voxelIndex = voxel_data_pointer + uint(getVoxelIndexInBrick(grid_position));
        if (!isVoxelAir(getVoxel(voxelIndex))) 
            return true;

        float minT = min(min(tMax.x, tMax.y), tMax.z);
        vec3 mask = vec3(1) - step(vec3(1e-4), abs(tMax - vec3(minT)));
        vec3 ray_step = mask * step_dir;

        t = minT;
        tMax += mask * tDelta;        
        grid_position += ivec3(ray_step);
        step_count++;
        normal = -ray_step;
    }
    
    return false;
}

bool voxelTraceWorld(vec3 origin, vec3 direction, vec2 range, out Voxel voxel, out float t, out ivec3 grid_position, out vec3 normal, out int step_count) {
    step_count = 0;
    grid_position = ivec3(0);
    voxel = createAirVoxel();
    float epsilon = 1e-4;

    float scale    = voxelWorldProperties.scale;
    float brick_scale    = scale * BRICK_EDGE_LENGTH;

    vec3 bounds_min = vec3(0.0);
    vec3 bounds_max = vec3(voxelWorldProperties.brick_grid_size.xyz) * brick_scale;

    vec3 invDir = 1.0 / max(abs(direction), vec3(epsilon)) * sign(direction);
    vec3 t0 = (bounds_min - origin) * invDir;
    vec3 t1 = (bounds_max - origin) * invDir;

    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float t_entry = max(max(tmin.x, tmin.y), tmin.z);
    float t_exit  = min(min(tmax.x, tmax.y), tmax.z);
    if (t_entry > t_exit || t_exit < 0.0)
        return false;

    // initialize normal based on the entry point
    if (t_entry == tmin.x) {
        normal = vec3(-sign(direction.x), 0.0, 0.0);
    } else if (t_entry == tmin.y) {
        normal = vec3(0.0, -sign(direction.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, -sign(direction.z));
    }

    t = max(t_entry, range.x);
    vec3 pos = origin + t * direction;
    
    pos = clamp(pos, bounds_min, bounds_max - vec3(epsilon));
    ivec3 brick_grid_position = ivec3(floor(pos / brick_scale));

    ivec3 step_dir   = ivec3(sign(direction));
    vec3 invAbsDir   = 1.0 / max(abs(direction), vec3(epsilon));
    vec3 factor      = step(vec3(0.0), direction);

    vec3 lowerDistance = (pos - vec3(brick_grid_position) * brick_scale);
    vec3 upperDistance = (((vec3(brick_grid_position) + vec3(1.0)) * brick_scale) - pos);
    vec3 tDelta      = brick_scale * invAbsDir;
    vec3 tMax        = vec3(t) + mix(lowerDistance, upperDistance, factor) * invAbsDir;    

    while(step_count < MAX_RAY_STEPS && t < min(range.y, t_exit)) {
        grid_position = brick_grid_position * BRICK_EDGE_LENGTH;

        if (!isValidPos(grid_position))
            break;
        
        uint brick_index = getBrickIndex(grid_position);
        Brick brick = voxelBricks[brick_index];
        if (brick.occupancy_count > 0) {
            pos = ((origin + t * direction) - grid_position * scale) / (brick_scale) * BRICK_EDGE_LENGTH;

            uint voxelIndex;
            ivec3 local_brick_grid_position;
            float brick_t = 0.0;
            if (voxelTraceBrick(pos, direction, brick.voxel_data_pointer * BRICK_VOLUME, voxelIndex, step_count, normal, local_brick_grid_position, brick_t)) {
                t += brick_t * voxelWorldProperties.scale;
                grid_position += local_brick_grid_position;
                voxel = getVoxel(voxelIndex);
                return true;
            }
        }

        float minT = min(min(tMax.x, tMax.y), tMax.z);
        vec3 mask = vec3(1) - step(vec3(epsilon), abs(tMax - vec3(minT)));
        vec3 ray_step = mask * step_dir;

        t = minT;
        tMax += mask * tDelta;        
        brick_grid_position += ivec3(ray_step);
        normal = -ray_step;
        step_count++;        
    }
    
    return false;
}

// -------------------------------------- Rendering --------------------------------------
vec3 sampleSkyColor(vec3 direction) {
    float intensity = max(0.0, 0.5 + dot(direction, vec3(0.0, 0.5, 0.0)));
    vec3 sky = mix(voxelWorldProperties.ground_color.rgb, voxelWorldProperties.sky_color.rgb, intensity);
    float sun_intensity = pow(max(0.0, dot(direction, voxelWorldProperties.sun_direction.xyz)), 50.0);
    return mix(sky, voxelWorldProperties.sun_color.rgb, sun_intensity);
}

float computeShadow(vec3 position, vec3 normal, vec3 lightDir) {
    float t; ivec3 grid_position; vec3 normal_out; int step_count; Voxel voxel;
    return voxelTraceWorld(position + normal * 0.001, lightDir, vec2(0.0, 100.0), voxel, t, grid_position, normal_out, step_count) ? (t > 0.001 ? 0.0 : 1.0) : 1.0;
}

// ----- AO utilities (occupancy and corner AO) -----
float _occ(ivec3 p) {
    if (!isValidPos(p)) return 0.0;
    return isVoxelAir(getVoxel(posToIndex(p))) ? 0.0 : 1.0;
}

float _vertexAo(vec2 side, float corner) {
    return (side.x + side.y + max(corner, side.x * side.y)) / 3.0;
}

float computeAmbientOcclusion(vec3 hitPos, ivec3 hitCell, vec3 normal) {
    vec3 n = normalize(normal);
    vec3 am = abs(n);
    vec3 mask = step(0.5, am);

    ivec3 imask = ivec3(int(mask.x), int(mask.y), int(mask.z));
    ivec3 d1 = ivec3(imask.z, imask.x, imask.y);
    ivec3 d2 = ivec3(imask.y, imask.z, imask.x);

    ivec3 baseCell = hitCell + ivec3(normal);

    ivec3 offsets[8] = {
        d1, -d1, d2, -d2,
        d1 + d2, -d1 + d2, -d1 - d2, d1 - d2
    };

    for (int i = 0; i < 8; ++i) {
        if (!isValidPos(baseCell + offsets[i])) {
            return 1.0;
        }
    }

    vec4 side = vec4(
        _occ(baseCell + d1),
        _occ(baseCell + d2),
        _occ(baseCell - d1),
        _occ(baseCell - d2)
    );
    vec4 corner = vec4(
        _occ(baseCell + d1 + d2),
        _occ(baseCell - d1 + d2),
        _occ(baseCell - d1 - d2),
        _occ(baseCell + d1 - d2)
    );

    vec4 light;
    light.x = 1.0 - _vertexAo(side.xy, corner.x);
    light.y = 1.0 - _vertexAo(side.yz, corner.y);
    light.z = 1.0 - _vertexAo(side.zw, corner.z);
    light.w = 1.0 - _vertexAo(side.wx, corner.w);

    vec3 gp = hitPos / voxelWorldProperties.scale;
    float u = fract(dot(mask * gp.yzx, vec3(1.0)));
    float v = fract(dot(mask * gp.zxy, vec3(1.0)));

    return mix(mix(light.z, light.w, u),
                       mix(light.y, light.x, u), v);
}

#endif // VOXEL_WORLD_GLSL
