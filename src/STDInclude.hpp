#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#ifndef DEBUG
#define DEBUG 0
#endif

// Version number
#include <version.hpp>
// Game definitions
#include <defines.hpp>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>
#include <wincrypt.h>
#include <time.h>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <d3d9.h>
#include <d3dx9.h>
#pragma warning(pop)

#include <timeapi.h>
#include <shellapi.h>
#include <WinSock2.h>
#include <assert.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#pragma warning(push)
#pragma warning(disable: 4244)
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <iostream> 
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <functional>
#include <cctype>
#include <filesystem>
#include <locale>
#include <regex>
#include <thread>
#include <chrono>
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <tchar.h>
#pragma warning(pop)

// Revision number
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define AssertSize(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")

#include <glm.hpp>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_md.h>
#include <WinHttpClient.h>

#include "Game/Structs.hpp"
#include "utils/vector.hpp"
#include "utils/utils.hpp"
#include "utils/polylib.hpp"
#include "utils/memory.hpp"
#include "utils/hooking.hpp"
#include "utils/Entities.hpp"
#include "utils/function.hpp"

#include "Game/Functions.hpp"
#include "Game/dvars.hpp"

#include "Components/Fonts.hpp"
#include "Components/loader.hpp"

using namespace std::literals;
