#ifndef VOXEL_DATA_VOX_H
#define VOXEL_DATA_VOX_H

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_data.h"
#include "voxel_data_vox_filter.h"
#include "voxel_world/voxel_properties.h"

using namespace godot;

class VoxelDataVox : public VoxelData
{
    GDCLASS(VoxelDataVox, VoxelData)

  public:
    VoxelDataVox() = default;
    ~VoxelDataVox() override = default;

    Vector3i get_size() const override
    {
        return size;
    }
    std::vector<Voxel> get_voxels() const override
    {
        return voxels;
    }
    std::vector<uint8_t> get_voxel_indices() const
    {
        return voxel_indices;
    }
    std::vector<Color> get_palette() const
    {
        return palette;
    }

    Voxel get_voxel_at(Vector3i p) const override
    {
        // if (swap_y_z)
        //     std::swap(p.y, p.z);

        if (p.x < 0 || p.y < 0 || p.z < 0 || p.x >= size.x || p.y >= size.y || p.z >= size.z)
            return Voxel::create_air_voxel();

        return voxels[index3(p.x, p.y, p.z)];
    }

    uint8_t get_voxel_palette_index_at(Vector3i p) const
    {
        if (p.x < 0 || p.y < 0 || p.z < 0 || p.x >= size.x || p.y >= size.y || p.z >= size.z)
            return 0;

        return voxel_indices[index3(p.x, p.y, p.z)];
    }

    String get_file_path() const
    {
        return file_path;
    }
    void set_file_path(const String &p_file_path)
    {
        file_path = p_file_path;
    }

    bool get_add_color_noise() const
    {
        return add_color_noise;
    }
    void set_add_color_noise(const bool p_add_color_noise)
    {
        add_color_noise = p_add_color_noise;
    }

    bool get_print_voxel_palette_counts() const
    {
        return print_voxel_palette_counts;
    }
    void set_print_voxel_palette_counts(const bool p_print_voxel_palette_counts)
    {
        print_voxel_palette_counts = p_print_voxel_palette_counts;
    }

    TypedArray<VoxelDataVoxFilter> get_filters() const
    {
        return filters;
    }
    void set_filters(const TypedArray<VoxelDataVoxFilter> &p_filters)
    {
        filters = p_filters;
    }

    size_t index3(int x, int y, int z) const;

    Error load() override;

    static void _bind_methods()
    {
        ClassDB::bind_method(D_METHOD("get_file_path"), &VoxelDataVox::get_file_path);
        ClassDB::bind_method(D_METHOD("set_file_path", "file_path"), &VoxelDataVox::set_file_path);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "file_path", PROPERTY_HINT_FILE, "*.vox"), "set_file_path",
                     "get_file_path");

        ClassDB::bind_method(D_METHOD("get_print_voxel_palette_counts"), &VoxelDataVox::get_print_voxel_palette_counts);
        ClassDB::bind_method(D_METHOD("set_print_voxel_palette_counts", "print_voxel_palette_counts"), &VoxelDataVox::set_print_voxel_palette_counts);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "print_voxel_palette_counts"), "set_print_voxel_palette_counts",
                     "get_print_voxel_palette_counts");

        
        ClassDB::bind_method(D_METHOD("get_add_color_noise"), &VoxelDataVox::get_add_color_noise);
        ClassDB::bind_method(D_METHOD("set_add_color_noise", "add_color_noise"), &VoxelDataVox::set_add_color_noise);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "add_color_noise"), "set_add_color_noise",
                     "get_add_color_noise");

        ClassDB::bind_method(D_METHOD("get_filters"), &VoxelDataVox::get_filters);
        ClassDB::bind_method(D_METHOD("set_filters", "value"), &VoxelDataVox::set_filters);
        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "filters", PROPERTY_HINT_TYPE_STRING,
                                  String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) +
                                      ":VoxelDataVoxFilter"), "set_filters", "get_filters");
    }

  private:
    String file_path;
    Vector3i size = Vector3i(0, 0, 0);
    std::vector<Color> palette;
    std::vector<Voxel> voxels;
    std::vector<uint8_t> voxel_indices;
    TypedArray<VoxelDataVoxFilter> filters;

    bool print_voxel_palette_counts = false;
    bool add_color_noise = false;

    // bool swap_y_z = true; // MagicaVoxel uses Z-up, Godot uses Y-up, so we need to swap Y and Z axes when loading
    static const uint32_t VoxelDataVox::DEFAULT_VOX_PALETTE_ABGR[256];
};

#endif // VOXEL_DATA_VOX_H
