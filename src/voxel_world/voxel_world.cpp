
#include "voxel_world.h"
#include "voxel_world_generator.h"
#include "voxel_world/shader_paths.h"
#include <limits>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

VoxelWorld::VoxelWorld()
{
}

VoxelWorld::~VoxelWorld()
{
    release();
}

void VoxelWorld::edit_world(const Vector3 &camera_origin, const Vector3 &camera_direction, const float radius,
                            const float range, const int value)
{
    if (!_initialized || !_edit_pass)
        return;
    _edit_pass->edit_using_raycast(camera_origin, camera_direction, radius, range, value);
}

void VoxelWorld::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_generator"), &VoxelWorld::get_generator);
    ClassDB::bind_method(D_METHOD("set_generator", "generator"), &VoxelWorld::set_generator);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "generator", PROPERTY_HINT_RESOURCE_TYPE, "VoxelWorldGenerator"),
                 "set_generator", "get_generator");

    ClassDB::bind_method(D_METHOD("get_brick_map_size"), &VoxelWorld::get_brick_map_size);
    ClassDB::bind_method(D_METHOD("set_brick_map_size", "brick_map_size"), &VoxelWorld::set_brick_map_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "brick_map_size"), "set_brick_map_size", "get_brick_map_size");

    ClassDB::bind_method(D_METHOD("get_scale"), &VoxelWorld::get_scale);
    ClassDB::bind_method(D_METHOD("set_scale", "scale"), &VoxelWorld::set_scale);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale"), "set_scale", "get_scale");

    ClassDB::bind_method(D_METHOD("get_simulation_enabled"), &VoxelWorld::get_simulation_enabled);
    ClassDB::bind_method(D_METHOD("set_simulation_enabled", "enabled"), &VoxelWorld::set_simulation_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_enabled"), "set_simulation_enabled", "get_simulation_enabled");

    ClassDB::bind_method(D_METHOD("set_voxel_world_collider", "collider"), &VoxelWorld::set_voxel_world_collider);
    ClassDB::bind_method(D_METHOD("get_voxel_world_collider"), &VoxelWorld::get_voxel_world_collider);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "voxel_world_collider", PROPERTY_HINT_NODE_TYPE, "VoxelWorldCollider"),
                 "set_voxel_world_collider", "get_voxel_world_collider");

    ClassDB::bind_method(D_METHOD("get_player_node"), &VoxelWorld::get_player_node);
    ClassDB::bind_method(D_METHOD("set_player_node", "player_node"), &VoxelWorld::set_player_node);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_node", PROPERTY_HINT_NODE_TYPE, "Node3D"), "set_player_node",
                 "get_player_node");

    ClassDB::bind_method(D_METHOD("get_sun_light"), &VoxelWorld::get_sun_light);
    ClassDB::bind_method(D_METHOD("set_sun_light", "sun_light"), &VoxelWorld::set_sun_light);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sun_light", PROPERTY_HINT_NODE_TYPE, "DirectionalLight3D"),
                 "set_sun_light", "get_sun_light");

    ClassDB::bind_method(D_METHOD("get_ground_color"), &VoxelWorld::get_ground_color);
    ClassDB::bind_method(D_METHOD("set_ground_color", "ground_color"), &VoxelWorld::set_ground_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ground_color"), "set_ground_color", "get_ground_color");

    ClassDB::bind_method(D_METHOD("get_sky_color"), &VoxelWorld::get_sky_color);
    ClassDB::bind_method(D_METHOD("set_sky_color", "sky_color"), &VoxelWorld::set_sky_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "sky_color"), "set_sky_color", "get_sky_color");

    // methods
    ClassDB::bind_method(D_METHOD("edit_world", "camera_origin", "camera_direction", "radius", "range", "value"),
                         &VoxelWorld::edit_world);
}

void VoxelWorld::_notification(int p_what)
{
    if (godot::Engine::get_singleton()->is_editor_hint())
    {
        return;
    }
    switch (p_what)
    {
    case NOTIFICATION_ENTER_TREE: {
        set_physics_process_internal(true);

        break;
    }
    case NOTIFICATION_EXIT_TREE: {
        set_physics_process_internal(false);

        break;
    }
    case NOTIFICATION_READY: {
        init();
        break;
    }
    case NOTIFICATION_PREDELETE: {
        release();
        break;
    }
    case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
        float delta = get_physics_process_delta_time();
        update(delta);
        break;
    }
    }
}

