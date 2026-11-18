#ifndef VOXEL_SHADER_PATHS_H
#define VOXEL_SHADER_PATHS_H

// Imported compute shaders (RDShaderFile). Shared include files use the .glslinc extension so that
// Godot does not try to import them as standalone shaders.
namespace VoxelShaders
{
#define VOXEL_SHADER_DIR "res://addons/voxel_playground/src/shaders/"
constexpr const char *RENDERER = VOXEL_SHADER_DIR "voxel_renderer.glsl";
constexpr const char *INITIALIZE_BRICK_GRID = VOXEL_SHADER_DIR "generators/initialize_brick_grid.glsl";
constexpr const char *FLOATING_ISLAND = VOXEL_SHADER_DIR "generators/floating_island.glsl";
constexpr const char *AUTOMATA_LIQUID = VOXEL_SHADER_DIR "automata/liquid.glsl";
constexpr const char *AUTOMATA_FREEZE_LAVA = VOXEL_SHADER_DIR "automata/freeze_lava.glsl";
constexpr const char *AUTOMATA_CLEANUP = VOXEL_SHADER_DIR "automata/cleanup_pass.glsl";
constexpr const char *EDIT_RAYCAST = VOXEL_SHADER_DIR "voxel_edit/raycast.glsl";
constexpr const char *EDIT_SPHERE = VOXEL_SHADER_DIR "voxel_edit/sphere_edit.glsl";
constexpr const char *FETCH_SOLID_MASK = VOXEL_SHADER_DIR "voxel_edit/fetch_solid_mask.glsl";
#undef VOXEL_SHADER_DIR
} // namespace VoxelShaders

#endif // VOXEL_SHADER_PATHS_H
