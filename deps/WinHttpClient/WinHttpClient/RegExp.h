/**
 *  Copyright 2008-2009 Cheng Shi.  All rights reserved.
 *  Email: shicheng107@hotmail.com
 */

#ifndef REGEXP_H
#define REGEXP_H

#include <iostream>
#include <string>
#include <vector>
//using namespace std;

#pragma warning(push)
#pragma warning(disable: 6385 6011 4127)
#include "atlrx.h"
#pragma warning(pop)

/*
 * Parameters
 *  [in] regExp: Value of type string which is the input regular expression.
 *  [in] caseSensitive: Value of type bool which indicate whether the parse is case sensitive.
 *  [in] groupCount: Value of type int which is the group count of the regular expression.
 *  [in] source: Value of type string reference which is the source to parse.
 *  [out] result: Value of type vecotr of strings which is the output of the parse.
 *  [in] allowDuplicate: Value of type bool which indicates whether duplicate items are added to the output result.
 *
 * Return Value
 *  Returns true if the function succeeds, or false otherwise.
 *
 * Remarks
 *  The output result is devided into groups.  User should get the groups according to the group count.  For example:
 *  1. RegExp = L"{ab}", source = L"abcabe", then result = L"ab", L"ab".
 *  2. RegExp = L"{ab}{cd}", source = L"abcdeabecd", then result = L"ab", L"cd", L"ab", L"cd".
*/
bool ParseRegExp(const std::wstring &regExp, bool caseSensitive, int groupCount, const std::wstring &source, std::vector<std::wstring> &result, bool allowDuplicate = false);

#endif // REGEXP_H
