#ifndef PATH_TRACING_CAMERA_H
#define PATH_TRACING_CAMERA_H

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/texture2drd.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <voxel_world.h>
#include <memory>

using namespace godot;

class VoxelCamera : public Node3D
{
    GDCLASS(VoxelCamera, Node3D);

  public:

    struct RenderParameters // match the struct on the gpu
    {
        Vector4 backgroundColor;
        int width;
        int height;
        float fov;

    };

    struct CameraParameters // match the struct on the gpu
    {
        float vp[16];
        float ivp[16];
        Vector4 cameraPosition;
        int frame_index;
        float nearPlane;
        float farPlane;

    };

  protected:
    static void _bind_methods();

  public:
    VoxelCamera() = default;
    ~VoxelCamera() override;

    void _notification(int what);

    float get_fov() const;
    void set_fov(float value);

    TextureRect *get_output_texture() const;
    void set_output_texture(TextureRect *value);

    VoxelWorld *get_voxel_world() const;
    void set_voxel_world(VoxelWorld *value);

  private:
    bool init(VoxelWorld *world);
    void release();
    bool resize_targets(const Vector2i &resolution);
    void render();

    float fov = 90.0f;
    float near_plane = 0.01f;
    float far_plane = 1000.0f;

    // Node references are stored as ObjectIDs so freed nodes never leave dangling pointers.
    ObjectID _output_texture_rect_id;
    ObjectID _voxel_world_id;

    std::unique_ptr<ComputeKernel> _kernel;
    bool _init_failed = false;
    Vector2i _resolution;

    Ref<Texture2DRD> output_texture;
    GpuResource _output_texture;
    GpuResource _depth_texture;
    GpuResource _render_parameters_buffer;
    GpuResource _camera_parameters_buffer;

    RenderParameters render_parameters = {};
    CameraParameters camera_parameters = {};
    RenderingDevice *_rd = nullptr;
};

#endif // PATH_TRACING_CAMERA_H
