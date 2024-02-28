#pragma once
#ifdef __cplusplus
#define EXTERN_C extern "C" 
#else
#define EXTERN_C
#endif
#ifdef _MSC_VER
#define EXPORT_API EXTERN_C __declspec(dllexport)
#else
#define EXPORT_API EXTERN_C __attribute__((visibility("default")))
#endif
