#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace Utils
{
	int		try_stoi(const std::string str, bool quite);
	float	try_stof(const std::string str, bool quite);

	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimer();
	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimerPrint(const char *string /*no fmt*/);
	std::chrono::time_point<std::chrono::steady_clock> Clock_StartTimerPrintToFile(std::ofstream& file, const char* string /*no fmt*/);
	
	void Clock_EndTimerPrintSeconds(std::chrono::time_point<std::chrono::steady_clock> clockStart, const char *string /*%.4f fmt for seconds*/);
	void Clock_EndTimerPrintSecondsToFile(std::ofstream& file, std::chrono::time_point<std::chrono::steady_clock> clockStart, const char* string /*%.4f fmt for seconds*/);

	// --------------------------------------------------

	int		Q_strncasecmp(char *s1, char *s2, int n);
	int		Q_stricmp(char *s1, char *s2);
	void	Q_strncpyz(char *dest, const char *src, int destsize);
	int		Q_stricmpn(const char *s1, const char *s2, int n);
	int		Q_stricmp(const char *s1, const char *s2);

	// ---------------------------------------------------

	std::string convertToString(char* a, int size);
	std::string convertToString(const char* a, int size);
	bool has_suffix(const std::string &str, const std::string &suffix);
	bool replace(std::string& str, const std::string& from, const std::string& to);
	void replaceAll(std::string& source, const std::string& from, const std::string& to);
	void extractIntegerWords(std::string str, std::vector<int> &Integers, bool checkForDuplicates);
	int  extractFirstIntegerFromString(std::string str);
	
	float floatToRange(float originalStart, float originalEnd, float newStart, float newEnd, float value);
	int   intToRange(int originalStart, int originalEnd, int newStart, int newEnd, int value);
	float fmaxOf3(float input, float arg1, float arg2);

	const char *VA(const char *fmt, ...);
	std::string StrToLower(std::string input);
	bool EndsWith(std::string haystack, std::string needle);
	std::vector<std::string> Explode(const std::string& str, char delim);
	
	void Replace(std::string &string, std::string find, std::string replace);
	bool Contains(std::string haystack, std::string needle);
	bool StartsWith(std::string haystack, std::string needle);
	void EraseSubstring(std::string &base, std::string replace);

	unsigned int OneAtATime(const char *key, size_t len);

	std::string &LTrim(std::string &s);
	std::string &RTrim(std::string &s);
	std::string &Trim(std::string &s);

	std::string FormatTimeSpan(int milliseconds);
	std::string ParseChallenge(std::string data);

	void CreateDir(std::string dir);
	bool FileExists(std::string file);
	bool FileExists(const char* file);
	void WriteFile(std::string file, std::string data);
	std::string ReadFile(std::string file);

	bool MemIsSet(void* mem, char chr, size_t length);

	void byte4_pack_rgba(const float* from, char* to);
	void byte4_unpack_rgba(const char* from, float* to);

	bool get_html(const std::string& url, std::wstring& header, std::wstring& hmtl);

	template <typename T> bool CompareNumber(T num1, T num2)
	{
		return std::abs(num1 - num2) < std::numeric_limits<T>::epsilon();
	}

	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) : InfoString() { this->Parse(buffer); };
		InfoString(const InfoString &obj) : KeyValuePairs(obj.KeyValuePairs) {};

		void Set(std::string key, std::string value);
		std::string Get(std::string key);

		std::string Build();

		void Dump();

	private:
		std::map<std::string, std::string> KeyValuePairs;
		void Parse(std::string buffer);
	};

	template <typename T> void Merge(std::vector<T>* target, T* source, size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T> void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto &entry : source)
		{
			target->push_back(entry);
		}
	}

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
