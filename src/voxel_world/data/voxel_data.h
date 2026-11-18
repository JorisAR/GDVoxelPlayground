#ifndef VOXEL_DATA_H
#define VOXEL_DATA_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelData : public Resource
{
    GDCLASS(VoxelData, Resource)

  public:
    VoxelData() {};
    ~VoxelData() {};

    static void _bind_methods() {};

    size_t get_count() const { return get_size().x * get_size().y * get_size().z; }
    virtual Vector3i get_size() const = 0;
    virtual std::vector<Voxel> get_voxels() const = 0; //get all voxels in a grid linearized into x, y, z order
    virtual Voxel get_voxel_at(Vector3i voxel_pos) const = 0; //get all voxels in a grid linearized into x, y, z order
    virtual Error load() = 0; //initialize the voxel data, e.g. load from file


};

#endif // VOXEL_DATA_H
