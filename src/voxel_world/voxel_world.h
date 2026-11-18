#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/directional_light3d.hpp>
#include <godot_cpp/core/object.hpp>
#include <memory>

#include "voxel_world/voxel_properties.h"
#include "voxel_world/cellular_automata/voxel_world_update_pass.h"
#include "voxel_world/voxel_edit/voxel_edit_pass.h"
#include "voxel_world/colliders/voxel_world_collider.h"
#include "voxel_world/generator/voxel_world_generator.h"

using namespace godot;

class VoxelWorld : public Node3D {
    GDCLASS(VoxelWorld, Node3D);

protected:
    static void _bind_methods();
    void _notification(int what);

private:
    // The size property will store the dimensions of your voxel world in terms of brick counts.
    const Vector3i BRICK_SIZE = Vector3i(8,8,8);
    Vector3i brick_map_size = Vector3i(16, 16, 16);
    float scale = 0.125f;
    bool simulation_enabled = true;
    bool _initialized = false;

    VoxelWorldRIDs _voxel_world_rids; // non-owning view handed to passes
    VoxelWorldProperties _voxel_properties;
    RenderingDevice* _rd = nullptr;

    // GPU buffers owned by the world.
    GpuResource _properties_buffer;
    GpuResource _bricks_buffer;
    GpuResource _voxel_data_buffer;
    GpuResource _voxel_data2_buffer;

    // Node references are stored as ObjectIDs so a freed node never leaves a dangling pointer.
    ObjectID _player_node_id;
    ObjectID _collider_id;

    Ref<VoxelWorldGenerator> generator;
    std::unique_ptr<VoxelWorldUpdatePass> _update_pass;
    std::unique_ptr<VoxelEditPass> _edit_pass;

    ObjectID _sun_light_id;
    Color ground_color = Color(0.5, 0.3, 0.15, 1.0);
    Color sky_color = Color(1.0, 1.0, 1.0, 1.0);

    void init();
    void release();
    void update(float delta);

    Vector3i get_voxel_world_position(const Vector3 &position) const {
        return Vector3i(static_cast<int32_t>(std::floor(position.x / scale)), static_cast<int32_t>(std::floor(position.y / scale)),
                        static_cast<int32_t>(std::floor(position.z / scale)));
    }

public:
    VoxelWorld();
    ~VoxelWorld();    

    // Property accessors for size.
    void set_brick_map_size(const Vector3i &p_size) { brick_map_size = p_size.clamp(Vector3i(0,0,0), Vector3i(256,256,256)); }
    Vector3i get_brick_map_size() const { return brick_map_size; }

    void set_scale(float p_scale) { scale = p_scale; }
    float get_scale() const { return scale; }

    void set_simulation_enabled(bool enabled) { simulation_enabled = enabled; }
    bool get_simulation_enabled() const { return simulation_enabled; }

    void set_sun_light(DirectionalLight3D* node) { _sun_light_id = node ? ObjectID(node->get_instance_id()) : ObjectID(); }
    DirectionalLight3D* get_sun_light() const { return Object::cast_to<DirectionalLight3D>(ObjectDB::get_instance(_sun_light_id)); }

    void set_ground_color(const Color &color) { ground_color = color; }
    Color get_ground_color() const { return ground_color; }
    void set_sky_color(const Color &color) { sky_color = color; }
    Color get_sky_color() const { return sky_color; }

    void set_player_node(Node3D* node) { _player_node_id = node ? ObjectID(node->get_instance_id()) : ObjectID(); }
    Node3D* get_player_node() const { return Object::cast_to<Node3D>(ObjectDB::get_instance(_player_node_id)); }

    void set_voxel_world_collider(VoxelWorldCollider* collider) { _collider_id = collider ? ObjectID(collider->get_instance_id()) : ObjectID(); }
    VoxelWorldCollider* get_voxel_world_collider() const { return Object::cast_to<VoxelWorldCollider>(ObjectDB::get_instance(_collider_id)); }

    bool is_initialized() const { return _initialized; }

    void edit_world(const Vector3 &camera_origin, const Vector3 &camera_direction, const float radius, const float range, const int value);

    const VoxelWorldRIDs &get_voxel_world_rids() const { return _voxel_world_rids; }
    const VoxelWorldProperties &get_voxel_properties() const { return _voxel_properties; }

    Ref<VoxelWorldGenerator> get_generator() const { return generator;}
    void set_generator(const Ref<VoxelWorldGenerator> p_generator) { generator = p_generator; }    
};

#endif // VOXEL_WORLD_H
