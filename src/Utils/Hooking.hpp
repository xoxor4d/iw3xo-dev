#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

namespace utils
{
	class hook
	{
	public:

		hook() : initialized(false), installed(false), place(nullptr), stub(nullptr), original(nullptr), useJump(false), protection(0) { ZeroMemory(this->buffer, sizeof(this->buffer)); }

		hook(void* place, void* stub, bool useJump = true) : hook() { this->initialize(place, stub, useJump); }
		hook(void* place, void(*stub)(), bool useJump = true) : hook(place, reinterpret_cast<void*>(stub), useJump) {}

		hook(DWORD place, void* stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), stub, useJump) {}
		hook(DWORD place, DWORD stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
		hook(DWORD place, void(*stub)(), bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}

		~hook();

		hook* initialize(void* place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void(*stub)(), bool useJump = true); // For lambdas
		hook* install(bool unprotect = true, bool keepUnportected = false);
		hook* uninstall(bool unprotect = true);

		void* getAddress();
		void quick();

		template <typename T> static std::function<T> Call(DWORD function)
		{
			return std::function<T>(reinterpret_cast<T*>(function));
		}

		template <typename T> static std::function<T> Call(FARPROC function)
		{
			return Call<T>(reinterpret_cast<DWORD>(function));
		}

		template <typename T> static std::function<T> Call(void* function)
		{
			return Call<T>(reinterpret_cast<DWORD>(function));
		}

		static void set_string(void* place, const char* string, size_t length);
		static void set_string(DWORD place, const char* string, size_t length);

		static void set_string(void* place, const char* string);
		static void set_string(DWORD place, const char* string);

		static void write_string(void* place, const std::string& string);
		static void write_string(DWORD place, const std::string& string);

		static void nop(void* place, size_t length);
		static void nop(DWORD place, size_t length);

		static void redirect_jump(void* place, void* stub);
		static void redirect_jump(DWORD place, void* stub);

		template <typename T> static void set(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) = value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void set(DWORD place, T value)
		{
			return set<T>(reinterpret_cast<void*>(place), value);
		}

	private:
		bool initialized;
		bool installed;

		void* place;
		void* stub;
		void* original;
		char buffer[5];
		bool useJump;

		DWORD protection;

		std::mutex stateMutex;
	};
}
