#ifndef VOXEL_WORLD_GENERATOR_H
#define VOXEL_WORLD_GENERATOR_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelWorldGenerator : public Resource
{
    GDCLASS(VoxelWorldGenerator, Resource)

  public:
    VoxelWorldGenerator() = default;
    virtual ~VoxelWorldGenerator() = default;

    void initialize_brick_grid(RenderingDevice *rd, VoxelWorldRIDs &voxel_world_rids,
                               const VoxelWorldProperties &properties);

    virtual void generate(RenderingDevice *rd, VoxelWorldRIDs &voxel_world_rids,
                          const VoxelWorldProperties &properties) = 0;

    static void _bind_methods() {};
};

#endif // VOXEL_WORLD_GENERATOR_H
