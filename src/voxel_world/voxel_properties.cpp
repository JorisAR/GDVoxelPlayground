#include "voxel_properties.h"

using namespace godot;

const Color Voxel::DEFAULT_WATER_COLOR = Color(0.1, 0.3, 0.8);
const Color Voxel::DEFAULT_LAVA_COLOR  = Color(4.0, 0.6, 0.1);

void VoxelWorldRIDs::add_voxel_buffers(ComputeKernel &kernel) const
{
    kernel.bind_storage_buffer(0, 0, properties);
    kernel.bind_storage_buffer(0, 1, voxel_bricks);
    kernel.bind_storage_buffer(0, 2, voxel_data);
    kernel.bind_storage_buffer(0, 3, voxel_data2);
}

void VoxelWorldRIDs::set_voxel_data(const std::vector<Voxel> &voxel_data)
{
    if (voxel_data.size() != voxel_count) {
        UtilityFunctions::printerr("set_voxel_data(): Voxel data size does not match voxel count.");
        return;
    }

    PackedByteArray byte_array;
    byte_array.resize(voxel_data.size() * sizeof(Voxel));
    std::memcpy(byte_array.ptrw(), voxel_data.data(), voxel_data.size() * sizeof(Voxel));

    rendering_device->buffer_update(this->voxel_data, 0, byte_array.size(), byte_array);
    rendering_device->buffer_update(this->voxel_data2, 0, byte_array.size(), byte_array);    
}