void VoxelWorld::init()
{
    release();

    _rd = RenderingServer::get_singleton()->get_rendering_device();
    if (_rd == nullptr)
    {
        UtilityFunctions::printerr("VoxelWorld: requires a RenderingDevice-based renderer (Forward+ or Mobile).");
        return;
    }
    if (generator.is_null())
    {
        UtilityFunctions::printerr("VoxelWorld: No world generator set.");
        return;
    }
    if (brick_map_size.x <= 0 || brick_map_size.y <= 0 || brick_map_size.z <= 0)
    {
        UtilityFunctions::printerr("VoxelWorld: brick_map_size must be positive on every axis.");
        return;
    }

    const Vector3i size = brick_map_size * BRICK_SIZE;
    const uint64_t brick_count = uint64_t(brick_map_size.x) * brick_map_size.y * brick_map_size.z;
    const uint64_t voxel_count = uint64_t(size.x) * size.y * size.z;
    const uint64_t voxel_bytes = voxel_count * sizeof(Voxel);

    // Storage buffers are addressed with 32-bit sizes, and shaders index voxels with uints.
    const uint64_t max_buffer_bytes = std::numeric_limits<uint32_t>::max();
    if (voxel_bytes > max_buffer_bytes)
    {
        UtilityFunctions::printerr("VoxelWorld: The voxel world is too large (", voxel_count,
                                   " voxels, max 4GB per buffer). Reduce brick_map_size.");
        return;
    }

    _voxel_properties = VoxelWorldProperties(size, brick_map_size, scale);
    _voxel_properties.set_sky_colors(sky_color, ground_color);
    if (DirectionalLight3D *sun = get_sun_light())
        _voxel_properties.set_sun(sun->get_color(), sun->get_global_transform().basis.get_column(2));
    _voxel_properties.frame = 0;

    // Buffers are created zero-filled on the GPU; no multi-GB CPU staging arrays.
    _properties_buffer = GpuResource::storage_buffer(_rd, _voxel_properties.to_packed_byte_array());
    _bricks_buffer = GpuResource::storage_buffer(_rd, static_cast<uint32_t>(brick_count * sizeof(Brick)));
    _voxel_data_buffer = GpuResource::storage_buffer(_rd, static_cast<uint32_t>(voxel_bytes));
    // Second buffer for ping-ponging in the cellular automata.
    _voxel_data2_buffer = GpuResource::storage_buffer(_rd, static_cast<uint32_t>(voxel_bytes));

    if (!_properties_buffer.is_valid() || !_bricks_buffer.is_valid() || !_voxel_data_buffer.is_valid() ||
        !_voxel_data2_buffer.is_valid())
    {
        UtilityFunctions::printerr("VoxelWorld: failed to allocate GPU buffers (out of video memory?).");
        release();
        return;
    }

    _voxel_world_rids.rendering_device = _rd;
    _voxel_world_rids.properties = _properties_buffer.rid();
    _voxel_world_rids.voxel_bricks = _bricks_buffer.rid();
    _voxel_world_rids.voxel_data = _voxel_data_buffer.rid();
    _voxel_world_rids.voxel_data2 = _voxel_data2_buffer.rid();
    _voxel_world_rids.brick_count = brick_count;
    _voxel_world_rids.voxel_count = voxel_count;

    generator->initialize_brick_grid(_rd, _voxel_world_rids, _voxel_properties);
    generator->generate(_rd, _voxel_world_rids, _voxel_properties);

    _update_pass = std::make_unique<VoxelWorldUpdatePass>(VoxelShaders::AUTOMATA_LIQUID, _rd, _voxel_world_rids, size);
    _edit_pass = std::make_unique<VoxelEditPass>(VoxelShaders::EDIT_SPHERE, _rd, _voxel_world_rids, size);

    if (VoxelWorldCollider *collider = get_voxel_world_collider())
        collider->init(_rd, _voxel_world_rids, scale);

    _initialized = true;
}

void VoxelWorld::release()
{
    _initialized = false;
    if (VoxelWorldCollider *collider = get_voxel_world_collider())
        collider->release();
    // Passes hold uniform sets that reference the world buffers, so destroy them first.
    _update_pass.reset();
    _edit_pass.reset();
    _properties_buffer.reset();
    _bricks_buffer.reset();
    _voxel_data_buffer.reset();
    _voxel_data2_buffer.reset();
    _voxel_world_rids = VoxelWorldRIDs();
}

void VoxelWorld::update(float delta)
{
    if (!_initialized)
        return;
    _voxel_properties.frame++;
    _properties_buffer.update_bytes(_voxel_properties.to_packed_byte_array());

    if (simulation_enabled)
        _update_pass->update(delta);

    VoxelWorldCollider *collider = get_voxel_world_collider();
    Node3D *player = get_player_node();
    if (collider != nullptr && player != nullptr && collider->is_initialized())
        collider->update(get_voxel_world_position(player->get_global_position()));
}
