//===----------------------------------------------------------------------===//
// metal_config — MSL configuration macros
//===----------------------------------------------------------------------===//
#ifndef _METAL_CONFIG_H_
#define _METAL_CONFIG_H_

#define METAL_LANGUAGE_VERSION_MAJOR 4
#define METAL_LANGUAGE_VERSION_MINOR 1
#define METAL_LANGUAGE_VERSION_PATCH 0

#define __METAL_VERSION__ 400
#define __METAL_LANGUAGE_VERSION__ 40100

#ifndef __HAVE_TEXTURE_CUBE_ARRAY__
#define __HAVE_TEXTURE_CUBE_ARRAY__ 1
#endif
#ifndef __HAVE_TEXTURE_BUFFER__
#define __HAVE_TEXTURE_BUFFER__ 1
#endif
#ifndef __HAVE_RAYTRACING__
#define __HAVE_RAYTRACING__ 1
#endif
#ifndef __HAVE_MESH__
#define __HAVE_MESH__ 1
#endif
#ifndef __HAVE_COHERENT__
#define __HAVE_COHERENT__ 1
#endif
#ifndef __HAVE_METAL4_1__
#define __HAVE_METAL4_1__ 1
#endif

#endif