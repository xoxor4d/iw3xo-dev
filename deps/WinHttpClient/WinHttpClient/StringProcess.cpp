#include "StringProcess.h"

std::wstring Trim(const std::wstring &source, const std::wstring &targets)
{
	std::wstring::size_type start = 0;
	std::wstring::size_type end = 0;
	for (start = 0; start < source.size(); start++)
	{
		bool bIsTarget = false;
		for (std::wstring::size_type i = 0; i < targets.size(); i++)
		{
			if (source[start] == targets[i])
			{
				bIsTarget = true;
				break;
			}
		}
		if (!bIsTarget)
		{
			break;
		}
	}
	for (end = source.size() - 1; (int)end >= 0; end--)
	{
		bool bIsTarget = false;
		for (std::wstring::size_type i = 0; i < targets.size(); i++)
		{
			if (source[end] == targets[i])
			{
				bIsTarget = true;
				break;
			}
		}
		if (!bIsTarget)
		{
			break;
		}
	}
	std::wstring result = L"";
	if (end >= start && start < source.size() && end >= 0)
	{
		result = source.substr(start, end - start + 1);
	}

	return result;
}

bool PrepareString(wchar_t *dest, size_t *size, const std::wstring &src)
{
	if (dest == NULL)
	{
		if (size != NULL)
		{
			*size = src.size();
		}
		return false;
	}
	else
	{
		if (size != NULL)
		{
			wcsncpy_s(dest, *size, src.c_str(), _TRUNCATE);
			if (*size <= src.size())
			{
				::SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}
		}
	}
	return true;
}

std::wstring ReplaceString(const std::wstring &srcStr, const std::wstring &oldStr, const std::wstring &newStr)
{
	if (srcStr.size() <= 0 || oldStr.size() <= 0)
	{
		return srcStr;
	}
	std::wstring strReturn = srcStr;
	std::wstring::size_type offset = 0;
	std::wstring::size_type start = strReturn.find(oldStr);
	while (start != std::wstring::npos)
	{
		offset = start + newStr.size();
		strReturn.replace(start, oldStr.size(), newStr);
		start = strReturn.find(oldStr, offset);
	}

	return strReturn;
}

int StringToInteger(const std::wstring &number)
{
	if (number.size() <= 0)
	{
		return 0;
	}
	std::wstring num = ReplaceString(number, L",", L"");
	num = ReplaceString(num, L" ", L"");

	return _wtoi(num.c_str());
}

std::wstring LowerString(const std::wstring &text)
{
	if (text.size() <= 0)
	{
		return L"";
	}
	unsigned int iLength = text.size() + 1;
	wchar_t *pTemp = new wchar_t[iLength];
	if (pTemp == NULL)
	{
		return L"";
	}
	wcscpy_s(pTemp, iLength, text.c_str());
	_wcslwr_s(pTemp, iLength);
	std::wstring retStr = pTemp;
	delete[] pTemp;

	return retStr;
}

std::wstring UpperString(const std::wstring &text)
{
	if (text.size() <= 0)
	{
		return L"";
	}
	unsigned int iLength = text.size() + 1;
	wchar_t *pTemp = new wchar_t[iLength];
	if (pTemp == NULL)
	{
		return L"";
	}
	wcscpy_s(pTemp, iLength, text.c_str());
	_wcsupr_s(pTemp, iLength);
	std::wstring retStr = pTemp;
	delete[] pTemp;

	return retStr;
}

std::wstring GetAnchorText(const std::wstring &anchor)
{
	std::wstring regExp = L"<a.*?>[ \t\r\n]*{.*?}[ \t\r\n]*</a>";
	std::vector<std::wstring> result;
	if (ParseRegExp(regExp, false, 1, anchor, result) && result.size() == 1)
	{
		std::wstring text = result[0];
		return text;
	}

	return L"";
}

std::wstring GetAnchorLink(const std::wstring &anchor)
{
	std::wstring regExp = L"<a.*?href=\"|\'{.*?}\"|\'.*?>.*?</a>";
	std::vector<std::wstring> result;
	if (ParseRegExp(regExp, false, 1, anchor, result) && result.size() == 1)
	{
		std::wstring link = result[0];
		return link;
	}

	return L"";
}

bool SeparateString(const std::wstring &content, const std::wstring &delimiter, std::vector<std::wstring> &result)
{
	if (content.size() <= 0 || delimiter.size() <= 0)
	{
		return false;
	}

	result.clear();
	std::wstring::size_type start = 0;
	std::wstring::size_type index = 0;
	index = content.find(delimiter, start);
	while (index != std::wstring::npos)
	{
		std::wstring::size_type size = index - start;
		if (size > 0)
		{
			std::wstring temp = content.substr(start, size);
			if (temp.size() > 0)
			{
				result.push_back(temp);
			}
		}
		start += size + delimiter.size();
		index = content.find(delimiter, start);
	}
	if (content.find(delimiter) != std::wstring::npos)
	{
		std::wstring last = content.substr(start);
		if (last.size() > 0)
		{
			result.push_back(last);
		}
	}
	else
	{
		false;
	}

	return true;
}

