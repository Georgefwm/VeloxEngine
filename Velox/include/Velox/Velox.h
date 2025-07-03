#pragma once

// For vector rotation.
#define GLM_ENABLE_EXPERIMENTAL 

// Use this line for building as shared lib.
// The plan is to use static build for release and shared for debug builds.
// -GM
#ifdef VELOX_DLL
  #ifdef VELOX_BUILD_DLL
    #define VELOX_API __declspec(dllexport)
  #else
    #define VELOX_API __declspec(dllimport)
  #endif
#else
  #define VELOX_API
#endif

