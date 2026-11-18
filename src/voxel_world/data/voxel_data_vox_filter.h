#ifndef VOXEL_DATA_VOX_FILTER_H
#define VOXEL_DATA_VOX_FILTER_H

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_data.h"
#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelDataVoxFilter : public Resource {
    GDCLASS(VoxelDataVoxFilter, Resource)

public:
    enum VoxelType {
        VOXEL_TYPE_AIR   = 0,
        VOXEL_TYPE_SOLID = 1,
        VOXEL_TYPE_WATER = 2,
        VOXEL_TYPE_LAVA  = 3,
        VOXEL_TYPE_SAND  = 4
    };

    VoxelDataVoxFilter() : type(VOXEL_TYPE_SOLID) {} // default to solid
    ~VoxelDataVoxFilter() override = default;

    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("get_palette_indices"), &VoxelDataVoxFilter::get_palette_indices);
        ClassDB::bind_method(D_METHOD("set_palette_indices", "palette_indices"), &VoxelDataVoxFilter::set_palette_indices);
        ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "palette_indices"), "set_palette_indices", "get_palette_indices");

        ClassDB::bind_method(D_METHOD("get_type"), &VoxelDataVoxFilter::get_type);
        ClassDB::bind_method(D_METHOD("set_type", "type"), &VoxelDataVoxFilter::set_type);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, 
            "Air:0,Solid:1,Water:2,Lava:3,Sand:4"),
            "set_type", "get_type");

        // Expose the enum to scripts/inspector
        BIND_ENUM_CONSTANT(VOXEL_TYPE_AIR);
        BIND_ENUM_CONSTANT(VOXEL_TYPE_SOLID);
        BIND_ENUM_CONSTANT(VOXEL_TYPE_WATER);
        BIND_ENUM_CONSTANT(VOXEL_TYPE_LAVA);
        BIND_ENUM_CONSTANT(VOXEL_TYPE_SAND);
    }

    PackedInt32Array get_palette_indices() const { return palette_indices; }
    void set_palette_indices(const PackedInt32Array &p_palette_indices) { palette_indices = p_palette_indices; }

    VoxelType get_type() const { return type; }
    void set_type(VoxelType p_type) { type = p_type; }

private:
    PackedInt32Array palette_indices;
    VoxelType type;
};

VARIANT_ENUM_CAST(VoxelDataVoxFilter::VoxelType);

#endif // VOXEL_DATA_VOX_FILTER_H
