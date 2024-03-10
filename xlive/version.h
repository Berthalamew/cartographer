#pragma once

// DO NOT USE VERSIONS ABOVE USHORT_MAX - 1 (above 65534) OR BELLOW 0 
// REPLACE THE DEFAULTS FOR BOTH EXECUTABLE_VERSION AND COMPATIBLE_VERSION WITH THE SAME VALUE FOR THE MOST PART
// OR YOU CAN SET THE COMPATIBLE VERSION HIGHER IF YOU WANT TO ALLOW OLDER VERSIONS TO JOIN (EXECUTABLE_VERSION SHOULD NOT BE ABOVE IT)
// ALSO THE VERSION SHOULD BE MODIFIED INCREMENTALLY, NOT RANDOMLY
#define EXECUTABLE_VERSION 12272
#define COMPATIBLE_VERSION 12272

// DO NOT CHANGE, DO NOT USE TYPES ABOVE 7 OR BELLOW 0 (3 bits values max)
#define EXECUTABLE_TYPE 4

// DLL VERSION
#define DLL_VERSION_MAJOR               0
#define DLL_VERSION_MINOR               6
#define DLL_VERSION_REVISION            9
#define DLL_VERSION_BUILD				0

#define DLL_VERSION            DLL_VERSION_MAJOR, DLL_VERSION_MINOR, DLL_VERSION_REVISION, DLL_VERSION_BUILD

#define _CART_VER_STRINGIFY2(s) #s
#define _CART_VER_STRINGIFY(s) _CART_VER_STRINGIFY2(s)
#define DLL_VERSION_STR _CART_VER_STRINGIFY(DLL_VERSION_MAJOR) "." _CART_VER_STRINGIFY(DLL_VERSION_MINOR) "." _CART_VER_STRINGIFY(DLL_VERSION_REVISION) "." _CART_VER_STRINGIFY(DLL_VERSION_BUILD) "\0"
