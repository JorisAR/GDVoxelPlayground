#ifndef VOXEL_WORLD_COLLIDER_H
#define VOXEL_WORLD_COLLIDER_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "voxel_world/voxel_properties.h"
#include <memory>

using namespace godot;

class VoxelWorldCollider : public Node3D
{
    GDCLASS(VoxelWorldCollider, Node3D);

    struct VoxelColliderParams
    {
        Vector4i bounds_min;
        Vector4i bounds_size;

          bool is_valid_pos(Vector3i grid_pos) const {
            return grid_pos.x >= 0 && grid_pos.x < bounds_size.x &&
                grid_pos.y >= 0 && grid_pos.y < bounds_size.y &&
                grid_pos.z >= 0 && grid_pos.z < bounds_size.z;
        }


    };

  protected:
    static void _bind_methods();

  public:
    bool init(RenderingDevice *rd, const VoxelWorldRIDs &voxel_world_rids, float scale);
    void release();
    bool is_initialized() const { return _fetch_data_shader != nullptr; }
    void addQuad(const Vector3 &p1, const int dir, const bool flip);
    void onDataFetched(const PackedByteArray &data);
    VoxelWorldCollider() = default;
    ~VoxelWorldCollider() override { release(); }

    void update(Vector3i position);

    Vector3i getColliderSize() const { return _collider_size;}
    void setColliderSize(const Vector3i &size) { _collider_size = size; }

    int getUpdateInterval() const { return _update_interval;}
    void setUpdateInterval(const int interval) { _update_interval = interval; }

    CollisionShape3D *getCollisionShape() const { return _collision_shape; }
    void setCollisionShape(CollisionShape3D *shape) { _collision_shape = shape; }

  private:
    bool is_voxel_air(Vector3i pos);

    int frames_since_last_update = 0;
    int _update_interval = 15;
    bool _is_updating = false;

    Vector3i _collider_size = Vector3i(24, 32, 24);

    Vector3 collider_offset = Vector3(0, 0, 0);
    float scale = 0.125f;

    CollisionShape3D *_collision_shape = nullptr;
    Ref<ConcavePolygonShape3D> _collision_polygon = nullptr;
    RenderingDevice *_rd = nullptr;
    std::unique_ptr<ComputeKernel> _fetch_data_shader;

    VoxelColliderParams _collider_params;
    std::vector<unsigned int> _collider_voxel_data;
    PackedVector3Array _collider_vertices;

    GpuResource _collider_params_buffer;
    GpuResource _collider_voxel_data_buffer;
};

#endif // VOXEL_WORLD_COLLIDER_H
