#include "voxel_world_update_pass.h"
#include "voxel_world/shader_paths.h"

using namespace godot;

VoxelWorldUpdatePass::VoxelWorldUpdatePass(const String &liquid_shader_path, RenderingDevice *rd,
                                           const VoxelWorldRIDs &voxel_world_rids, const Vector3i size)
    : _liquid(rd, liquid_shader_path), _freeze_lava(rd, VoxelShaders::AUTOMATA_FREEZE_LAVA),
      _cleanup(rd, VoxelShaders::AUTOMATA_CLEANUP), _size(size)
{
    voxel_world_rids.add_voxel_buffers(_liquid);
    voxel_world_rids.add_voxel_buffers(_freeze_lava);
    voxel_world_rids.add_voxel_buffers(_cleanup);
}

void VoxelWorldUpdatePass::update(float /*delta*/)
{
    // Automata passes: one thread per voxel, 8x8x8 work groups.
    const Vector3i automata_groups = ComputeKernel::group_count(_size, Vector3i(8, 8, 8));
    _liquid.dispatch(automata_groups);
    _freeze_lava.dispatch(automata_groups);

    // Cleanup pass: each thread covers 2x4x2 voxels, work groups are 4x2x4 threads.
    const Vector3i cleanup_span(2 * 4, 4 * 2, 2 * 4);
    _cleanup.dispatch(ComputeKernel::group_count(_size, cleanup_span));
}
