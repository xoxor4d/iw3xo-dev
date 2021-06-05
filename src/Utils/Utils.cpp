#include "STDInclude.hpp"

#define VA_BUFFER_COUNT		64
#define VA_BUFFER_SIZE		65536

namespace Utils
{
	int try_stoi(const std::string str, bool quite)
	{
		int ret = 0;

		try
		{
			ret = std::stoi(str);
		}
		catch (const std::invalid_argument)
		{
			if (!quite)
			{
				Game::Com_PrintMessage(0, Utils::VA("[!] (%s) is not a valid argument! Defaulting to integer 0!\n", str.c_str()), 0);
			}
		}

		return ret;
	}

	float try_stof(const std::string str, bool quite)
	{
		float ret = 0.0f;

		try
		{
			ret = std::stof(str);
		}
		catch (const std::invalid_argument)
		{
			if (!quite)
			{
				Game::Com_PrintMessage(0, Utils::VA("[!] (%s) is not a valid argument! Defaulting to float 0.0f!\n", str.c_str()), 0);
			}
		}

		return ret;
	}

	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimer()
	{
		return std::chrono::high_resolution_clock::now();
	}

	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimerPrint(const char *string /*no fmt*/)
	{
		Game::Com_PrintMessage(0, string, 0);
		return std::chrono::high_resolution_clock::now();
	}

	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimerPrintToFile(std::ofstream &file, const char* string /*no fmt*/)
	{
		if(string)
			file << string;

		return std::chrono::high_resolution_clock::now();
	}

	void Clock_EndTimerPrintSeconds(std::chrono::time_point<std::chrono::steady_clock> clockStart, const char *string /*%.4f fmt for seconds*/)
	{
		auto clockEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = clockEnd - clockStart;
		Game::Com_PrintMessage(0, Utils::VA(string, elapsed.count()), 0);
	}

	void Clock_EndTimerPrintSecondsToFile(std::ofstream& file, std::chrono::time_point<std::chrono::steady_clock> clockStart, const char* string /*%.4f fmt for seconds*/)
	{
		auto clockEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = clockEnd - clockStart;

		if(string)
			file << Utils::VA(string, elapsed.count()) << std::endl;
	}

	int Q_strncasecmp(char *s1, char *s2, int n)
	{
		int		c1, c2;

		do
		{
			c1 = *s1++;
			c2 = *s2++;

			if (!n--)
				return 0;		// strings are equal until end point

			if (c1 != c2)
			{
				if (c1 >= 'a' && c1 <= 'z')
					c1 -= ('a' - 'A');
				if (c2 >= 'a' && c2 <= 'z')
					c2 -= ('a' - 'A');
				if (c1 != c2)
					return -1;		// strings not equal
			}
		} while (c1);

		return 0;		// strings are equal
	}

	int Q_stricmp(char *s1, char *s2)
	{
		return Q_strncasecmp(s1, s2, 99999);
	}

	void Q_strncpyz(char *dest, const char *src, int destsize) 
	{
		strncpy(dest, src, destsize - 1);
		dest[destsize - 1] = 0;
	}

	int Q_stricmpn(const char *s1, const char *s2, int n)
	{
		int		c1, c2;

		if (s1 == NULL) {
			if (s2 == NULL)
				return 0;
			else
				return -1;
		}
		else if (s2 == NULL)
			return 1;



		do {
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) {
				return 0;		// strings are equal until end point
			}

			if (c1 != c2) {
				if (c1 >= 'a' && c1 <= 'z') {
					c1 -= ('a' - 'A');
				}
				if (c2 >= 'a' && c2 <= 'z') {
					c2 -= ('a' - 'A');
				}
				if (c1 != c2) {
					return c1 < c2 ? -1 : 1;
				}
			}
		} while (c1);

