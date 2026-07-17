/*  Copyright (c) 2021-2024 Apple, Inc.  All rights reserved.
 *
 *  This header provides the definition of a subset of the macros defined in
 *  the system header `TargetConditionals.h` to be used for Metal.
 *
 *  TARGET_OS_*
 *  These conditionals specify in which Operating System the generated code
 *  will run. Indention is used to show which conditionals are evolutionary
 *  subclasses.
 *
 *  The MAC/WIN32/UNIX conditionals are mutually exclusive.
 *  The IOS/TV/WATCH conditionals are mutually exclusive.
 *
 *  TARGET_OS_WIN32             - Generated code will run on WIN32 API
 *  TARGET_OS_WINDOWS           - Generated code will run on Windows
 *  TARGET_OS_UNIX              - Generated code will run on some Unix (not macOS)
 *  TARGET_OS_LINUX             - Generated code will run on Linux
 *  TARGET_OS_MAC               - Generated code will run on a variant of macOS
 *    TARGET_OS_OSX               - Generated code will run on macOS
 *    TARGET_OS_IPHONE            - Generated code will run on a variant of iOS (firmware, devices, simulator)
 *      TARGET_OS_IOS               - Generated code will run on iOS
 *        TARGET_OS_MACCATALYST       - Generated code will run on macOS
 *      TARGET_OS_TV                - Generated code will run on tvOS
 *      TARGET_OS_WATCH             - Generated code will run on watchOS
 *      TARGET_OS_VISION            - Generated code will run on visionOS
 *    TARGET_OS_SIMULATOR         - Generated code will run on a simulator
 *
 *  NOTE: This header is tailored for the Apple Metal compiler and the
 *  corresponding supported targets.
 */

#ifndef __TARGETCONDITIONALS__
#define __TARGETCONDITIONALS__

#define TARGET_OS_WIN32 0
#define TARGET_OS_WINDOWS 0
#define TARGET_OS_UNIX 0
#define TARGET_OS_LINUX 0
#define TARGET_OS_MAC 1

#if !__has_builtin(__is_target_vendor) || !__has_builtin(__is_target_os) || \
    !__has_builtin(__is_target_environment)
#error "Unsupported compiler!"
#endif

#if !__is_target_vendor(apple)
#error "Unsupported vendor!"
#endif

#if __is_target_os(macosx) || __is_target_os(macos)

#define TARGET_OS_OSX 1
#define TARGET_OS_IPHONE 0
#define TARGET_OS_IOS 0
#define TARGET_OS_VISION 0
#define TARGET_OS_TV 0
#define TARGET_OS_WATCH 0
#define TARGET_OS_SIMULATOR 0
#define TARGET_OS_MACCATALYST 0

#elif __is_target_os(ios)

#define TARGET_OS_OSX 0
#define TARGET_OS_IPHONE 1
#define TARGET_OS_IOS 1
#define TARGET_OS_VISION 0
#define TARGET_OS_TV 0
#define TARGET_OS_WATCH 0
#if __is_target_environment(simulator)
#define TARGET_OS_SIMULATOR 1
#define TARGET_OS_MACCATALYST 0
#elif __is_target_environment(macabi)
#define TARGET_OS_SIMULATOR 0
#define TARGET_OS_MACCATALYST 1
#else
#define TARGET_OS_SIMULATOR 0
#define TARGET_OS_MACCATALYST 0
#endif

#elif __is_target_os(tvos)

#define TARGET_OS_OSX 0
#define TARGET_OS_IPHONE 1
#define TARGET_OS_IOS 0
#define TARGET_OS_VISION 0
#define TARGET_OS_TV 1
#define TARGET_OS_WATCH 0
#if __is_target_environment(simulator)
#define TARGET_OS_SIMULATOR 1
#else
#define TARGET_OS_SIMULATOR 0
#endif
#define TARGET_OS_MACCATALYST 0

#elif __is_target_os(watchos)

#define TARGET_OS_OSX 0
#define TARGET_OS_IPHONE 1
#define TARGET_OS_IOS 0
#define TARGET_OS_VISION 0
#define TARGET_OS_TV 0
#define TARGET_OS_WATCH 1
#if __is_target_environment(simulator)
#define TARGET_OS_SIMULATOR 1
#else
#define TARGET_OS_SIMULATOR 0
#endif
#define TARGET_OS_MACCATALYST 0

#elif __is_target_os(xros)

#define TARGET_OS_OSX 0
#define TARGET_OS_IPHONE 1
#define TARGET_OS_IOS 0
#define TARGET_OS_VISION 1
#define TARGET_OS_TV 0
#define TARGET_OS_WATCH 0
#if __is_target_environment(simulator)
#define TARGET_OS_SIMULATOR 1
#else
#define TARGET_OS_SIMULATOR 0
#endif
#define TARGET_OS_MACCATALYST 0

#else
#error "Unsupported target!"
#endif

#endif /* __TARGETCONDITIONALS__ */
