// libpost_mesh_dump_rt — Mesh shader debug dump
// Captures mesh shader output vertices/primitives for debugging
typedef unsigned uint;

void __metal_post_mesh_dump_begin(uint mesh_id) {}
void __metal_post_mesh_dump_end(uint mesh_id) {}
void __metal_post_mesh_dump_vertex(uint mesh_id, uint idx, float x, float y, float z, float w) {}
void __metal_post_mesh_dump_primitive(uint mesh_id, uint idx, uint i0, uint i1, uint i2) {}
void __metal_post_mesh_dump_payload(uint mesh_id, const void *data, uint size) {}
void __metal_post_mesh_dump_set_output_buffer(void *buffer, uint size) {}
uint __metal_post_mesh_dump_get_output_size(uint mesh_id) { return 0; }
void __metal_post_mesh_dump_reset(uint mesh_id) {}
