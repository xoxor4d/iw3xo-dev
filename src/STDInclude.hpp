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
#include <d3d9.h>
#include <timeapi.h>
#include <shellapi.h>
#include <WinSock2.h>
#include <assert.h>
#include <filesystem>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d3d9.lib")

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
#include <locale>
#include <regex>
#include <thread>
#include <chrono>
#include <future>
#include <unordered_map>
#pragma warning(pop)

// Revision number
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define AssertSize(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")

#include <glm.hpp>

#include "Game/Structs.hpp"
#include "Utils/vector.hpp"
#include "Utils/Utils.hpp"
#include "Utils/polylib.hpp"
#include "Utils/Memory.hpp"
#include "Utils/Stream.hpp"
#include "Utils/Hooking.hpp"
#include "Utils/Entities.hpp"
#include "Utils/function.hpp"

#include "Game/Functions.hpp"
#include "Game/Dvars.hpp"

#include "Components/Loader.hpp"

using namespace std::literals;