		return 0;		// strings are equal
	}

	int Q_stricmp(const char *s1, const char *s2)
	{
		return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
	}

	// ---------------------------------------------------

	std::string convertToString(char* a, int size)
	{
		int i;
		std::string s = "";
		for (i = 0; i < size; i++) {
			s = s + a[i];
		}
		return s;
	}

	std::string convertToString(const char* a, int size)
	{
		int i;
		std::string s = "";
		for (i = 0; i < size; i++) {
			s = s + a[i];
		}
		return s;
	}

	bool has_suffix(const std::string &str, const std::string &suffix)
	{
		return str.size() >= suffix.size() &&
			str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	bool replace(std::string& str, const std::string& from, const std::string& to)
	{
		size_t start_pos = str.find(from);

		if (start_pos == std::string::npos)
		{
			return false;
		}

		str.replace(start_pos, from.length(), to);
		return true;
	}

	void replaceAll(std::string& source, const std::string& from, const std::string& to)
	{
		std::string newString;
		newString.reserve(source.length());  // avoids a few memory allocations

		std::string::size_type lastPos = 0;
		std::string::size_type findPos;

		while (std::string::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		// Care for the rest after last occurrence
		newString += source.substr(lastPos);

		source.swap(newString);
	}

	// used to extract Integers from dvar strings
	void extractIntegerWords(std::string str, std::vector<int> &Integers, bool checkForDuplicates)
	{
		std::stringstream ss;

		//Storing the whole string into string stream
		ss << str;

		// Running loop till the end of the stream
		std::string temp;
		int found;

		while (!ss.eof())
		{
			// extracting word by word from stream 
			ss >> temp;

			// Checking the given word is integer or not
			if (std::stringstream(temp) >> found) 
			{
				if (checkForDuplicates)
				{
					// check if we added the integer already
					if (std::find(Integers.begin(), Integers.end(), found) == Integers.end()) 
					{
						// new integer
						Integers.push_back(found);
					}
				}

				else 
				{
					//cout << found << " ";
					Integers.push_back(found);
				}
			}

			// To save from space at the end of string
			temp = "";
		}
	}

	// used to extract Integers from dvar strings
	int extractFirstIntegerFromString(std::string str)
	{
		std::stringstream ss;

		//Storing the whole string into string stream
		ss << str;

		// Running loop till the end of the stream
		std::string temp;
		int found;

		while (!ss.eof())
		{
			// extracting word by word from stream 
			ss >> temp;

			// Checking the given word is integer or not
			if (std::stringstream(temp) >> found)
			{
					return found;
			}

			// To save from space at the end of string
			temp = "";
		}

		return 0;
	}

	//----------------- original range ------------------------ desired range --------------- value to convert
	float floatToRange(	float originalStart, float originalEnd, float newStart, float newEnd, float value)                            
	{
		if (value == originalStart)
			return newStart;
		if (value == originalEnd)
			return newEnd;

		float orgDiff	= originalEnd - originalStart;
		float newDiff	= newEnd - newStart;

		return			((newDiff / orgDiff) * value) + newStart;
	}

	//------------ original range -------------------- desired range ----------- value to convert
	int intToRange(int originalStart, int originalEnd, int newStart, int newEnd, int value)
	{
		float orgDiff		= (float)(originalEnd - originalStart);
		float newDiff		= (float)(newEnd - newStart);

		return	(int)(((newDiff / orgDiff) * value) + newStart);
	}

	float fmaxOf3(float input, float arg1, float arg2)
	{
		float x, output = arg1;

		if (input - arg2 < 0.0f) {
			x = input;
		}

		else {
			x = arg2;
		}

		if (0.0f > arg1 - input) {
			output = x;
		}

		return output;
	}

	const char *VA(const char *fmt, ...)
	{
		static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int g_vaNextBufferIndex = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = g_vaBuffer[g_vaNextBufferIndex];
		vsnprintf(g_vaBuffer[g_vaNextBufferIndex], VA_BUFFER_SIZE, fmt, ap);
		g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	std::string StrToLower(std::string input)
	{
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		return input;
	}

	bool EndsWith(std::string haystack, std::string needle)
	{
		return (strstr(haystack.data(), needle.data()) == (haystack.data() + haystack.size() - needle.size()));
	}

	// Complementary function for memset, which checks if a memory is set
	bool MemIsSet(void* mem, char chr, size_t length)
	{
		char* memArr = reinterpret_cast<char*>(mem);

		for (size_t i = 0; i < length; ++i)
		{
			if (memArr[i] != chr)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<std::string> Explode(const std::string& str, char delim)
	{
		std::vector<std::string> result;
		std::istringstream iss(str);

		for (std::string token; std::getline(iss, token, delim);)
		{
			std::string _entry = std::move(token);

			// Remove trailing 0x0 bytes
			while (_entry.size() && !_entry[_entry.size() - 1])
			{
				_entry = _entry.substr(0, _entry.size() - 1);
			}

			result.push_back(_entry);
		}

		return result;
	}

	void Replace(std::string &string, std::string find, std::string replace)
	{
		size_t nPos = 0;

		while ((nPos = string.find(find, nPos)) != std::string::npos)
		{
			string = string.replace(nPos, find.length(), replace);
			nPos += replace.length();
		}
	}

	bool Contains(std::string haystack, std::string needle)
	{
		if (haystack.find(needle) != std::string::npos) {
			return true;
		}

		return false;
	}

	bool StartsWith(std::string haystack, std::string needle)
	{
		return (haystack.size() >= needle.size() && !strncmp(needle.data(), haystack.data(), needle.size()));
	}

	void EraseSubstring(std::string &base, std::string replace)
	{
		auto it = base.find(replace.c_str());
		if (it != std::string::npos)
		{
			base.erase(it, replace.size());
		}
	}

	unsigned int OneAtATime(const char *key, size_t len)
	{
		unsigned int hash, i;
		for (hash = i = 0; i < len; ++i)
		{
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	int IsSpace(int c)
	{
		if (c < -1) return 0;
		return _isspace_l(c, nullptr);
	}

	// trim from start
	std::string &LTrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int val)
		{
			return !IsSpace(val);
		}));
		return s;
	}

	// trim from end
	std::string &RTrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int val)
		{
			return !IsSpace(val);
		}).base(), s.end());
		return s;
	}

	// trim from both ends
	std::string &Trim(std::string &s)
	{
		return LTrim(RTrim(s));
	}

	std::string FormatTimeSpan(int milliseconds)
	{
		int secondsTotal = milliseconds / 1000;
		int seconds = secondsTotal % 60;
		int minutesTotal = secondsTotal / 60;
		int minutes = minutesTotal % 60;
		int hoursTotal = minutesTotal / 60;

		return Utils::VA("%02d:%02d:%02d", hoursTotal, minutes, seconds);
	}

	std::string ParseChallenge(std::string data)
	{
		auto pos = data.find_first_of("\n ");
		if (pos == std::string::npos) return data;
		return data.substr(0, pos).data();
	}

	void CreateDir(std::string dir)
	{
		// Win32 compatibility
		for (unsigned int i = 0; i < dir.size(); ++i)
		{
			if (dir[i] == '/')
			{
				dir[i] = '\\';
			}
		}

		// Terminate the path.
		for (auto i = dir.begin(); i != dir.end(); ++i)
		{
			if(*i == '\\')
			{
				CreateDirectoryW(std::wstring(dir.begin(), i).data(), nullptr);
			}
		}

		CreateDirectoryW(std::wstring(dir.begin(), dir.end()).data(), nullptr);
	}

	bool FileExists(std::string file)
	{
		std::ifstream f(file);
		return f.good();
	}

	bool FileExists(const char* file)
	{
		std::ifstream f(file);
		return f.good();
	}

	void WriteFile(std::string file, std::string data)
	{
		auto pos = file.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			CreateDir(file.substr(0, pos));
		}

		std::ofstream stream(file, std::ios::binary);

		if (stream.is_open())
		{
			stream.write(data.data(), data.size());
			stream.close();
		}
	}

	std::string ReadFile(std::string file)
	{
		std::string buffer;

		if (FileExists(file))
		{
			std::streamsize size;
			std::ifstream stream(file, std::ios::binary);
			if (!stream.is_open()) return buffer;

			stream.seekg(0, std::ios::end);
			size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				buffer.clear();
				buffer.resize(static_cast<uint32_t>(size));

				stream.read(const_cast<char*>(buffer.data()), size);
			}

			stream.close();
		}

		return buffer;
	}

	// Infostring class
	void InfoString::Set(std::string key, std::string value)
	{
		this->KeyValuePairs[key] = value;
	}

	std::string InfoString::Get(std::string key)
	{
		if (this->KeyValuePairs.find(key) != this->KeyValuePairs.end())
		{
			return this->KeyValuePairs[key];
		}

		return "";
	}

	std::string InfoString::Build()
	{
		std::string infoString;

		bool first = true;

		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); ++i)
		{
			if (first) first = false;
			else infoString.append("\\");

			infoString.append(i->first); // Key
			infoString.append("\\");
			infoString.append(i->second); // Value
		}

		return infoString;
	}

	void InfoString::Dump()
	{
		for (auto i = this->KeyValuePairs.begin(); i != this->KeyValuePairs.end(); ++i)
		{
			OutputDebugStringA(Utils::VA("%s: %s", i->first.data(), i->second.data()));
		}
	}

	void InfoString::Parse(std::string buffer)
	{
		if (buffer[0] == '\\')
		{
			buffer = buffer.substr(1);
		}

		std::vector<std::string> KeyValues = Utils::Explode(buffer, '\\');

		for (unsigned int i = 0; i < (KeyValues.size() - 1); i+=2)
		{
			this->KeyValuePairs[KeyValues[i]] = KeyValues[i + 1];
		}
	}

	void byte4_pack_rgba(const float* from, char* to)
	{
		for (auto i = 0; i < 4; i++)
		{
			double pack = (float)(255.0 * from[i]) + 9.313225746154785e-10;
			
			if ((signed int)pack < 0) {
				pack = 0.0;
			}

			if ((signed int)pack > 255) {
				pack = 255.0;
			}

			to[i] = (char)pack;
		}
	}

	void byte4_unpack_rgba(const char* from, float* to)
	{
		to[0] = (float)*((unsigned __int8*)from + 0) * 0.0039215689f;
		to[1] = (float)*((unsigned __int8*)from + 1) * 0.0039215689f;
		to[2] = (float)*((unsigned __int8*)from + 2) * 0.0039215689f;
		to[3] = (float)*((unsigned __int8*)from + 3) * 0.0039215689f;
	}
}
