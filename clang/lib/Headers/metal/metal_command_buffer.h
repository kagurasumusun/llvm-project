// metal_command_buffer — MSL command buffer / ICB (cleanroom, address-space-qualified)
#ifndef _METAL_COMMAND_BUFFER_H_
#define _METAL_COMMAND_BUFFER_H_
#include <metal/metal_common>

namespace metal
{

// ============================================================================
// MTLIndirectCommandBufferExecutionRange
// ============================================================================
struct MTLIndirectCommandBufferExecutionRange {
  uint location;
  uint length;
};

// ============================================================================
// Helper: generate address-space constructor/operator pattern
// ============================================================================
#define _METAL_STRUCT_AS_CTORS(NAME) \
  METAL_FUNC NAME() thread = default; \
  METAL_FUNC NAME() constant = default; \
  METAL_FUNC NAME() device = default; \
  METAL_FUNC NAME() device coherent(device) = default; \
  /* copy constructors to thread */ \
  METAL_FUNC NAME(const thread NAME &) thread = default; \
  METAL_FUNC NAME(const device NAME &) thread = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) thread = default; \
  METAL_FUNC NAME(const constant NAME &) thread = default; \
  METAL_FUNC NAME(const ray_data NAME &) thread = default; \
  METAL_FUNC NAME(const object_data NAME &) thread = default; \
  /* copy constructors to constant */ \
  METAL_FUNC NAME(const thread NAME &) constant = default; \
  METAL_FUNC NAME(const device NAME &) constant = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) constant = default; \
  METAL_FUNC NAME(const constant NAME &) constant = default; \
  METAL_FUNC NAME(const ray_data NAME &) constant = default; \
  METAL_FUNC NAME(const object_data NAME &) constant = default; \
  /* copy constructors to device */ \
  METAL_FUNC NAME(const thread NAME &) device = default; \
  METAL_FUNC NAME(const device NAME &) device = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) device = default; \
  METAL_FUNC NAME(const constant NAME &) device = default; \
  METAL_FUNC NAME(const ray_data NAME &) device = default; \
  METAL_FUNC NAME(const object_data NAME &) device = default; \
  /* copy constructors to device coherent */ \
  METAL_FUNC NAME(const thread NAME &) device coherent(device) = default; \
  METAL_FUNC NAME(const device NAME &) device coherent(device) = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) device coherent(device) = default; \
  METAL_FUNC NAME(const constant NAME &) device coherent(device) = default; \
  METAL_FUNC NAME(const ray_data NAME &) device coherent(device) = default; \
  METAL_FUNC NAME(const object_data NAME &) device coherent(device) = default; \
  /* copy constructors to ray_data */ \
  METAL_FUNC NAME(const thread NAME &) ray_data = default; \
  METAL_FUNC NAME(const device NAME &) ray_data = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) ray_data = default; \
  METAL_FUNC NAME(const constant NAME &) ray_data = default; \
  METAL_FUNC NAME(const ray_data NAME &) ray_data = default; \
  METAL_FUNC NAME(const object_data NAME &) ray_data = default; \
  /* copy constructors to object_data */ \
  METAL_FUNC NAME(const thread NAME &) object_data = default; \
  METAL_FUNC NAME(const device NAME &) object_data = default; \
  METAL_FUNC NAME(const device coherent(device) NAME &) object_data = default; \
  METAL_FUNC NAME(const constant NAME &) object_data = default; \
  METAL_FUNC NAME(const ray_data NAME &) object_data = default; \
  METAL_FUNC NAME(const object_data NAME &) object_data = default; \
  /* assignment operators from all AS to all AS */ \
  METAL_FUNC thread NAME &operator=(const thread NAME &) thread = default; \
  METAL_FUNC thread NAME &operator=(const device NAME &) thread = default; \
  METAL_FUNC thread NAME &operator=(const device coherent(device) NAME &) thread = default; \
  METAL_FUNC thread NAME &operator=(const constant NAME &) thread = default; \
  METAL_FUNC thread NAME &operator=(const ray_data NAME &) thread = default; \
  METAL_FUNC thread NAME &operator=(const object_data NAME &) thread = default; \
  METAL_FUNC device NAME &operator=(const thread NAME &) device = default; \
  METAL_FUNC device NAME &operator=(const device NAME &) device = default; \
  METAL_FUNC device NAME &operator=(const device coherent(device) NAME &) device = default; \
  METAL_FUNC device NAME &operator=(const constant NAME &) device = default; \
  METAL_FUNC device NAME &operator=(const ray_data NAME &) device = default; \
  METAL_FUNC device NAME &operator=(const object_data NAME &) device = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const thread NAME &) device coherent(device) = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const device NAME &) device coherent(device) = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const device coherent(device) NAME &) device coherent(device) = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const constant NAME &) device coherent(device) = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const ray_data NAME &) device coherent(device) = default; \
  METAL_FUNC device coherent(device) NAME &operator=(const object_data NAME &) device coherent(device) = default; \
  METAL_FUNC constant NAME &operator=(const thread NAME &) constant = default; \
  METAL_FUNC constant NAME &operator=(const device NAME &) constant = default; \
  METAL_FUNC constant NAME &operator=(const device coherent(device) NAME &) constant = default; \
  METAL_FUNC constant NAME &operator=(const constant NAME &) constant = default; \
  METAL_FUNC constant NAME &operator=(const ray_data NAME &) constant = default; \
  METAL_FUNC constant NAME &operator=(const object_data NAME &) constant = default; \
  METAL_FUNC ray_data NAME &operator=(const thread NAME &) ray_data = default; \
  METAL_FUNC ray_data NAME &operator=(const device NAME &) ray_data = default; \
  METAL_FUNC ray_data NAME &operator=(const device coherent(device) NAME &) ray_data = default; \
  METAL_FUNC ray_data NAME &operator=(const constant NAME &) ray_data = default; \
  METAL_FUNC ray_data NAME &operator=(const ray_data NAME &) ray_data = default; \
  METAL_FUNC ray_data NAME &operator=(const object_data NAME &) ray_data = default; \
  METAL_FUNC object_data NAME &operator=(const thread NAME &) object_data = default; \
  METAL_FUNC object_data NAME &operator=(const device NAME &) object_data = default; \
  METAL_FUNC object_data NAME &operator=(const device coherent(device) NAME &) object_data = default; \
  METAL_FUNC object_data NAME &operator=(const constant NAME &) object_data = default; \
  METAL_FUNC object_data NAME &operator=(const ray_data NAME &) object_data = default; \
  METAL_FUNC object_data NAME &operator=(const object_data NAME &) object_data = default

