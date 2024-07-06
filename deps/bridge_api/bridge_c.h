// Bridge API header for both the x86 bridge client and the x86 game

#ifndef BRIDGE_C_H_
#define BRIDGE_C_H_

#include <cstdint>
#include <windows.h>

#define BRIDGEAPI_CALL __stdcall
#define BRIDGEAPI_PTR  BRIDGEAPI_CALL

#define BRIDGE_API  __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum bridgeapi_ErrorCode {
    BRIDGEAPI_ERROR_CODE_SUCCESS = 0,
    BRIDGEAPI_ERROR_CODE_GENERAL_FAILURE = 1,
    // WinAPI's GetModuleHandle has failed
    BRIDGEAPI_ERROR_CODE_GET_MODULE_HANDLE_FAILURE = 2,
    BRIDGEAPI_ERROR_CODE_INVALID_ARGUMENTS = 3,
    // Couldn't find 'remixInitialize' function in the .dll
    BRIDGEAPI_ERROR_CODE_GET_PROC_ADDRESS_FAILURE = 4,
    // CreateD3D9 / RegisterD3D9Device can be called only once
    BRIDGEAPI_ERROR_CODE_ALREADY_EXISTS = 5,
    // RegisterD3D9Device requires the device that was created with IDirect3DDevice9Ex, returned by CreateD3D9
    BRIDGEAPI_ERROR_CODE_REGISTERING_NON_REMIX_D3D9_DEVICE = 6,
    // RegisterD3D9Device was not called
    BRIDGEAPI_ERROR_CODE_REMIX_DEVICE_WAS_NOT_REGISTERED = 7,
    BRIDGEAPI_ERROR_CODE_INCOMPATIBLE_VERSION = 8,
    // WinAPI's SetDllDirectory has failed
    //BRIDGEAPI_ERROR_CODE_SET_DLL_DIRECTORY_FAILURE = 9,
    // WinAPI's GetFullPathName has failed
    //BRIDGEAPI_ERROR_CODE_GET_FULL_PATH_NAME_FAILURE = 10,
    BRIDGEAPI_ERROR_CODE_NOT_INITIALIZED = 11,
    // Error codes that are encoded as HRESULT, i.e. returned from D3D9 functions.
    // Look MAKE_D3DHRESULT, but with _FACD3D=0x896, instead of D3D9's 0x876
    //BRIDGEAPI_ERROR_CODE_HRESULT_NO_REQUIRED_GPU_FEATURES = 0x88960001,
    //BRIDGEAPI_ERROR_CODE_HRESULT_DRIVER_VERSION_BELOW_MINIMUM = 0x88960002,
    //BRIDGEAPI_ERROR_CODE_HRESULT_DXVK_INSTANCE_EXTENSION_FAIL = 0x88960003,
    //BRIDGEAPI_ERROR_CODE_HRESULT_VK_CREATE_INSTANCE_FAIL = 0x88960004,
    //BRIDGEAPI_ERROR_CODE_HRESULT_VK_CREATE_DEVICE_FAIL = 0x88960005,
    //BRIDGEAPI_ERROR_CODE_HRESULT_GRAPHICS_QUEUE_FAMILY_MISSING = 0x88960006,
  } BRIDGEAPI_ErrorCode;

  // -----------------------------------------------------------
  // <- remix api types directly grabbed from the remix_c header

  typedef enum remixapi_StructType {
    REMIXAPI_STRUCT_TYPE_NONE = 0,
    REMIXAPI_STRUCT_TYPE_INITIALIZE_LIBRARY_INFO = 1,
    REMIXAPI_STRUCT_TYPE_MATERIAL_INFO = 2,
    REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_PORTAL_EXT = 3,
    REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_TRANSLUCENT_EXT = 4,
    REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_OPAQUE_EXT = 5,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO = 6,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISTANT_EXT = 7,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_CYLINDER_EXT = 8,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISK_EXT = 9,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_RECT_EXT = 10,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT = 11,
    REMIXAPI_STRUCT_TYPE_MESH_INFO = 12,
    REMIXAPI_STRUCT_TYPE_INSTANCE_INFO = 13,
    REMIXAPI_STRUCT_TYPE_INSTANCE_INFO_BONE_TRANSFORMS_EXT = 14,
    REMIXAPI_STRUCT_TYPE_INSTANCE_INFO_BLEND_EXT = 15,
    REMIXAPI_STRUCT_TYPE_CAMERA_INFO = 16,
    REMIXAPI_STRUCT_TYPE_CAMERA_INFO_PARAMETERIZED_EXT = 17,
    REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_OPAQUE_SUBSURFACE_EXT = 18,
    REMIXAPI_STRUCT_TYPE_INSTANCE_INFO_OBJECT_PICKING_EXT = 19,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DOME_EXT = 20,
    REMIXAPI_STRUCT_TYPE_LIGHT_INFO_USD_EXT = 21,
    REMIXAPI_STRUCT_TYPE_STARTUP_INFO = 22,
    REMIXAPI_STRUCT_TYPE_PRESENT_INFO = 23,
    // NOTE: if adding a new struct, register it in 'rtx_remix_specialization.inl'
  } remixapi_StructType;

  namespace x86
  {
    typedef uint32_t remixapi_Bool;

    typedef struct remixapi_Rect2D {
      int32_t left;
      int32_t top;
      int32_t right;
      int32_t bottom;
    } remixapi_Rect2D;

    typedef struct remixapi_Float2D {
      float x;
      float y;
    } remixapi_Float2D;

    typedef struct remixapi_Float3D {
      float x;
      float y;
      float z;
    } remixapi_Float3D;

    typedef struct remixapi_Float4D {
      float x;
      float y;
      float z;
      float w;
    } remixapi_Float4D;

    typedef struct remixapi_Transform {
      float matrix[3][4];
    } remixapi_Transform;

    typedef struct remixapi_LightInfoLightShaping {
      // The direction the Light Shaping is pointing in. Must be normalized.
      remixapi_Float3D               direction;
      float                          coneAngleDegrees;
      float                          coneSoftness;
      float                          focusExponent;
    } remixapi_LightInfoLightShaping;

    typedef struct remixapi_LightInfoSphereEXT {
      remixapi_StructType            sType;
      //void* pNext;
      remixapi_Float3D               position;
      float                          radius;
      remixapi_Bool                  shaping_hasvalue;
      remixapi_LightInfoLightShaping shaping_value;
    } remixapi_LightInfoSphereEXT;

    typedef struct remixapi_LightInfo {
      remixapi_StructType             sType;
      //void* pNext;
      uint64_t                        hash;
      remixapi_Float3D                radiance;
    } remixapi_LightInfo;

    typedef struct remixapi_LightHandle_T* remixapi_LightHandle;
  }

  // -----------------------------------------------------------
  // remix api types end ->


  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_DebugPrint)(const char* text);
  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_Present)(void);
  typedef uint64_t(BRIDGEAPI_PTR* PFN_bridgeapi_CreateSphereLight)(const x86::remixapi_LightInfo* info, x86::remixapi_LightInfoSphereEXT* sphere_info);
  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_DestroyLight)(uint64_t handle);
  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_DrawLightInstance)(uint64_t handle);
  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_SetConfigVariable)(const char* var, const char* value);
  typedef void(BRIDGEAPI_PTR* PFN_bridgeapi_RegisterDevice)(void);

  typedef struct bridgeapi_Interface {
    bool initialized;
    PFN_bridgeapi_DebugPrint		    DebugPrint;
    PFN_bridgeapi_Present		        Present;
    PFN_bridgeapi_CreateSphereLight CreateSphereLight;
    PFN_bridgeapi_DestroyLight      DestroyLight;
    PFN_bridgeapi_DrawLightInstance DrawLightInstance;
    PFN_bridgeapi_SetConfigVariable SetConfigVariable;
    PFN_bridgeapi_RegisterDevice    RegisterDevice;
  } bridgeapi_Interface;

  BRIDGE_API BRIDGEAPI_ErrorCode __cdecl bridgeapi_InitFuncs(bridgeapi_Interface* out_result);
  typedef BRIDGEAPI_ErrorCode(__cdecl* PFN_bridgeapi_InitFuncs)(bridgeapi_Interface* out_result);

  // --------
  // --------

  inline BRIDGEAPI_ErrorCode bridgeapi_initialize(bridgeapi_Interface* out_bridgeInterface) {

    PFN_bridgeapi_InitFuncs pfn_Initialize = nullptr;
  	HMODULE hModule = GetModuleHandleA("d3d9.dll");
	if (hModule) {
	  PROC func = GetProcAddress(hModule, "bridgeapi_InitFuncs");
	  if (func) {
	  	pfn_Initialize = (PFN_bridgeapi_InitFuncs)func;
	  }
	  else {
	  	return BRIDGEAPI_ERROR_CODE_GET_PROC_ADDRESS_FAILURE;
	  }
	  
	  bridgeapi_Interface bridgeInterface = { false };
	  bridgeapi_ErrorCode status = pfn_Initialize(&bridgeInterface);
	  if (status != BRIDGEAPI_ERROR_CODE_SUCCESS) {
	  	return status;
	  }

    bridgeInterface.initialized = true;
	  *out_bridgeInterface = bridgeInterface;
	  return status;
	}

  	return BRIDGEAPI_ERROR_CODE_GET_MODULE_HANDLE_FAILURE;
  }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BRIDGE_C_H_