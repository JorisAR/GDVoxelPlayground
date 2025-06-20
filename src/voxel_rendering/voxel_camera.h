#ifndef PATH_TRACING_CAMERA_H
#define PATH_TRACING_CAMERA_H

#include "gdcs/include/gdcs.h"
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

        PackedByteArray to_packed_byte_array()
        {
            PackedByteArray byte_array;
            byte_array.resize(sizeof(RenderParameters));
            std::memcpy(byte_array.ptrw(), this, sizeof(RenderParameters));
            return byte_array;
        }
    };

    struct CameraParameters // match the struct on the gpu
    {
        float vp[16];
        float ivp[16];
        Vector4 cameraPosition;
        int frame_index;
        float nearPlane;
        float farPlane;

        PackedByteArray to_packed_byte_array()
        {
            PackedByteArray byte_array;
            byte_array.resize(sizeof(CameraParameters));
            std::memcpy(byte_array.ptrw(), this, sizeof(CameraParameters));
            return byte_array;
        }
    };

  protected:
    static void _bind_methods();

  public:
    void _notification(int what);

    float get_fov() const;
    void set_fov(float value);

    TextureRect *get_output_texture() const;
    void set_output_texture(TextureRect *value);

    VoxelWorld *get_voxel_world() const;
    void set_voxel_world(VoxelWorld* value);

  private:
    void init();
    void update(double delta);
    void clear_compute_shader();
    void render();

    float fov = 90.0f;
    // int num_bounces = 4;

    ComputeShader *cs = nullptr;
    TextureRect *output_texture_rect = nullptr;
    VoxelWorld *voxel_world = nullptr;
    Ref<Image> output_image;
    Ref<Image> depth_image;
    Ref<Texture2DRD> output_texture;

    RenderParameters render_parameters;
    CameraParameters camera_parameters;
    Projection projection_matrix;

    // BUFFER IDs
    RID output_texture_rid;
    RID depth_texture_rid;
    RID render_parameters_rid;
    RID camera_parameters_rid;
    RenderingDevice *_rd;
};

#endif // PATH_TRACING_CAMERA_H