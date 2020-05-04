#include "STDInclude.hpp"

// Do necessary assertions here
// Some compilers treat them differently which causes a size mismatch

// WinAPI types
AssertSize(DWORD, 4);
AssertSize(WORD, 2);
AssertSize(BYTE, 1);

// 128 bit integers (only x64)
//AssertSize(__int128, 16);
//AssertSize(unsigned __int128, 16);

// 64 bit integers
AssertSize(__int64, 8);
AssertSize(unsigned __int64, 8);
AssertSize(long long, 8);
AssertSize(unsigned long long, 8);
AssertSize(int64_t, 8);
AssertSize(uint64_t, 8);
AssertSize(std::int64_t, 8);
AssertSize(std::uint64_t, 8);

// 64 bit double precision floating point numbers
AssertSize(double, 8);

// 32 bit integers
AssertSize(__int32, 4);
AssertSize(unsigned __int32, 4);
AssertSize(int, 4);
AssertSize(unsigned int, 4);
AssertSize(long, 4);
AssertSize(unsigned long, 4);
AssertSize(int32_t, 4);
AssertSize(uint32_t, 4);
AssertSize(std::int32_t, 4);
AssertSize(std::uint32_t, 4);

// 32 bit single precision floating point numbers
AssertSize(float, 4);

// 16 bit integers
AssertSize(__int16, 2);
AssertSize(unsigned __int16, 2);
AssertSize(short, 2);
AssertSize(unsigned short, 2);
AssertSize(int16_t, 2);
AssertSize(uint16_t, 2);
AssertSize(std::int16_t, 2);
AssertSize(std::uint16_t, 2);

// 8 bit integers
AssertSize(bool, 1);
AssertSize(__int8, 1);
AssertSize(unsigned __int8, 1);
AssertSize(char, 1);
AssertSize(unsigned char, 1);
AssertSize(int8_t, 1);
AssertSize(uint8_t, 1);
AssertSize(std::int8_t, 1);
AssertSize(std::uint8_t, 1);

// Ensure pointers are 4 bytes in size (32-bit)
static_assert(sizeof(intptr_t) == 4 && sizeof(void*) == 4 && sizeof(size_t) == 4, "This doesn't seem to be a 32-bit environment!");

// Disable telemetry data logging
extern "C"
{
	void _cdecl __vcrt_initialize_telemetry_provider() {}
	void _cdecl __telemetry_main_invoke_trigger() {}
	void _cdecl __telemetry_main_return_trigger() {}
	void _cdecl __vcrt_uninitialize_telemetry_provider() {}

	// Enable 'High Performance Graphics'
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
};

// Bridge d3d9
#pragma comment(linker, "/export:Direct3DShaderValidatorCreate9=d3d9.Direct3DShaderValidatorCreate9")
#pragma comment(linker, "/export:PSGPError=d3d9.PSGPError")
#pragma comment(linker, "/export:PSGPSampleTexture=d3d9.PSGPSampleTexture")
#pragma comment(linker, "/export:D3DPERF_BeginEvent=d3d9.D3DPERF_BeginEvent")
#pragma comment(linker, "/export:D3DPERF_EndEvent=d3d9.D3DPERF_EndEvent")
#pragma comment(linker, "/export:D3DPERF_GetStatus=d3d9.D3DPERF_GetStatus")
#pragma comment(linker, "/export:D3DPERF_QueryRepeatFrame=d3d9.D3DPERF_QueryRepeatFrame")
#pragma comment(linker, "/export:D3DPERF_SetMarker=d3d9.D3DPERF_SetMarker")
#pragma comment(linker, "/export:D3DPERF_SetOptions=d3d9.D3DPERF_SetOptions")
#pragma comment(linker, "/export:D3DPERF_SetRegion=d3d9.D3DPERF_SetRegion")
#pragma comment(linker, "/export:DebugSetLevel=d3d9.DebugSetLevel")
#pragma comment(linker, "/export:DebugSetMute=d3d9.DebugSetMute")
#pragma comment(linker, "/export:Direct3DCreate9=d3d9.Direct3DCreate9")
#pragma comment(linker, "/export:Direct3DCreate9Ex=d3d9.Direct3DCreate9Ex")
