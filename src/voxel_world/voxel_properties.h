#ifndef VOXEL_WORLD_PROPERTIES_H
#define VOXEL_WORLD_PROPERTIES_H

#include "gdcs/include/gdcs.h"
#include "utils.h"
#include <functional>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/vector4.hpp>

namespace godot
{
struct Brick
{                                    // we may add color to this for simple LOD
    int occupancy_count;             // amount of voxels in the brick; 0 means the brick is empty
    unsigned int voxel_data_pointer; // index of the first voxel in the brick (voxels stored in Morton order)
};

// should match the struct on the GPU
struct Voxel
{
    Voxel() : data(0)
    {
    }

    int data;

    String get_string() const
    {
        return String::num(data);
    }

    bool operator==(const Voxel &other) const
    {
        return data == other.data;
    }

    bool operator!=(const Voxel &other) const
    {
        return data != other.data;
    }

    inline bool is_air() const
    {
        return is_type(VOXEL_TYPE_AIR);
    }
    inline bool is_type(int type) const
    {
        return ((data >> 24) & 0xFF) == (type & 0xFF);
    }

    inline Color get_color() const
    {
        unsigned int packedColor = (data >> 8) & 0xFFFF;
        return Utils::decompress_color16(packedColor);
    }

    inline int get_type() const
    {
        return (data >> 24) & 0xFF;
    }

    // static values and methods, defined the same as on the GPU
    static const unsigned int VOXEL_TYPE_AIR = 0;
    static const unsigned int VOXEL_TYPE_SOLID = 1;
    static const unsigned int VOXEL_TYPE_WATER = 2;
    static const unsigned int VOXEL_TYPE_LAVA = 3;
    static const unsigned int VOXEL_TYPE_SAND = 4;

    static const Color DEFAULT_WATER_COLOR;
    static const Color DEFAULT_LAVA_COLOR;

    static Voxel create_voxel(unsigned int type, Color color)
    {
        Voxel voxel;
        voxel.data = (type & 0xFF) << 24;                             // Store type in the highest byte
        voxel.data |= (Utils::compress_color16(color) & 0xFFFF) << 8; // store color in the next 2 bytes
        return voxel;
    }

    static Voxel create_air_voxel()
    {
        return create_voxel(VOXEL_TYPE_AIR, Color(0, 0, 0));
    }

    static Voxel create_solid_voxel(Color color)
    {
        return create_voxel(VOXEL_TYPE_SOLID, color);
    }
};

struct VoxelWorldProperties // match the struct on the gpu
{
    static const int BRICK_SIZE = 8;
    static const int BRICK_VOLUME = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;

    VoxelWorldProperties() = default;
    VoxelWorldProperties(Vector3i _grid_size, Vector3i _brick_grid_size, float _scale = 1.0f) : scale(_scale)
    {
        grid_size = Vector4i(_grid_size.x, _grid_size.y, _grid_size.z, 0);
        brick_grid_size = Vector4i(_brick_grid_size.x, _brick_grid_size.y, _brick_grid_size.z, 0);
    };

    void set_sky_colors(const Color &_sky_color, const Color &_ground_color)
    {
        sky_color = Vector4(_sky_color.r, _sky_color.g, _sky_color.b, 0);
        ground_color = Vector4(_ground_color.r, _ground_color.g, _ground_color.b, 0);
    }

    void set_sun(const Color &_sun_color, const Vector3 &_sun_direction)
    {
        sun_color = Vector4(_sun_color.r, _sun_color.g, _sun_color.b, 0);
        sun_direction = Vector4(_sun_direction.x, _sun_direction.y, _sun_direction.z, 0);
    }

    Vector4i grid_size;
    Vector4i brick_grid_size;
    Vector4 sky_color;
    Vector4 ground_color;
    Vector4 sun_color;
    Vector4 sun_direction;
    float scale;
    unsigned int frame;

    PackedByteArray to_packed_byte_array() const
    {
        PackedByteArray byte_array;
        byte_array.resize(sizeof(VoxelWorldProperties));
        std::memcpy(byte_array.ptrw(), this, sizeof(VoxelWorldProperties));
        return byte_array;
    }

    bool isValidPos(Vector3i grid_pos) const
    {
        return grid_pos.x >= 0 && grid_pos.x < grid_size.x && grid_pos.y >= 0 && grid_pos.y < grid_size.y &&
               grid_pos.z >= 0 && grid_pos.z < grid_size.z;
    }

    // get the index of the brick. multiplied by brick volume, this is the pointer to the first voxel in the brick.
    unsigned int getBrickIndex(Vector3i grid_pos) const
    {
        Vector3i brick_pos = grid_pos / BRICK_SIZE;
        return brick_pos.x + brick_pos.y * brick_grid_size.x + brick_pos.z * brick_grid_size.x * brick_grid_size.y;
    }

    // get the pointer to the first voxel in the voxel array in the brick at grid_pos.
    // NOTE: this assumes that each brick grid position has a brick, and that they are initialized in order.

    unsigned int getDefaultBrickVoxelPointer(Vector3i grid_pos) const
    {
        return getBrickIndex(grid_pos) * BRICK_VOLUME;
    }

#define VOXEL_USE_MORTON_ORDER
    unsigned int getVoxelIndexInBrick(Vector3i grid_pos) const
    {
        Vector3i localPos = grid_pos % BRICK_SIZE;
#ifdef VOXEL_USE_MORTON_ORDER
        unsigned int morton = 0u;
        morton |= ((static_cast<unsigned int>(localPos.x) >> 0) & 1u) << 0;
        morton |= ((static_cast<unsigned int>(localPos.y) >> 0) & 1u) << 1;
        morton |= ((static_cast<unsigned int>(localPos.z) >> 0) & 1u) << 2;
        morton |= ((static_cast<unsigned int>(localPos.x) >> 1) & 1u) << 3;
        morton |= ((static_cast<unsigned int>(localPos.y) >> 1) & 1u) << 4;
        morton |= ((static_cast<unsigned int>(localPos.z) >> 1) & 1u) << 5;
        morton |= ((static_cast<unsigned int>(localPos.x) >> 2) & 1u) << 6;
        morton |= ((static_cast<unsigned int>(localPos.y) >> 2) & 1u) << 7;
        morton |= ((static_cast<unsigned int>(localPos.z) >> 2) & 1u) << 8;
        return morton;
#endif
        return static_cast<unsigned int>(localPos.x + (localPos.y * BRICK_SIZE) + (localPos.z * BRICK_SIZE * BRICK_SIZE));
    }

    unsigned int pos_to_voxel_index(Vector3i grid_pos) const
    {
        if (!isValidPos(grid_pos))
            return 0;
        return getDefaultBrickVoxelPointer(grid_pos) + getVoxelIndexInBrick(grid_pos);
    }

    Vector3i worldToGrid(Vector3 pos) const
    {
        return Vector3i(pos / scale);
    }
};

struct VoxelWorldRIDs
{
    RID properties;
    RID voxel_bricks;
    RID voxel_data;
    RID voxel_data2;

    size_t brick_count;
    size_t voxel_count;

    RenderingDevice *rendering_device = nullptr;

    void add_voxel_buffers(ComputeShader *shader);
    void set_voxel_data(const std::vector<Voxel> &voxel_data);
};
} // namespace godot

template <> struct std::hash<godot::Voxel>
{
    size_t operator()(const godot::Voxel &v) const noexcept
    {
        return std::hash<uint32_t>{}(v.data);
    }
};

#endif // VOXEL_WORLD_PROPERTIES_H