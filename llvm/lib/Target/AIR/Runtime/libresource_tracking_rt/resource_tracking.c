// libresource_tracking_rt — Resource tracking runtime
// Tracks buffer/texture/sampler usage for automatic dependency management
typedef unsigned uint;

void __metal_resource_tracking_begin(void *resource, uint usage) {}
void __metal_resource_tracking_end(void *resource) {}
void __metal_resource_tracking_barrier(void *resource, uint from_usage, uint to_usage) {}
uint __metal_resource_tracking_get_usage(void *resource) { return 0; }
uint __metal_resource_tracking_get_last_write(void *resource) { return 0; }
void __metal_resource_tracking_set_alias(void *a, void *b) {}
void __metal_resource_tracking_clear_alias(void *a) {}
void __metal_resource_tracking_mark_complete(void *resource) {}
int  __metal_resource_tracking_is_complete(void *resource) { return 1; }
