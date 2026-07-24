// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

struct Payload { int x; };

kernel void good_mesh_ray_attrs(device int *out [[buffer(0)]],
                                uint local [[local_index]],
                                uint ident [[id]],
                                Payload objectData [[object]],
                                __metal_mesh_t meshData [[mesh]],
                                Payload payloadData [[payload]],
                                Payload intersectionData [[intersection]],
                                visible_function_table table [[visible]]) {}

kernel void bad_local_index(int local [[local_index]]) {} // expected-error {{'local_index' attribute requires parameter type uint}}
kernel void bad_id(int ident [[id]]) {} // expected-error {{'id' attribute requires parameter type uint}}
kernel void bad_mesh(Payload data [[mesh]]) {} // expected-error {{'mesh' attribute requires parameter type a mesh object type}}
kernel void bad_payload(uint data [[payload]]) {} // expected-error {{'payload' attribute requires parameter type a record type}}
kernel void bad_intersection(uint data [[intersection]]) {} // expected-error {{'intersection' attribute requires parameter type a record type}}
kernel void bad_visible(Payload data [[visible]]) {} // expected-error {{'visible' attribute requires parameter type a visible or intersection function table object type}}

[[object]] void object_entry() {}
[[mesh]] void mesh_entry() {}
[[intersection]] void intersection_entry() {}
[[visible]] void visible_entry() {}

object void object_keyword_entry() {}
mesh void mesh_keyword_entry() {}
intersection void intersection_keyword_entry() {}

vertex fragment void bad_vertex_fragment_conflict() {} // expected-error {{'fragment' and 'vertex' attributes are not compatible}} expected-note {{conflicting attribute is here}}
object mesh void bad_object_mesh_conflict() {} // expected-error {{'mesh' and 'object' attributes are not compatible}} expected-note {{conflicting attribute is here}}
