#ifndef UTILITY_GLSL
#define UTILITY_GLSL

// --------------------------------------------- MATH ---------------------------------------------

vec4 saturate(vec4 color) {
    return clamp(color, vec4(0.0), vec4(1.0));
}

vec3 saturate(vec3 color) {
    return clamp(color, vec3(0.0), vec3(1.0));
}

vec2 saturate(vec2 color) {
    return clamp(color, vec2(0.0), vec2(1.0));
}

float saturate(float color) {
    return clamp(color, 0.0, 1.0);
}

// --------------------------------------------- COLORS ---------------------------------------------

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

uint compress_color16(vec3 rgb) {
    // Convert RGB to HSV
    vec3 hsv = rgb2hsv(rgb);
    
    // H: 7 bits, S: 4 bits, V: 5 bits
    uint h = uint(hsv.x * 127.0);
    uint s = uint(hsv.y * 15.0);
    uint v = uint(hsv.z * 31.0);
    
    // Pack into a single uint
    return (h << 9) | (s << 5) | v;
}

vec3 decompress_color16(uint packedColor) {
    // Extract H, S, V components
    uint h = (packedColor >> 9) & 0x7F; // 7 bits for hue
    uint s = (packedColor >> 5) & 0x0F; // 4 bits for saturation
    uint v = packedColor & 0x1F;        // 5 bits for value
    
    // Convert back to RGB
    vec3 hsv = vec3(float(h) / 128.0, float(s) / 16.0, float(v) / 32.0);
    return hsv2rgb(hsv);
}

// --------------------------------------------- RNG ---------------------------------------------

vec2 pcg2d(inout uvec2 seed) {
	// PCG2D from https://jcgt.org/published/0009/03/02/
	seed = 1664525u * seed + 1013904223u;
	seed.x += 1664525u * seed.y;
	seed.y += 1664525u * seed.x;
	seed ^= (seed >> 16u);
	seed.x += 1664525u * seed.y;
	seed.y += 1664525u * seed.x;
	seed ^= (seed >> 16u);
	return vec2(seed) * 2.32830643654e-10; 
}

uint hash(uint x) {
    x ^= x >> 16u;
    x *= 0x45d9f3bu;
    x ^= x >> 16u;
    x *= 0x45d9f3bu;
    x ^= x >> 16u;
    return x;
}

uvec2 hash(uvec2 x) {
    const uint k = 1103515245u;
    x *= k;
    return ((x >> 2u) ^ (x.yx >> 1u)) * k;
}

uvec3 hash(uvec3 x) {
    const uint k = 1103515245u;
    x *= k;
    return ((x >> 2u) ^ (x.yzx >> 1u) ^ (x.zxy)) * k;
}

uvec4 hash(uvec4 x) {
    const uint k = 1103515245u;
    x *= k;
    return ((x >> 2u) ^ (x.yzwx >> 1u) ^ (x.zwxy >> 3u) ^ (x.wxyz >> 4u)) * k;
}


vec2 box_muller(vec2 rands) {
    float R = sqrt(-2.0f * log(rands.x));
    float theta = 6.2831853f * rands.y;
    return vec2(cos(theta), sin(theta));
}

float hash(float h) {
	return fract(sin(h) * 43758.5453123);
}

float smooth_noise(vec3 x) {
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(
			mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
					mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
			mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
					mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

float fbm(vec3 p) {
	float f = 0.0;
    float scale = 0.5;

    for (int i = 0; i < 5; ++i) {
        f += smooth_noise(p) * scale;
        scale *= 0.5;
        p *= 2.01;
    }

	return f;
}

vec3 randomizedColor(vec3 base_color, ivec3 pos) {
    uvec3 hash_value = hash(pos);
    vec2 rn = pcg2d(hash_value.xy);
    vec3 color = rgb2hsv(base_color);
    color.x += rn.x * 0.025;
    color.yz *= 0.9 + rn.y * 0.2;
    return saturate(hsv2rgb(color));
}


#endif //UTILITY_GLSL