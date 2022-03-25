#include "STDInclude.hpp"

namespace utils
{
	hook::~hook()
	{
		if (this->initialized)
		{
			this->uninstall();
		}
	}

	hook* hook::initialize(DWORD _place, void(*_stub)(), bool _useJump)
	{
		return this->initialize(_place, reinterpret_cast<void*>(_stub), _useJump);
	}

	hook* hook::initialize(DWORD _place, void* _stub, bool _useJump)
	{
		return this->initialize(reinterpret_cast<void*>(_place), _stub, _useJump);
	}

	hook* hook::initialize(void* _place, void* _stub, bool _useJump)
	{
		if (this->initialized) return this;
		this->initialized = true;

		this->useJump = _useJump;
		this->place = _place;
		this->stub = _stub;

		this->original = static_cast<char*>(this->place) + 5 + *reinterpret_cast<DWORD*>((static_cast<char*>(this->place) + 1));

		return this;
	}

	hook* hook::install(bool unprotect, bool keepUnportected)
	{
		std::lock_guard<std::mutex> _(this->stateMutex);

		if (!this->initialized || this->installed)
		{
			return this;
		}

		this->installed = true;

		if (unprotect) VirtualProtect(this->place, sizeof(this->buffer), PAGE_EXECUTE_READWRITE, &this->protection);
		std::memcpy(this->buffer, this->place, sizeof(this->buffer));

		char* code = static_cast<char*>(this->place);

		*code = static_cast<char>(this->useJump ? 0xE9 : 0xE8);

		*reinterpret_cast<size_t*>(code + 1) = reinterpret_cast<size_t>(this->stub) - (reinterpret_cast<size_t>(this->place) + 5);

		if (unprotect && !keepUnportected) VirtualProtect(this->place, sizeof(this->buffer), this->protection, &this->protection);

		FlushInstructionCache(GetCurrentProcess(), this->place, sizeof(this->buffer));

		return this;
	}

	void hook::quick()
	{
		if (hook::installed)
		{
			hook::installed = false;
		}
	}

	hook* hook::uninstall(bool unprotect)
	{
		std::lock_guard<std::mutex> _(this->stateMutex);

		if (!this->initialized || !this->installed)
		{
			return this;
		}

		this->installed = false;

		if(unprotect) VirtualProtect(this->place, sizeof(this->buffer), PAGE_EXECUTE_READWRITE, &this->protection);

		std::memcpy(this->place, this->buffer, sizeof(this->buffer));

		if (unprotect) VirtualProtect(this->place, sizeof(this->buffer), this->protection, &this->protection);

		FlushInstructionCache(GetCurrentProcess(), this->place, sizeof(this->buffer));

		return this;
	}

	void* hook::getAddress()
	{
		return this->place;
	}

	void hook::nop(void* place, size_t length)
	{
		DWORD oldProtect;
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &oldProtect);

		memset(place, 0x90, length);

		VirtualProtect(place, length, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void hook::nop(DWORD place, size_t length)
	{
		nop(reinterpret_cast<void*>(place), length);
	}

	void hook::set_string(void* place, const char* string, size_t length)
	{
		DWORD oldProtect;
		VirtualProtect(place, length + 1, PAGE_EXECUTE_READWRITE, &oldProtect);

		strncpy_s(static_cast<char*>(place), length, string, length);

		VirtualProtect(place, length + 1, oldProtect, &oldProtect);
	}

	void hook::set_string(DWORD place, const char* string, size_t length)
	{
		hook::set_string(reinterpret_cast<void*>(place), string, length);
	}

	void hook::set_string(void* place, const char* string)
	{
		hook::set_string(place, string, strlen(static_cast<char*>(place)));
	}

	void hook::set_string(DWORD place, const char* string)
	{
		hook::set_string(reinterpret_cast<void*>(place), string);
	}

	void hook::write_string(void* place, const std::string& string)
	{
		DWORD old_protect;
		VirtualProtect(place, string.size() + 1, PAGE_EXECUTE_READWRITE, &old_protect);

		memcpy(place, &string[0], string.size() + 1);

		VirtualProtect(place, string.size() + 1, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, string.size());
	}

	void hook::write_string(const DWORD place, const std::string& string)
	{
		write_string(reinterpret_cast<void*>(place), string);
	}

	void hook::redirect_jump(void* place, void* stub)
	{
		char* operandPtr = static_cast<char*>(place) + 2;
		int newOperand = reinterpret_cast<int>(stub) - (reinterpret_cast<int>(place) + 6);
		utils::hook::set<int>(operandPtr, newOperand);
	}

	void hook::redirect_jump(DWORD place, void* stub)
	{
		hook::redirect_jump(reinterpret_cast<void*>(place), stub);
	}
}
