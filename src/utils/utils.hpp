#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace utils
{
	int		try_stoi(const std::string& str, bool quite);
	float	try_stof(const std::string& str, bool quite);

	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer();
	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer_print(const char *string /*no fmt*/);
	std::chrono::time_point<std::chrono::steady_clock> clock_start_timer_print_to_file(std::ofstream& file, const char* string /*no fmt*/);
	
	void clock_end_timer_print_seconds(std::chrono::time_point<std::chrono::steady_clock> clockStart, const char *string /*%.4f fmt for seconds*/);
	void clock_end_timer_print_seconds_to_file(std::ofstream& file, std::chrono::time_point<std::chrono::steady_clock> clockStart, const char* string /*%.4f fmt for seconds*/);

	// --------------------------------------------------

	int	q_strncasecmp(char *s1, char *s2, int n);
	int	q_stricmp(char *s1, char *s2);
	void q_strncpyz(char *dest, const char *src, int destsize);
	int	q_stricmpn(const char *s1, const char *s2, int n);
	int	q_stricmp(const char *s1, const char *s2);

	const char *va(const char *fmt, ...);
	std::string convert_to_string(char* a, int size);
	std::string convert_to_string(const char* a, int size);
	std::string convert_wstring(const std::wstring& wstr);
	bool string_equals(const char* s1, const char* s2);
	bool contains(std::string haystack, std::string needle);
	bool has_suffix(const std::string &str, const std::string &suffix);
	bool starts_with(std::string haystack, std::string needle);
	bool ends_with(std::string haystack, std::string needle);
	bool replace(std::string& str, const std::string& from, const std::string& to);
	void replace_all(std::string& source, const std::string& from, const std::string& to);
	void erase_substring(std::string &base, std::string replace);

	std::string& ltrim(std::string& s);
	std::string& rtrim(std::string& s);
	std::string& trim(std::string& s);

	std::string str_to_lower(std::string input);
	std::vector<std::string> explode(const std::string& str, char delim);
	void extract_integer_words(std::string str, std::vector<int> &Integers, bool checkForDuplicates);
	int  extract_first_integer_from_string(std::string str);
	
	float float_to_range(float original_start, float original_end, float new_start, float new_end, float value);
	int   int_to_range(int original_start, int original_end, int new_start, int new_end, int value);
	float fmaxf3(float input, float arg1, float arg2);

	unsigned int one_at_a_time(const char *key, size_t len);
	
	bool file_exists(std::string file);
	bool file_exists(const char* file);

	bool mem_is_set(void* mem, char chr, size_t length);

	void byte3_pack_rgba(const float* from, char* to);
	void byte3_unpack_rgba(const char* from, float* to);
	void byte4_pack_rgba(const float* from, char* to);
	void byte4_unpack_rgba(const char* from, float* to);

	bool get_html(const std::string& url, std::wstring& header, std::wstring& hmtl);

	template <typename T> using Slot = std::function<T>;
	template <typename T>
	class Signal
	{
	public:
		void connect(Slot<T> slot)
		{
			slots.push_back(slot);
		}

		void clear()
		{
			slots.clear();
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			for (auto& slot : slots)
			{
				slot(std::forward<Args>(args)...);
			}
		}

	private:
		std::vector<Slot<T>> slots;
	};
}
