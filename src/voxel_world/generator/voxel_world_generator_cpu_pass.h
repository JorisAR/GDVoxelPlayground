#ifndef VOXEL_WORLD_GENERATOR_CPU_PASS_H
#define VOXEL_WORLD_GENERATOR_CPU_PASS_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelWorldGeneratorCPUPass : public Resource
{
    GDCLASS(VoxelWorldGeneratorCPUPass, Resource)

  public:
    VoxelWorldGeneratorCPUPass() = default;
    virtual ~VoxelWorldGeneratorCPUPass() = default;

    virtual bool generate(std::vector<Voxel> &voxel_data, const Vector3i bounds_min, const Vector3i bounds_max, const VoxelWorldProperties &properties) = 0;

    static void _bind_methods() {};
};

#endif // VOXEL_WORLD_GENERATOR_CPU_PASS_H