std::wstring URLEncoding(const std::wstring &keyword, bool convertToUTF8)
{
	int iLength = 0;
	char *szKeyword = NULL;

	if (convertToUTF8)
	{
		iLength = ::WideCharToMultiByte(CP_UTF8,
			0,
			keyword.c_str(),
			keyword.length(),
			NULL,
			0,
			NULL,
			NULL);
		if (iLength <= 0)
		{
			return L"";
		}

		szKeyword = new char[iLength];
		if (szKeyword == NULL)
		{
			return L"";
		}
		iLength = ::WideCharToMultiByte(CP_UTF8,
			0,
			keyword.c_str(),
			keyword.length(),
			szKeyword,
			iLength,
			NULL,
			NULL);
	}
	else
	{
		std::string strKeyword = (char *)(_bstr_t)keyword.c_str();
		iLength = (int)strKeyword.length();
		szKeyword = new char[strKeyword.length() + 1];
		strcpy_s(szKeyword, strKeyword.length() + 1, strKeyword.c_str());
	}

	std::wstring encodedKeyword = L"";
	std::string strEncodedKeyword = "";
	for (int i = 0; i < iLength; i++)
	{
		unsigned char c = (unsigned char)szKeyword[i];
		char temp[MAX_PATH] = "";
		sprintf_s(temp, MAX_PATH, "%%%2X", c);
		if (temp[1] == ' ')
		{
			temp[1] = '0';
		}
		strEncodedKeyword += temp;
	}
	if (szKeyword != NULL)
	{
		delete[] szKeyword;
	}
	encodedKeyword = (wchar_t *)(_bstr_t)strEncodedKeyword.c_str();
	encodedKeyword = ReplaceString(encodedKeyword, L" ", L"+");

	return encodedKeyword;
}

unsigned int GetSeparateKeywordMatchGrade(const std::wstring &source, const std::wstring &keyword)
{
	if (source.length() <= 0 || keyword.length() <= 0)
	{
		return 0;
	}

	std::wstring lowerSource = LowerString(source);
	std::wstring lowerKeyword = LowerString(keyword);

	unsigned int grade = 0;
	if (lowerKeyword.length() <= 3)
	{
		if (lowerSource.find(lowerKeyword) != std::wstring::npos)
		{
			grade = 100;
		}
		else
		{
			grade = 0;
		}
	}
	else
	{
		unsigned int matchLength = 0;
		unsigned int index = 0;
		while (index < lowerKeyword.length())
		{
			unsigned int compareLength = lowerKeyword.length() - index;
			while (compareLength > 0 && index < lowerKeyword.length())
			{
				std::wstring subKeyword = lowerKeyword.substr(index, compareLength);
				if (lowerSource.find(subKeyword) != std::wstring::npos)
				{
					matchLength += compareLength;
					index += compareLength;
				}
				else
				{
					compareLength--;
				}
			}
			index++;
		}
		grade = matchLength * 100 / lowerKeyword.length();
	}

	return grade;
}

unsigned int GetKeywordMatchGrade(const std::wstring &source, const std::wstring & keyword)
{
	if (source.length() <= 0 || keyword.length() <= 0)
	{
		return 0;
	}

	unsigned int grade = 0;
	std::wstring src = source;
	while (src.find(L"\t") != std::wstring::npos)
	{
		src = ReplaceString(src, L"\t", L" ");
	}
	while (src.find(L"  ") != std::wstring::npos)
	{
		src = ReplaceString(src, L"  ", L" ");
	}
	std::vector<std::wstring> results;
	if (SeparateString(keyword, L" ", results) && results.size() > 0)
	{
		unsigned int keywordTotalLength = 0;
		for (std::vector<std::wstring>::size_type index = 0; index < results.size(); index++)
		{
			keywordTotalLength += results[index].length();
		}
		for (std::vector<std::wstring>::size_type index = 0; index < results.size(); index++)
		{
			grade += GetSeparateKeywordMatchGrade(src, results[index]) * results[index].length() / keywordTotalLength;
		}
	}
	else
	{
		grade = GetSeparateKeywordMatchGrade(src, keyword);
	}

	return grade;
}

std::wstring GetDateString(const COleDateTime &time, const std::wstring &separator, bool align)
{
	std::wstring date = L"";
	wchar_t szTemp[MAX_PATH] = L"";

	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetYear());
	date += szTemp;
	date += separator;

	memset(szTemp, 0, sizeof(wchar_t) * MAX_PATH);
	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetMonth());
	if (time.GetMonth() < 10 && align)
	{
		date += L"0";
	}
	date += szTemp;
	date += separator;

	memset(szTemp, 0, sizeof(wchar_t) * MAX_PATH);
	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetDay());
	if (time.GetDay() < 10 && align)
	{
		date += L"0";
	}
	date += szTemp;

	return date;
}