// ============================================================================
// command_buffer
// ============================================================================
struct command_buffer
{
  _METAL_STRUCT_AS_CTORS(command_buffer);

  // size() for each address space
  METAL_FUNC size_t size() const thread { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const device { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const device coherent(device) { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const constant { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const ray_data { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const object_data { return __air_get_size_command_buffer(t); }

  // GPU timing
  METAL_FUNC ulong get_gpu_thread_execution_time() const thread { return __air_get_gpu_thread_execution_time_command_buffer(t); }
  METAL_FUNC ulong get_kernel_start_time() const thread { return __air_get_kernel_start_time_command_buffer(t); }
  METAL_FUNC ulong get_kernel_end_time() const thread { return __air_get_kernel_end_time_command_buffer(t); }

private:
  __air_command_buffer_t t;
  friend struct _command;
};

// ============================================================================
// _command (protected base for ICB commands)
// ============================================================================
struct _command
{
protected:
  METAL_FUNC explicit _command(command_buffer cb, uint idx) thread
    : icb(cb.t), icb_index(idx) {}

  __air_command_buffer_t icb;
  uint icb_index;

  friend struct render_command;
  friend struct compute_command;
};

// ============================================================================
// Additional enums
// ============================================================================
enum class store_action {
  dont_care = 0,
  store = 1,
  multisample_resolve = 2
};

enum class load_action {
  dont_care = 0,
  load = 1,
  clear = 2
};

enum class blend_factor {
  zero = 0,
  one = 1,
  source_color = 2,
  one_minus_source_color = 3,
  source_alpha = 4,
  one_minus_source_alpha = 5,
  destination_color = 6,
  one_minus_destination_color = 7,
  destination_alpha = 8,
  one_minus_destination_alpha = 9,
  source_alpha_saturated = 10,
  blend_color = 11,
  one_minus_blend_color = 12,
  blend_alpha = 13,
  one_minus_blend_alpha = 14
};

enum class blend_operation {
  add = 0,
  subtract = 1,
  reverse_subtract = 2,
  min = 3,
  max = 4
};

enum class compare_function {
  never = 0,
  less = 1,
  equal = 2,
  less_equal = 3,
  greater = 4,
  not_equal = 5,
  greater_equal = 6,
  always = 7
};

enum class stencil_operation {
  keep = 0,
  zero = 1,
  replace = 2,
  increment_clamp = 3,
  decrement_clamp = 4,
  invert = 5,
  increment_wrap = 6,
  decrement_wrap = 7
};

enum class index_type {
  uint16 = 0,
  uint32 = 1
};

enum class triangle_fill_mode {
  fill = 0,
  lines = 1
};

// ============================================================================
// vertex_amplification_view_count
// ============================================================================
struct vertex_amplification_view_count {
  uint count;
  uint4 mask;
  METAL_FUNC vertex_amplification_view_count() thread = default;
  METAL_FUNC vertex_amplification_view_count(uint c, uint4 m) thread : count(c), mask(m) {}
};

// ============================================================================
// visibility_result_buffer
// ============================================================================
struct visibility_result_buffer {
  METAL_FUNC visibility_result_buffer() thread = default;
  METAL_FUNC visibility_result_buffer(const thread visibility_result_buffer &) thread = default;
  METAL_FUNC thread visibility_result_buffer &operator=(const thread visibility_result_buffer &) thread = default;

  METAL_FUNC void set_visibility_result(device ulong *buffer, uint offset) thread {
    __air_set_visibility_result(t, buffer, offset);
  }
  METAL_FUNC void clear_visibility_result() thread { __air_clear_visibility_result(t); }

private:
  __air_visibility_result_buffer_t t;
};

// ============================================================================
// compute_pipeline_state
// ============================================================================
struct compute_pipeline_state
{
  _METAL_STRUCT_AS_CTORS(compute_pipeline_state);

private:
  __air_compute_pipeline_state_t t;
  friend struct compute_command;
};

// ============================================================================
// compute_command — extended with AS-qualified constructors
// ============================================================================
struct compute_command : _command
{
  METAL_FUNC explicit compute_command(command_buffer cb, uint idx) thread
    : _command(cb, idx) {}

  METAL_FUNC compute_command(const thread compute_command &) thread = default;
  METAL_FUNC thread compute_command &operator=(const thread compute_command &) thread = default;

  // Additional AS-qualified copy constructors (for ICB access patterns)
  METAL_FUNC compute_command(const device compute_command &) thread = default;
  METAL_FUNC compute_command(const constant compute_command &) thread = default;
  METAL_FUNC compute_command(const device coherent(device) compute_command &) thread = default;
  METAL_FUNC compute_command(const ray_data compute_command &) thread = default;
  METAL_FUNC compute_command(const object_data compute_command &) thread = default;
  METAL_FUNC compute_command(const thread compute_command &) device = default;
  METAL_FUNC compute_command(const device compute_command &) device = default;
  METAL_FUNC compute_command(const constant compute_command &) device = default;
  METAL_FUNC compute_command(const device coherent(device) compute_command &) device = default;
  METAL_FUNC compute_command(const thread compute_command &) constant = default;
  METAL_FUNC compute_command(const device compute_command &) constant = default;
  METAL_FUNC compute_command(const constant compute_command &) constant = default;
  METAL_FUNC compute_command(const device coherent(device) compute_command &) constant = default;
  METAL_FUNC compute_command(const thread compute_command &) ray_data = default;
  METAL_FUNC compute_command(const device compute_command &) ray_data = default;
  METAL_FUNC compute_command(const constant compute_command &) ray_data = default;
  METAL_FUNC compute_command(const device coherent(device) compute_command &) ray_data = default;
  METAL_FUNC compute_command(const thread compute_command &) object_data = default;
  METAL_FUNC compute_command(const device compute_command &) object_data = default;
  METAL_FUNC compute_command(const constant compute_command &) object_data = default;
  METAL_FUNC compute_command(const device coherent(device) compute_command &) object_data = default;

  // Assignment operators for all AS pairs
  METAL_FUNC thread compute_command &operator=(const device compute_command &) thread = default;
  METAL_FUNC thread compute_command &operator=(const constant compute_command &) thread = default;
  METAL_FUNC thread compute_command &operator=(const device coherent(device) compute_command &) thread = default;
  METAL_FUNC thread compute_command &operator=(const ray_data compute_command &) thread = default;
  METAL_FUNC thread compute_command &operator=(const object_data compute_command &) thread = default;
  METAL_FUNC device compute_command &operator=(const thread compute_command &) device = default;
  METAL_FUNC device compute_command &operator=(const device compute_command &) device = default;
  METAL_FUNC device compute_command &operator=(const constant compute_command &) device = default;
  METAL_FUNC device compute_command &operator=(const device coherent(device) compute_command &) device = default;
  METAL_FUNC device compute_command &operator=(const ray_data compute_command &) device = default;
  METAL_FUNC device compute_command &operator=(const object_data compute_command &) device = default;
  METAL_FUNC constant compute_command &operator=(const thread compute_command &) constant = default;
  METAL_FUNC constant compute_command &operator=(const device compute_command &) constant = default;
  METAL_FUNC constant compute_command &operator=(const constant compute_command &) constant = default;
  METAL_FUNC constant compute_command &operator=(const device coherent(device) compute_command &) constant = default;
  METAL_FUNC constant compute_command &operator=(const ray_data compute_command &) constant = default;
  METAL_FUNC constant compute_command &operator=(const object_data compute_command &) constant = default;
  METAL_FUNC ray_data compute_command &operator=(const thread compute_command &) ray_data = default;
  METAL_FUNC ray_data compute_command &operator=(const device compute_command &) ray_data = default;
  METAL_FUNC ray_data compute_command &operator=(const constant compute_command &) ray_data = default;
  METAL_FUNC ray_data compute_command &operator=(const device coherent(device) compute_command &) ray_data = default;
  METAL_FUNC ray_data compute_command &operator=(const ray_data compute_command &) ray_data = default;
  METAL_FUNC ray_data compute_command &operator=(const object_data compute_command &) ray_data = default;
  METAL_FUNC object_data compute_command &operator=(const thread compute_command &) object_data = default;
  METAL_FUNC object_data compute_command &operator=(const device compute_command &) object_data = default;
  METAL_FUNC object_data compute_command &operator=(const constant compute_command &) object_data = default;
  METAL_FUNC object_data compute_command &operator=(const device coherent(device) compute_command &) object_data = default;
  METAL_FUNC object_data compute_command &operator=(const ray_data compute_command &) object_data = default;
  METAL_FUNC object_data compute_command &operator=(const object_data compute_command &) object_data = default;

  METAL_FUNC void set_compute_pipeline_state(compute_pipeline_state ps) thread {
    __air_set_pipeline_state_compute_command(icb, icb_index, ps.t);
  }

  template <typename T>
  METAL_FUNC void set_kernel_buffer(device T *buffer, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(constant T *buffer, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(constant T *buffer, size_t stride, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(threadgroup T *buffer, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }

  METAL_FUNC void concurrent_dispatch_threadgroups(uint3 tg_per_grid, uint3 threads_per_tg) thread {
    __air_concurrent_dispatch_threadgroups_compute_command(icb, icb_index, tg_per_grid, threads_per_tg);
  }
  METAL_FUNC void concurrent_dispatch_threads(uint3 threads_per_grid, uint3 threads_per_tg) thread {
    __air_concurrent_dispatch_threads_compute_command(icb, icb_index, threads_per_grid, threads_per_tg);
  }
  METAL_FUNC void set_barrier() thread { __air_set_barrier_compute_command(icb, icb_index); }
  METAL_FUNC void clear_barrier() thread { __air_clear_barrier_compute_command(icb, icb_index); }
  METAL_FUNC void set_stage_in_region(uint3 origin, uint3 size) thread {
    __air_set_stage_in_region_compute_command(icb, icb_index, origin, size);
  }
  METAL_FUNC void set_threadgroup_memory_length(uint length, uint idx) thread {
    __air_set_threadgroup_memory_length_compute_command(icb, icb_index, length, idx);
  }
  METAL_FUNC void set_imageblock_size(ushort2 size) thread {
    __air_set_imageblock_size_compute_command(icb, icb_index, size);
  }
  METAL_FUNC void reset() thread { __air_reset_compute_command(icb, icb_index); }
  METAL_FUNC void copy_command(compute_command that) thread {
    __air_copy_compute_command(icb, icb_index, that.icb, that.icb_index);
  }

  // Additional compute command methods
  METAL_FUNC void set_kernel_buffer(void *buffer, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, buffer, ~size_t(0), index);
  }
  METAL_FUNC void set_kernel_buffer(void *buffer, size_t stride, uint index) thread {
    __air_set_kernel_buffer_compute_command(icb, icb_index, buffer, stride, index);
  }
  METAL_FUNC void set_texture(void *texture, uint index) thread {
    __air_set_texture_compute_command(icb, icb_index, texture, index);
  }
  METAL_FUNC void set_sampler(void *sampler, uint index) thread {
    __air_set_sampler_compute_command(icb, icb_index, sampler, index);
  }
  METAL_FUNC void set_threadgroup_memory_length(uint length, uint index, uint alignment) thread {
    __air_set_threadgroup_memory_length_aligned_compute_command(icb, icb_index, length, index, alignment);
  }
  METAL_FUNC void set_imageblock_size(ushort2 size, ushort bytes_per_row) thread {
    __air_set_imageblock_size_with_bytes_compute_command(icb, icb_index, size, bytes_per_row);
  }
};

// ============================================================================
// render_pipeline_state
// ============================================================================
struct render_pipeline_state
{
  _METAL_STRUCT_AS_CTORS(render_pipeline_state);

private:
  __air_render_pipeline_state_t t;
  friend struct render_command;
};

// ============================================================================
// depth_stencil_state
// ============================================================================
struct depth_stencil_state
{
  _METAL_STRUCT_AS_CTORS(depth_stencil_state);

private:
  __air_depth_stencil_state_t t;
  friend struct render_command;
};

// ============================================================================
// Enums for render command
// ============================================================================
enum class primitive_type {
  point = 0,
  line = 1,
  line_strip = 2,
  triangle = 3,
  triangle_strip = 4
};

enum class depth_clip_mode {
  clip = 0,
  clamp = 1
};

enum class visibility_result_mode {
  disabled = 0,
  boolean = 1,
  counting = 2
};

enum class cull_mode {
  none = 0,
  front = 1,
  back = 2
};

enum class winding {
  clockwise = 0,
  counter_clockwise = 1
};

// ============================================================================
// render_command — extended with AS-qualified constructors
// ============================================================================
struct render_command : _command
{
  METAL_FUNC explicit render_command(command_buffer cb, uint idx) thread
    : _command(cb, idx) {}

  METAL_FUNC render_command(const thread render_command &) thread = default;
  METAL_FUNC thread render_command &operator=(const thread render_command &) thread = default;

  // Additional AS-qualified copy constructors (for ICB access patterns)
  METAL_FUNC render_command(const device render_command &) thread = default;
  METAL_FUNC render_command(const constant render_command &) thread = default;
  METAL_FUNC render_command(const device coherent(device) render_command &) thread = default;
  METAL_FUNC render_command(const ray_data render_command &) thread = default;
  METAL_FUNC render_command(const object_data render_command &) thread = default;
  METAL_FUNC render_command(const thread render_command &) device = default;
  METAL_FUNC render_command(const device render_command &) device = default;
  METAL_FUNC render_command(const constant render_command &) device = default;
  METAL_FUNC render_command(const device coherent(device) render_command &) device = default;
  METAL_FUNC render_command(const thread render_command &) constant = default;
  METAL_FUNC render_command(const device render_command &) constant = default;
  METAL_FUNC render_command(const constant render_command &) constant = default;
  METAL_FUNC render_command(const device coherent(device) render_command &) constant = default;
  METAL_FUNC render_command(const thread render_command &) ray_data = default;
  METAL_FUNC render_command(const device render_command &) ray_data = default;
  METAL_FUNC render_command(const constant render_command &) ray_data = default;
  METAL_FUNC render_command(const device coherent(device) render_command &) ray_data = default;
  METAL_FUNC render_command(const thread render_command &) object_data = default;
  METAL_FUNC render_command(const device render_command &) object_data = default;
  METAL_FUNC render_command(const constant render_command &) object_data = default;
  METAL_FUNC render_command(const device coherent(device) render_command &) object_data = default;

  // Assignment operators for all AS pairs
  METAL_FUNC thread render_command &operator=(const device render_command &) thread = default;
  METAL_FUNC thread render_command &operator=(const constant render_command &) thread = default;
  METAL_FUNC thread render_command &operator=(const device coherent(device) render_command &) thread = default;
  METAL_FUNC thread render_command &operator=(const ray_data render_command &) thread = default;
  METAL_FUNC thread render_command &operator=(const object_data render_command &) thread = default;
  METAL_FUNC device render_command &operator=(const thread render_command &) device = default;
  METAL_FUNC device render_command &operator=(const device render_command &) device = default;
  METAL_FUNC device render_command &operator=(const constant render_command &) device = default;
  METAL_FUNC device render_command &operator=(const device coherent(device) render_command &) device = default;
  METAL_FUNC device render_command &operator=(const ray_data render_command &) device = default;
  METAL_FUNC device render_command &operator=(const object_data render_command &) device = default;
  METAL_FUNC constant render_command &operator=(const thread render_command &) constant = default;
  METAL_FUNC constant render_command &operator=(const device render_command &) constant = default;
  METAL_FUNC constant render_command &operator=(const constant render_command &) constant = default;
  METAL_FUNC constant render_command &operator=(const device coherent(device) render_command &) constant = default;
  METAL_FUNC constant render_command &operator=(const ray_data render_command &) constant = default;
  METAL_FUNC constant render_command &operator=(const object_data render_command &) constant = default;
  METAL_FUNC ray_data render_command &operator=(const thread render_command &) ray_data = default;
  METAL_FUNC ray_data render_command &operator=(const device render_command &) ray_data = default;
  METAL_FUNC ray_data render_command &operator=(const constant render_command &) ray_data = default;
  METAL_FUNC ray_data render_command &operator=(const device coherent(device) render_command &) ray_data = default;
  METAL_FUNC ray_data render_command &operator=(const ray_data render_command &) ray_data = default;
  METAL_FUNC ray_data render_command &operator=(const object_data render_command &) ray_data = default;
  METAL_FUNC object_data render_command &operator=(const thread render_command &) object_data = default;
  METAL_FUNC object_data render_command &operator=(const device render_command &) object_data = default;
  METAL_FUNC object_data render_command &operator=(const constant render_command &) object_data = default;
  METAL_FUNC object_data render_command &operator=(const device coherent(device) render_command &) object_data = default;
  METAL_FUNC object_data render_command &operator=(const ray_data render_command &) object_data = default;
  METAL_FUNC object_data render_command &operator=(const object_data render_command &) object_data = default;

  // Pipeline state
  METAL_FUNC void set_render_pipeline_state(render_pipeline_state ps) thread {
    __air_set_render_pipeline_state_render_command(icb, icb_index, ps.t);
  }
  METAL_FUNC void set_depth_stencil_state(depth_stencil_state ds) thread {
    __air_set_depth_stencil_state_render_command(icb, icb_index, ds.t);
  }

  // Vertex buffers
  template <typename T>
  METAL_FUNC void set_vertex_buffer(device T *buffer, uint index) thread {
    __air_set_vertex_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_vertex_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_vertex_buffer_render_command(icb, icb_index, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_vertex_buffer(constant T *buffer, uint index) thread {
    __air_set_vertex_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_vertex_buffer(threadgroup T *buffer, uint index) thread {
    __air_set_vertex_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }

  // Fragment buffers
  template <typename T>
  METAL_FUNC void set_fragment_buffer(device T *buffer, uint index) thread {
    __air_set_fragment_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_fragment_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_fragment_buffer_render_command(icb, icb_index, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_fragment_buffer(constant T *buffer, uint index) thread {
    __air_set_fragment_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_fragment_buffer(threadgroup T *buffer, uint index) thread {
    __air_set_fragment_buffer_render_command(icb, icb_index, (const void*)buffer, ~size_t(0), index);
  }

  // Draw calls
  METAL_FUNC void draw_primitives(primitive_type type, uint vertex_start, uint vertex_count,
                                   uint instance_count, uint base_instance) thread {
    __air_draw_primitives_render_command(icb, icb_index, (uint)type, vertex_start, vertex_count, instance_count, base_instance);
  }
  METAL_FUNC void draw_primitives(primitive_type type, uint vertex_start, uint vertex_count) thread {
    __air_draw_primitives_render_command(icb, icb_index, (uint)type, vertex_start, vertex_count, 1, 0);
  }

  METAL_FUNC void draw_indexed_primitives(primitive_type type, uint index_count,
                                            void *index_buffer, uint index_buffer_offset,
                                            uint instance_count, int base_vertex, uint base_instance) thread {
    __air_draw_indexed_primitives_render_command(icb, icb_index, (uint)type, index_count,
                                                  index_buffer, index_buffer_offset, instance_count, base_vertex, base_instance);
  }
  METAL_FUNC void draw_indexed_primitives(primitive_type type, uint index_count,
                                            void *index_buffer, uint index_buffer_offset) thread {
    __air_draw_indexed_primitives_render_command(icb, icb_index, (uint)type, index_count,
                                                  index_buffer, index_buffer_offset, 1, 0, 0);
  }

  // Vertex/Fragment threadgroup memory
  METAL_FUNC void set_vertex_threadgroup_memory_length(uint length, uint index) thread {
    __air_set_vertex_threadgroup_memory_length_render_command(icb, icb_index, length, index);
  }
  METAL_FUNC void set_fragment_threadgroup_memory_length(uint length, uint index) thread {
    __air_set_fragment_threadgroup_memory_length_render_command(icb, icb_index, length, index);
  }

  // Render state
  METAL_FUNC void set_vertex_amplification_count(uint count) thread {
    __air_set_vertex_amplification_count_render_command(icb, icb_index, count);
  }
  METAL_FUNC void set_scissor_rect(uint4 rect) thread {
    __air_set_scissor_rect_render_command(icb, icb_index, rect);
  }
  METAL_FUNC void set_viewport(float4 vp) thread {
    __air_set_viewport_render_command(icb, icb_index, vp);
  }
  METAL_FUNC void set_cull_mode(cull_mode mode) thread {
    __air_set_cull_mode_render_command(icb, icb_index, (uint)mode);
  }
  METAL_FUNC void set_front_facing_winding(winding w) thread {
    __air_set_front_facing_winding_render_command(icb, icb_index, (uint)w);
  }
  METAL_FUNC void set_depth_clip_mode(depth_clip_mode mode) thread {
    __air_set_depth_clip_mode_render_command(icb, icb_index, (uint)mode);
  }
  METAL_FUNC void set_depth_bias(float bias, float scale, float clamp) thread {
    __air_set_depth_bias_render_command(icb, icb_index, bias, scale, clamp);
  }
  METAL_FUNC void set_visibility_result_mode(visibility_result_mode mode, uint offset) thread {
    __air_set_visibility_result_mode_render_command(icb, icb_index, (uint)mode, offset);
  }

  // Stencil
  METAL_FUNC void set_stencil_reference_value(uint val) thread {
    __air_set_stencil_reference_value_render_command(icb, icb_index, val);
  }
  METAL_FUNC void set_stencil_front_back_reference_value(uint front, uint back) thread {
    __air_set_stencil_front_back_reference_value_render_command(icb, icb_index, front, back);
  }

  METAL_FUNC void reset() thread { __air_reset_render_command(icb, icb_index); }
  METAL_FUNC void copy_command(render_command that) thread {
    __air_copy_render_command(icb, icb_index, that.icb, that.icb_index);
  }

  // Tessellation draw calls
  METAL_FUNC void draw_patches(uint number_of_patch_control_points, uint patch_start, uint patch_count,
                                void *patch_index_buffer, uint patch_index_buffer_offset,
                                uint instance_count, uint base_instance) thread {
    __air_draw_patches_render_command(icb, icb_index, number_of_patch_control_points,
                                      patch_start, patch_count, patch_index_buffer,
                                      patch_index_buffer_offset, instance_count, base_instance);
  }
  METAL_FUNC void draw_patches(uint number_of_patch_control_points, uint patch_start, uint patch_count,
                                void *patch_index_buffer, uint patch_index_buffer_offset) thread {
    __air_draw_patches_render_command(icb, icb_index, number_of_patch_control_points,
                                      patch_start, patch_count, patch_index_buffer,
                                      patch_index_buffer_offset, 1, 0);
  }

  METAL_FUNC void draw_indexed_patches(uint number_of_patch_control_points, uint patch_start, uint patch_count,
                                         void *control_point_index_buffer, uint control_point_index_buffer_offset,
                                         uint instance_count, uint base_instance) thread {
    __air_draw_indexed_patches_render_command(icb, icb_index, number_of_patch_control_points,
                                               patch_start, patch_count, control_point_index_buffer,
                                               control_point_index_buffer_offset, instance_count, base_instance);
  }
  METAL_FUNC void draw_indexed_patches(uint number_of_patch_control_points, uint patch_start, uint patch_count,
                                         void *control_point_index_buffer, uint control_point_index_buffer_offset) thread {
    __air_draw_indexed_patches_render_command(icb, icb_index, number_of_patch_control_points,
                                               patch_start, patch_count, control_point_index_buffer,
                                               control_point_index_buffer_offset, 1, 0);
  }

  // Indirect draw calls
  METAL_FUNC void draw_primitives_indirect(primitive_type type, void *indirect_args_buffer,
                                            uint indirect_args_buffer_offset) thread {
    __air_draw_primitives_indirect_render_command(icb, icb_index, (uint)type,
                                                   indirect_args_buffer, indirect_args_buffer_offset);
  }
  METAL_FUNC void draw_indexed_primitives_indirect(primitive_type type, uint index_count,
                                                      void *index_buffer, uint index_buffer_offset,
                                                      void *indirect_args_buffer,
                                                      uint indirect_args_buffer_offset) thread {
    __air_draw_indexed_primitives_indirect_render_command(icb, icb_index, (uint)type, index_count,
                                                           index_buffer, index_buffer_offset,
                                                           indirect_args_buffer, indirect_args_buffer_offset);
  }

  // Instance amplification
  METAL_FUNC void set_vertex_amplification_count(uint count, uint4 mask) thread {
    __air_set_vertex_amplification_count_mask_render_command(icb, icb_index, count, mask);
  }

  // Texture and sampler binding
  METAL_FUNC void set_vertex_texture(void *texture, uint index) thread {
    __air_set_vertex_texture_render_command(icb, icb_index, texture, index);
  }
  METAL_FUNC void set_fragment_texture(void *texture, uint index) thread {
    __air_set_fragment_texture_render_command(icb, icb_index, texture, index);
  }
  METAL_FUNC void set_vertex_sampler(void *sampler, uint index) thread {
    __air_set_vertex_sampler_render_command(icb, icb_index, sampler, index);
  }
  METAL_FUNC void set_fragment_sampler(void *sampler, uint index) thread {
    __air_set_fragment_sampler_render_command(icb, icb_index, sampler, index);
  }

  // Tessellation factors
  METAL_FUNC void set_tessellation_factor_buffer(void *buffer, uint offset, uint instance_stride) thread {
    __air_set_tessellation_factor_buffer_render_command(icb, icb_index, buffer, offset, instance_stride);
  }
  METAL_FUNC void set_tessellation_factor_scale(float scale) thread {
    __air_set_tessellation_factor_scale_render_command(icb, icb_index, scale);
  }

  // Polygon fill
  METAL_FUNC void set_polygon_fill_mode(/* polygon_fill_mode */ uint mode) thread {
    __air_set_polygon_fill_mode_render_command(icb, icb_index, mode);
  }
};

// ============================================================================
// indirect_command_buffer
// ============================================================================
struct indirect_command_buffer
{
  _METAL_STRUCT_AS_CTORS(indirect_command_buffer);

  // get_command_at for compute and render
  METAL_FUNC compute_command get_compute_command_at(uint index) thread {
    return compute_command(*(command_buffer*)this, index);
  }
  METAL_FUNC render_command get_render_command_at(uint index) thread {
    return render_command(*(command_buffer*)this, index);
  }

  // Size for each address space
  METAL_FUNC size_t size() const thread { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const device { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const device coherent(device) { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const constant { return __air_get_size_command_buffer(t); }

  // Reset range
  METAL_FUNC void reset(MTLIndirectCommandBufferExecutionRange range) thread {
    __air_reset_indirect_command_buffer(t, range.location, range.length);
  }
  METAL_FUNC void reset() thread {
    __air_reset_indirect_command_buffer(t, 0, size());
  }

  // Additional ICB methods
  METAL_FUNC size_t size() const ray_data { return __air_get_size_command_buffer(t); }
  METAL_FUNC size_t size() const object_data { return __air_get_size_command_buffer(t); }

  METAL_FUNC compute_command get_compute_command_at(uint index) const thread {
    return compute_command(*(command_buffer*)this, index);
  }
  METAL_FUNC render_command get_render_command_at(uint index) const thread {
    return render_command(*(command_buffer*)this, index);
  }

  // Resource tracking
  METAL_FUNC ulong get_gpu_address() const thread { return __air_get_gpu_address_indirect_command_buffer(t); }

private:
  __air_command_buffer_t t;
};

// ============================================================================
// indirect_render_command (thin wrapper for ICB render commands)
// ============================================================================
struct indirect_render_command {
  METAL_FUNC indirect_render_command() thread = default;
  METAL_FUNC indirect_render_command(const thread indirect_render_command &) thread = default;
  METAL_FUNC thread indirect_render_command &operator=(const thread indirect_render_command &) thread = default;

  METAL_FUNC void set_render_pipeline_state(render_pipeline_state ps) thread {
    __air_set_render_pipeline_state_indirect_render_command(t, ps.t);
  }
  METAL_FUNC void set_depth_stencil_state(depth_stencil_state ds) thread {
    __air_set_depth_stencil_state_indirect_render_command(t, ds.t);
  }
  template <typename T>
  METAL_FUNC void set_vertex_buffer(device T *buffer, uint index) thread {
    __air_set_vertex_buffer_indirect_render_command(t, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_vertex_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_vertex_buffer_indirect_render_command(t, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_fragment_buffer(device T *buffer, uint index) thread {
    __air_set_fragment_buffer_indirect_render_command(t, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_fragment_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_fragment_buffer_indirect_render_command(t, (const void*)buffer, stride, index);
  }
  METAL_FUNC void draw_primitives(primitive_type type, uint vertex_start, uint vertex_count,
                                   uint instance_count, uint base_instance) thread {
    __air_draw_primitives_indirect_render_command_full(t, (uint)type, vertex_start, vertex_count, instance_count, base_instance);
  }
  METAL_FUNC void draw_indexed_primitives(primitive_type type, uint index_count,
                                            void *index_buffer, uint index_buffer_offset,
                                            uint instance_count, int base_vertex, uint base_instance) thread {
    __air_draw_indexed_primitives_indirect_render_command_full(t, (uint)type, index_count,
                                                                index_buffer, index_buffer_offset,
                                                                instance_count, base_vertex, base_instance);
  }
  METAL_FUNC void reset() thread { __air_reset_indirect_render_command(t); }

private:
  __air_indirect_render_command_t t;
};

// ============================================================================
// indirect_compute_command (thin wrapper for ICB compute commands)
// ============================================================================
struct indirect_compute_command {
  METAL_FUNC indirect_compute_command() thread = default;
  METAL_FUNC indirect_compute_command(const thread indirect_compute_command &) thread = default;
  METAL_FUNC thread indirect_compute_command &operator=(const thread indirect_compute_command &) thread = default;

  METAL_FUNC void set_compute_pipeline_state(compute_pipeline_state ps) thread {
    __air_set_compute_pipeline_state_indirect_compute_command(t, ps.t);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(device T *buffer, uint index) thread {
    __air_set_kernel_buffer_indirect_compute_command(t, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(device T *buffer, size_t stride, uint index) thread {
    __air_set_kernel_buffer_indirect_compute_command(t, (const void*)buffer, stride, index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(constant T *buffer, uint index) thread {
    __air_set_kernel_buffer_indirect_compute_command(t, (const void*)buffer, ~size_t(0), index);
  }
  template <typename T>
  METAL_FUNC void set_kernel_buffer(threadgroup T *buffer, uint index) thread {
    __air_set_kernel_buffer_indirect_compute_command(t, (const void*)buffer, ~size_t(0), index);
  }
  METAL_FUNC void concurrent_dispatch_threadgroups(uint3 tg_per_grid, uint3 threads_per_tg) thread {
    __air_concurrent_dispatch_threadgroups_indirect_compute_command(t, tg_per_grid, threads_per_tg);
  }
  METAL_FUNC void concurrent_dispatch_threads(uint3 threads_per_grid, uint3 threads_per_tg) thread {
    __air_concurrent_dispatch_threads_indirect_compute_command(t, threads_per_grid, threads_per_tg);
  }
  METAL_FUNC void set_barrier() thread { __air_set_barrier_indirect_compute_command(t); }
  METAL_FUNC void clear_barrier() thread { __air_clear_barrier_indirect_compute_command(t); }
  METAL_FUNC void set_threadgroup_memory_length(uint length, uint index) thread {
    __air_set_threadgroup_memory_length_indirect_compute_command(t, length, index);
  }
  METAL_FUNC void reset() thread { __air_reset_indirect_compute_command(t); }

private:
  __air_indirect_compute_command_t t;
};

} // namespace metal
#endif // _METAL_COMMAND_BUFFER_H_
