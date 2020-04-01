#include "STDInclude.hpp"

namespace Utils
{
	Stream::Stream()
	{

	}

	Stream::Stream(size_t size) : Stream()
	{
		this->buffer.reserve(size);
	}

	Stream::~Stream()
	{
		this->buffer.clear();
	};

	size_t Stream::length()
	{
		return this->buffer.length();
	}

	size_t Stream::capacity()
	{
		return this->buffer.capacity();
	}

	char* Stream::save(const void * _str, size_t size, size_t count)
	{
		this->buffer.append(static_cast<const char*>(_str), size * count);

		return this->at() - (size * count);
	}

	char* Stream::save(int value, size_t count)
	{
		auto ret = this->length();

		for (size_t i = 0; i < count; ++i)
		{
			this->save(&value, 4, 1);
		}

		return this->data() + ret;
	}

	char* Stream::saveString(std::string string)
	{
		return this->saveString(string.data()/*, string.size()*/);
	}

	char* Stream::saveString(const char* string)
	{
		return this->saveString(string, strlen(string));
	}

	char* Stream::saveString(const char* string, size_t len)
	{
		auto ret = this->length();

		if (string)
		{
			this->save(string, len);
		}

		this->saveNull();

		return this->data() + ret;
	}

	char* Stream::saveText(std::string string)
	{
		return this->save(string.data(), string.length());
	}

	char* Stream::saveByte(unsigned char byte, size_t count)
	{
		auto ret = this->length();

		for (size_t i = 0; i < count; ++i)
		{
			this->save(&byte, 1);
		}

		return this->data() + ret;
	}

	char* Stream::saveNull(size_t count)
	{
		return this->saveByte(0, count);
	}

	char* Stream::saveMax(size_t count)
	{
		return this->saveByte(static_cast<unsigned char>(-1), count);
	}

	char* Stream::at()
	{
		return reinterpret_cast<char*>(this->data() + this->length());
	}

	Stream::Offset Stream::offset()
	{
		return Stream::Offset(this, this->length());
	}

	char* Stream::data()
	{
		return const_cast<char*>(this->buffer.data());
	}

	void Stream::toBuffer(std::string& outBuffer)
	{
		outBuffer.clear();
		outBuffer.append(this->data(), this->length());
	}

	std::string Stream::toBuffer()
	{
		std::string outBuffer;
		this->toBuffer(outBuffer);
		return outBuffer;
	}

	void Stream::storePointer(void* pointer)
	{
		if (hasPointer(pointer)) throw new std::runtime_error("Pointer already stored!");
		this->pointers.push_back(pointer);
		this->saveObject(pointer);
	}

	bool Stream::hasPointer(void* pointer)
	{
		return std::find(this->pointers.begin(), this->pointers.end(), pointer) != this->pointers.end();
	}
}
