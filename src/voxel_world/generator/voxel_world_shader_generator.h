#ifndef VOXEL_WORLD_SHADER_GENERATOR_H
#define VOXEL_WORLD_SHADER_GENERATOR_H

#include "voxel_world/generator/voxel_world_generator.h"
#include "voxel_world/shader_paths.h"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

class VoxelWorldShaderGenerator : public VoxelWorldGenerator {
    GDCLASS(VoxelWorldShaderGenerator, VoxelWorldGenerator)

protected:
    String shader_path = VoxelShaders::FLOATING_ISLAND;

public:
    VoxelWorldShaderGenerator() = default;
    ~VoxelWorldShaderGenerator() override = default;

    void set_shader_path(const String& path) { shader_path = path; }
    String get_shader_path() const { return shader_path; }

    void generate(RenderingDevice* rd, VoxelWorldRIDs& voxel_world_rids, const VoxelWorldProperties& properties) override;

    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_shader_path", "path"), &VoxelWorldShaderGenerator::set_shader_path);
        ClassDB::bind_method(D_METHOD("get_shader_path"), &VoxelWorldShaderGenerator::get_shader_path);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "shader_path", PROPERTY_HINT_FILE, "*.glsl"), "set_shader_path", "get_shader_path");
    }
};

#endif // VOXEL_WORLD_SHADER_GENERATOR_H
