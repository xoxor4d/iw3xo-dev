#include "RegExp.h"

bool ParseRegExp(const std::wstring &regExp, bool caseSensitive, int groupCount, const std::wstring &source, std::vector<std::wstring> &result, bool allowDuplicate)
{
	result.clear();
	if (regExp.size() <= 0)
	{
		return false;
	}
	if (groupCount <= 0)
	{
		return false;
	}
	if (source.size() <= 0)
	{
		return false;
	}
	CAtlRegExp<> re;
	REParseError error = re.Parse(regExp.c_str(), caseSensitive);
	if (error != REPARSE_ERROR_OK)
	{
		return false;
	}
	wchar_t *pSource = new wchar_t[source.size() + 1];
	wchar_t *pSourceEnd = pSource + source.size();
	if (pSource == NULL)
	{
		return false;
	}
	wcscpy_s(pSource, source.size() + 1, source.c_str());
	BOOL bSucceed = TRUE;
	CAtlREMatchContext<> mc;
	const wchar_t *pFrom = pSource;
	const wchar_t *pTo = NULL;
	while (bSucceed)
	{
		bSucceed = re.Match(pFrom, &mc, &pTo);
		if (bSucceed)
		{
			const wchar_t *pStart = NULL;
			const wchar_t *pEnd = NULL;
			std::vector<std::wstring> tempMatch;
			for (int i = 0; i < groupCount; i++)
			{
				mc.GetMatch(i, &pStart, &pEnd);
				if (pStart != NULL && pEnd != NULL)
				{
					std::wstring match(pStart, pEnd - pStart);
					tempMatch.push_back(match);
				}
				else
				{
					break;
				}
			}
			bool bAdd = true;
			if (!allowDuplicate)
			{
				// Check whether this match already exists in the vector.
				for (std::vector<std::wstring>::iterator it = result.begin(); it != result.end();)
				{
					bool bEqual = true;
					for (std::vector<std::wstring>::iterator tempMatchIt = tempMatch.begin(); tempMatchIt != tempMatch.end(); tempMatchIt++, it++)
					{
						bool bGroupEqual = true;
						if (caseSensitive)
						{
							bGroupEqual = (wcscmp(it->c_str(), tempMatchIt->c_str()) == 0);
						}
						else
						{
							bGroupEqual = (_wcsicmp(it->c_str(), tempMatchIt->c_str()) == 0);
						}
						if (!bGroupEqual)
						{
							bEqual = false;
						}
					}
					if (bEqual)
					{
						bAdd = false;
						break;
					}
				}
			}
			if (bAdd)
			{
				for (std::vector<std::wstring>::iterator tempMatchIt = tempMatch.begin(); tempMatchIt != tempMatch.end(); tempMatchIt++)
				{
					result.push_back(*tempMatchIt);
				}
			}
			if (pTo < pSourceEnd)
			{
				pFrom = pTo;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	delete[] pSource;

	return true;
}