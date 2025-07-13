#include "std_include.hpp"

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

	bool hook::conditional_jump_to_jmp(const DWORD place)
	{
		// map for long conditional jumps (6 bytes: 2-byte opcode + 4-byte offset)
		static const std::unordered_map<std::uint16_t, const char*> long_jump_opcodes = {
			{ static_cast<std::uint16_t>(0x800F), "JO" },   // 0F 80
			{ static_cast<std::uint16_t>(0x810F), "JNO" },  // 0F 81
			{ static_cast<std::uint16_t>(0x820F), "JB" },   // 0F 82
			{ static_cast<std::uint16_t>(0x830F), "JAE" },  // 0F 83
			{ static_cast<std::uint16_t>(0x840F), "JZ" },   // 0F 84
			{ static_cast<std::uint16_t>(0x850F), "JNZ" },  // 0F 85
			{ static_cast<std::uint16_t>(0x860F), "JBE" },  // 0F 86
			{ static_cast<std::uint16_t>(0x870F), "JA" },   // 0F 87
			{ static_cast<std::uint16_t>(0x880F), "JS" },   // 0F 88
			{ static_cast<std::uint16_t>(0x890F), "JNS" },  // 0F 89
			{ static_cast<std::uint16_t>(0x8A0F), "JP" },   // 0F 8A
			{ static_cast<std::uint16_t>(0x8B0F), "JNP" },  // 0F 8B
			{ static_cast<std::uint16_t>(0x8C0F), "JL" },   // 0F 8C
			{ static_cast<std::uint16_t>(0x8D0F), "JNL" },  // 0F 8D
			{ static_cast<std::uint16_t>(0x8E0F), "JLE" },  // 0F 8E
			{ static_cast<std::uint16_t>(0x8F0F), "JG" }    // 0F 8F
		};

		static const std::unordered_map<std::uint8_t, const char*> short_jump_opcodes = {
			{ static_cast<std::uint8_t>(0x70), "JO" },   // Jump if overflow
			{ static_cast<std::uint8_t>(0x71), "JNO" },  // Jump if not overflow
			{ static_cast<std::uint8_t>(0x72), "JB" },   // JB/JNAE/JC
			{ static_cast<std::uint8_t>(0x73), "JAE" },  // JAE/JNB/JNC
			{ static_cast<std::uint8_t>(0x74), "JZ" },   // JE/JZ
			{ static_cast<std::uint8_t>(0x75), "JNZ" },  // JNE/JNZ
			{ static_cast<std::uint8_t>(0x76), "JBE" },  // JBE/JNA
			{ static_cast<std::uint8_t>(0x77), "JA" },   // JA/JNBE
			{ static_cast<std::uint8_t>(0x78), "JS" },   // Jump if sign
			{ static_cast<std::uint8_t>(0x79), "JNS" },  // Jump if not sign
			{ static_cast<std::uint8_t>(0x7A), "JP" },   // JP/JPE
			{ static_cast<std::uint8_t>(0x7B), "JNP" },  // JNP/JPO
			{ static_cast<std::uint8_t>(0x7C), "JL" },   // JL/JNGE
			{ static_cast<std::uint8_t>(0x7D), "JNL" },  // JGE/JNL
			{ static_cast<std::uint8_t>(0x7E), "JLE" },  // JLE/JNG
			{ static_cast<std::uint8_t>(0x7F), "JG" }    // JG/JNLE
		};

		// read the first 2 bytes
		const std::uint8_t* code = (std::uint8_t*)place;
		const std::uint16_t first_two_bytes = (code[1] << 8) | code[0]; // Little-endian

		const char* jump_name = nullptr;
		std::int32_t target_address = 0;
		std::uint8_t new_instruction[6] = { 0 };
		size_t instruction_size = 0;

		// Check if it's a long conditional jump (6 bytes)
		if (long_jump_opcodes.contains(first_two_bytes))
		{
			jump_name = long_jump_opcodes.at(first_two_bytes);
			instruction_size = 6;

			// read the 4-byte relative offset (little-endian)
			std::int32_t jmp_offset;
			std::memcpy(&jmp_offset, code + 2, sizeof(jmp_offset));

			// calculate target address: instruction_address + 6 + jmp_offset
			target_address = place + 6 + jmp_offset;

			// calculate JMP relative offset: target_address - (instruction_address + 5)
			std::int32_t new_jmp_offset = target_address - (place + 5);

			// prepare new instruction: E9 + new_jmp_offset + 90
			new_instruction[0] = 0xE9; // JMP
			new_instruction[1] = static_cast<std::uint8_t>(new_jmp_offset & 0xFF);
			new_instruction[2] = static_cast<std::uint8_t>((new_jmp_offset >> 8) & 0xFF);
			new_instruction[3] = static_cast<std::uint8_t>((new_jmp_offset >> 16) & 0xFF);
			new_instruction[4] = static_cast<std::uint8_t>((new_jmp_offset >> 24) & 0xFF);
			new_instruction[5] = 0x90; // NOP
		}
		// check if it's a short conditional jump (2 bytes)
		else if (short_jump_opcodes.contains(code[0]))
		{
			jump_name = short_jump_opcodes.at(code[0]);
			instruction_size = 2;

			// read the 1-byte relative offset (signed)
			std::int8_t jmp_offset = static_cast<std::int8_t>(code[1]);

			// calculate target address: instruction_address + 2 + jmp_offset
			target_address = place + 2 + jmp_offset;

			// calculate short JMP relative offset: target_address - (instruction_address + 2)
			std::int32_t new_jmp_offset = target_address - (place + 2);

			// check if the offset fits in a short jump (�127 bytes)
			if (new_jmp_offset < -128 || new_jmp_offset > 127)
			{
				//#if DEBUG
				std::cout << "[HOOK][conditional_jump_to_jmp] Target address 0x" << std::hex << target_address << " is out of range for a short JMP at 0x" << place << std::dec << std::endl;
				//#endif
				return false;
			}

			// Prepare new instruction: EB + 1-byte offset
			new_instruction[0] = 0xEB; // Short JMP
			new_instruction[1] = static_cast<std::uint8_t>(new_jmp_offset & 0xFF);
		}
		else
		{
			//#if DEBUG
			std::cout << "[HOOK][conditional_jump_to_jmp] Instruction at 0x" << std::hex << place << " is not a supported conditional jump (opcode: " << std::hex << first_two_bytes << ")" << std::dec << std::endl;
			//#endif
			return false;
		}

#if DEBUG
		// log old bytes
		std::cout << "[HOOK][conditional_jump_to_jmp] Old bytes at 0x" << std::hex << place << ": ";
		for (size_t i = 0; i < instruction_size; i++) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)code[i] << " ";
		}
		std::cout << std::dec << std::endl;
#endif

		// change memory protection
		DWORD old_protect;
		if (!VirtualProtect((void*)place, instruction_size, PAGE_EXECUTE_READWRITE, &old_protect))
		{
			//#if DEBUG
			std::cout << "[HOOK][conditional_jump_to_jmp] Failed to change memory protection at 0x" << std::hex << place << std::dec << std::endl;
			//#endif
			return false;
		}

		// write new instruction
		std::memcpy((void*)place, new_instruction, instruction_size);

		// restore original protection
		VirtualProtect((void*)place, instruction_size, old_protect, &old_protect);

		// flush instruction cache
		FlushInstructionCache(GetCurrentProcess(), (void*)place, instruction_size);

#if DEBUG
		std::cout << "[HOOK][conditional_jump_to_jmp] Patched " << jump_name << " to JMP at 0x" << std::hex << place << ": ";
		for (size_t i = 0; i < instruction_size; i++) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)new_instruction[i] << " ";
		}
		std::cout << std::dec << std::endl;
#endif
		return true;
	}
}
