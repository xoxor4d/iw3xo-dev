#include "std_include.hpp"

#define VA_BUFFER_COUNT		64
#define VA_BUFFER_SIZE		65536

namespace utils
{
	int try_stoi(const std::string& str, bool quite)
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
				game::Com_PrintMessage(0, utils::va("[!] (%s) is not a valid argument! Defaulting to integer 0!\n", str.c_str()), 0);
			}
		}

		return ret;
	}

	float try_stof(const std::string& str, bool quite)
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
				game::Com_PrintMessage(0, utils::va("[!] (%s) is not a valid argument! Defaulting to float 0.0f!\n", str.c_str()), 0);
			}
		}

		return ret;
	}

	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer()
	{
		return std::chrono::high_resolution_clock::now();
	}

	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer_print(const char *string /*no fmt*/)
	{
		game::Com_PrintMessage(0, string, 0);
		return std::chrono::high_resolution_clock::now();
	}

	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer_print_to_file(std::ofstream &file, const char* string /*no fmt*/)
	{
		if(string)
		{
			file << string;
		}

		return std::chrono::high_resolution_clock::now();
	}

	void clock_end_timer_print_seconds(std::chrono::time_point<std::chrono::steady_clock> clockStart, const char *string /*%.4f fmt for seconds*/)
	{
		const auto clockEnd = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double> elapsed = clockEnd - clockStart;
		game::Com_PrintMessage(0, utils::va(string, elapsed.count()), 0);
	}

	void clock_end_timer_print_seconds_to_file(std::ofstream& file, std::chrono::time_point<std::chrono::steady_clock> clockStart, const char* string /*%.4f fmt for seconds*/)
	{
		const auto clockEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = clockEnd - clockStart;

		if(string)
		{
			file << utils::va(string, elapsed.count()) << std::endl;
		}
	}

	int q_strncasecmp(char *s1, char *s2, int n)
	{
		int	c1, c2;

		do
		{
			c1 = *s1++;
			c2 = *s2++;

			if (!n--)
			{
				return 0; // strings are equal until end point
			}

			if (c1 != c2)
			{
				if (c1 >= 'a' && c1 <= 'z')
				{
					c1 -= ('a' - 'A');
				}

				if (c2 >= 'a' && c2 <= 'z')
				{
					c2 -= ('a' - 'A');
				}

				if (c1 != c2)
				{
					return -1; // strings not equal
				}
			}

		} while (c1);

		return 0; // strings are equal
	}

	int q_stricmp(char *s1, char *s2)
	{
		return q_strncasecmp(s1, s2, 99999);
	}

	void q_strncpyz(char *dest, const char *src, int destsize) 
	{
		strncpy(dest, src, destsize - 1);
		dest[destsize - 1] = 0;
	}

	int q_stricmpn(const char *s1, const char *s2, int n)
	{
		int	c1, c2;

		if (s1 == nullptr) 
		{
			if (s2 == nullptr)
			{
				return 0;
			}

			return -1;
			
		}

		if (s2 == nullptr)
		{
			return 1;
		}

		do 
		{
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) 
			{
				return 0; // strings are equal until end point
			}

			if (c1 != c2) 
			{
				if (c1 >= 'a' && c1 <= 'z') 
				{
					c1 -= ('a' - 'A');
				}

				if (c2 >= 'a' && c2 <= 'z') 
				{
					c2 -= ('a' - 'A');
				}

				if (c1 != c2) 
				{
					return c1 < c2 ? -1 : 1;
				}
			}

		} while (c1);

		return 0; // strings are equal
	}

	int q_stricmp(const char *s1, const char *s2)
	{
		return (s1 && s2) ? q_stricmpn(s1, s2, 99999) : -1;
	}

	const char* va(const char* fmt, ...)
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

	std::string convert_to_string(char* a, int size)
	{
		int i;
		std::string s;

		for (i = 0; i < size; i++) 
		{
			s += a[i];
		}

		return s;
	}

	std::string convert_to_string(const char* a, int size)
	{
		int i;
		std::string s;

		for (i = 0; i < size; i++)
		{
			s += a[i];
		}

		return s;
	}

	bool string_equals(const char* s1, const char* s2)
	{
		return !q_stricmp(s1, s2);
	}

	bool contains(std::string haystack, std::string needle)
	{
		if (haystack.find(needle) != std::string::npos)
		{
			return true;
		}

		return false;
	}

	bool has_suffix(const std::string &str, const std::string &suffix)
	{
		return str.size() >= suffix.size() &&
			str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	bool starts_with(std::string haystack, std::string needle)
	{
		return (haystack.size() >= needle.size() && !strncmp(needle.data(), haystack.data(), needle.size()));
	}

	bool ends_with(std::string haystack, std::string needle)
	{
		return (strstr(haystack.data(), needle.data()) == (haystack.data() + haystack.size() - needle.size()));
	}

	bool replace(std::string& str, const std::string& from, const std::string& to)
	{
		const size_t start_pos = str.find(from);

		if (start_pos == std::string::npos)
		{
			return false;
		}

		str.replace(start_pos, from.length(), to);
		return true;
	}

	void replace_all(std::string& source, const std::string& from, const std::string& to)
	{
		std::string new_string;
		new_string.reserve(source.length());  // avoids a few memory allocations

		std::string::size_type last_pos = 0;
		std::string::size_type findPos;

		while (std::string::npos != (findPos = source.find(from, last_pos)))
		{
			new_string.append(source, last_pos, findPos - last_pos);
			new_string += to;
			last_pos = findPos + from.length();
		}

		// Care for the rest after last occurrence
		new_string += source.substr(last_pos);

		source.swap(new_string);
	}

	void erase_substring(std::string& base, std::string replace)
	{
		if (const auto	it = base.find(replace);
			it != std::string::npos)
		{
			base.erase(it, replace.size());
		}
	}

	int is_space(int c)
	{
		if (c < -1)
		{
			return 0;
		}

		return _isspace_l(c, nullptr);
	}

	// trim from start
	std::string& ltrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int val)
			{
				return !is_space(val);
			}));

		return s;
	}

	// trim from end
	std::string& rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int val)
		{
			return !is_space(val);

		}).base(), s.end());

		return s;
	}

	// trim from both ends
	std::string& trim(std::string& s)
	{
		return ltrim(rtrim(s));
	}

	std::string str_to_lower(std::string input)
	{
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		return input;
	}

	std::vector<std::string> explode(const std::string& str, char delim)
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

	// used to extract Integers from dvar strings
	void extract_integer_words(std::string str, std::vector<int> &Integers, bool checkForDuplicates)
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
	int extract_first_integer_from_string(std::string str)
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

	//------------------------------ original range ------------------- desired range ----- value to convert
	float float_to_range(float original_start, float original_end, float new_start, float new_end, float value)                            
	{
		if (value == original_start)
		{
			return new_start;
		}

		if (value == original_end)
		{
			return new_end;
		}

		const float org_diff = original_end - original_start;
		const float new_diff = new_end - new_start;

		return (new_diff / org_diff) * value + new_start;
	}

	//------------ original range -------------------- desired range ----------- value to convert
	int int_to_range(int original_start, int original_end, int new_start, int new_end, int value)
	{
		const int org_diff = original_end - original_start;
		const int new_diff = new_end - new_start;

		return ((new_diff / org_diff) * value + new_start);
	}

	float fmaxf3(float input, float arg1, float arg2)
	{
		float x, output = arg1;

		if (input - arg2 < 0.0f) 
		{
			x = input;
		}
		else 
		{
			x = arg2;
		}

		if (0.0f > arg1 - input) 
		{
			output = x;
		}

		return output;
	}

	// complementary function for memset, which checks if memory is set
	bool mem_is_set(void* mem, char chr, size_t length)
	{
		const char* mem_arr = static_cast<char*>(mem);
		for (size_t i = 0; i < length; ++i)
		{
			if (mem_arr[i] != chr)
			{
				return false;
			}
		}

		return true;
	}

	unsigned int one_at_a_time(const char *key, size_t len)
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

	bool file_exists(std::string file)
	{
		std::ifstream f(file);
		return f.good();
	}

	bool file_exists(const char* file)
	{
		std::ifstream f(file);
		return f.good();
	}

	void byte3_pack_rgba(const float* from, char* to)
	{
		for (auto i = 0; i < 3; i++)
		{
			double pack = (float)(255.0 * static_cast<double>(from[i])) + 9.313225746154785e-10;

			if ((signed int)pack < 0) {
				pack = 0.0;
			}

			if ((signed int)pack > 255) {
				pack = 255.0;
			}

			to[i] = (char)pack;
		}
	}

	void byte4_pack_rgba(const float* from, char* to)
	{
		for (auto i = 0; i < 4; i++)
		{
			double pack = (float)(255.0 * static_cast<double>(from[i])) + 9.313225746154785e-10;
			
			if ((signed int)pack < 0) {
				pack = 0.0;
			}

			if ((signed int)pack > 255) {
				pack = 255.0;
			}

			to[i] = (char)pack;
		}
	}

	void byte3_unpack_rgba(const char* from, float* to)
	{
		to[0] = (float)*((unsigned __int8*)from + 0) * 0.0039215689f;
		to[1] = (float)*((unsigned __int8*)from + 1) * 0.0039215689f;
		to[2] = (float)*((unsigned __int8*)from + 2) * 0.0039215689f;
	}

	void byte4_unpack_rgba(const char* from, float* to)
	{
		to[0] = (float)*((unsigned __int8*)from + 0) * 0.0039215689f;
		to[1] = (float)*((unsigned __int8*)from + 1) * 0.0039215689f;
		to[2] = (float)*((unsigned __int8*)from + 2) * 0.0039215689f;
		to[3] = (float)*((unsigned __int8*)from + 3) * 0.0039215689f;
	}


	bool get_html(const std::string& url, std::wstring& header, std::wstring& hmtl)
	{
		const auto wurl = std::wstring(url.begin(), url.end());
		bool ret = false;

		try
		{
			WinHttpClient client(wurl);
			std::string url_protocol = url.substr(0, 5);

			std::transform(url_protocol.begin(), url_protocol.end(), url_protocol.begin(), (int (*)(int))std::toupper);

			if (url_protocol == "HTTPS")
			{
				client.SetRequireValidSslCertificates(false);
			}

			client.SetUserAgent(L"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:19.0) Gecko/20100101 Firefox/19.0");

			if (client.SendHttpRequest())
			{
				header = client.GetResponseHeader();
				hmtl = client.GetResponseContent();
				ret = true;
			}
		}
		catch (...)
		{
			header = L"Error";
			hmtl = L"";
		}
		return ret;
	}
}