std::wstring GetDateString(int dayOffset, const std::wstring &separator, bool align)
{
	COleDateTime time = COleDateTime::GetCurrentTime();
	time += COleDateTimeSpan(dayOffset, 0, 0, 0);

	return GetDateString(time, separator, align);
}

std::wstring GetTimeString(const COleDateTime &time, const std::wstring &separator, bool align)
{
	std::wstring date = L"";
	wchar_t szTemp[MAX_PATH] = L"";

	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetHour());
	date += szTemp;
	date += separator;

	memset(szTemp, 0, sizeof(wchar_t) * MAX_PATH);
	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetMinute());
	if (time.GetMinute() < 10 && align)
	{
		date += L"0";
	}
	date += szTemp;
	date += separator;

	memset(szTemp, 0, sizeof(wchar_t) * MAX_PATH);
	swprintf_s(szTemp, MAX_PATH, L"%d", time.GetSecond());
	if (time.GetSecond() < 10 && align)
	{
		date += L"0";
	}
	date += szTemp;

	return date;
}

std::wstring MD5(const std::wstring &text)
{
	if (text.size() <= 0)
	{
		return L"";
	}
	std::string asciiText = (char *)(_bstr_t)text.c_str();
	std::wstring encrypted = L"";
	HCRYPTPROV hCryptProv = NULL;
	if (::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))
	{
		HCRYPTHASH hHash = NULL;
		if (::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash))
		{
			if (::CryptHashData(hHash, (BYTE *)asciiText.c_str(), asciiText.size(), 0))
			{
				BYTE result[16];
				DWORD dwSize = 16;
				wchar_t temp[3] = L"";
				if (::CryptGetHashParam(hHash, HP_HASHVAL, result, &dwSize, 0))
				{
					for (unsigned int i = 0; i < 16; i++)
					{
						memset(temp, 0, 6);
						swprintf(temp, 3, L"%02x", result[i]);
						encrypted += temp;
					}
				}
			}
			::CryptDestroyHash(hHash);
			::CryptReleaseContext(hCryptProv, 0);
		}
	}

	return encrypted;
}

std::wstring FilterFileName(const std::wstring &name)
{
	if (name.size() <= 0)
	{
		return L"";
	}

	std::wstring filteredName = name;
	filteredName = ReplaceString(filteredName, L"/", L"_");
	filteredName = ReplaceString(filteredName, L"\\", L"_");
	filteredName = ReplaceString(filteredName, L":", L"_");
	filteredName = ReplaceString(filteredName, L"*", L"_");
	filteredName = ReplaceString(filteredName, L"?", L"_");
	filteredName = ReplaceString(filteredName, L"\"", L"_");
	filteredName = ReplaceString(filteredName, L"<", L"_");
	filteredName = ReplaceString(filteredName, L">", L"_");
	filteredName = ReplaceString(filteredName, L"|", L"_");

	return filteredName;
}

std::wstring GetMagic(unsigned int length)
{
	srand(::GetTickCount());
	if (length <= 0)
	{
		return L"";
	}

	std::wstring margic = L"";
	for (unsigned int i = 0; i < length; i++)
	{
		wchar_t szMargic[50] = L"";
		swprintf_s(szMargic, 50, L"%c", rand() % 26 + L'a');
		margic += szMargic;
	}

	return margic;
}

std::wstring GetHost(const std::wstring &url)
{
	if (url.size() <= 0)
	{
		return L"";
	}

	std::wstring urlWidthoutHttp = ReplaceString(LowerString(url), L"http://", L"");

	unsigned int index = urlWidthoutHttp.find(L"/");
	if (index == std::wstring::npos)
	{
		index = urlWidthoutHttp.find(L"\\");
	}
	if (index == std::wstring::npos)
	{
		return urlWidthoutHttp;
	}

	return urlWidthoutHttp.substr(0, index);
}

std::wstring GetValidFileName(const std::wstring &fileName)
{
	if (fileName.size() == 0)
	{
		return L"";
	}
	std::wstring tempFileName = fileName;
	tempFileName = ReplaceString(tempFileName, L"\\", L"_");
	tempFileName = ReplaceString(tempFileName, L"/", L"_");
	tempFileName = ReplaceString(tempFileName, L":", L"_");
	tempFileName = ReplaceString(tempFileName, L"*", L"_");
	tempFileName = ReplaceString(tempFileName, L"?", L"_");
	tempFileName = ReplaceString(tempFileName, L"\"", L"_");
	tempFileName = ReplaceString(tempFileName, L"<", L"_");
	tempFileName = ReplaceString(tempFileName, L">", L"_");
	tempFileName = ReplaceString(tempFileName, L"|", L"_");
	tempFileName = ReplaceString(tempFileName, L"\r", L"_");
	tempFileName = ReplaceString(tempFileName, L"\n", L"_");
	tempFileName = ReplaceString(tempFileName, L"%", L"_");

	return tempFileName;
}