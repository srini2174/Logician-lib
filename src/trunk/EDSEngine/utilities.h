/*
This file is part of the EDSEngine Library.
Copyright (C) 2009-2015 Eric D. Schmidt, DigiRule Solutions LLC

    EDSEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    EDSEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EDSEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#ifndef UTILITIES_EDS
#define UTILITIES_EDS

#include <string>
#include <vector>
#include "XMLWrapper.h"

using namespace std;

namespace EDSUTIL
{
	string FindAndReplace (const string& source, const string& target, const string& replacement);
	bool StringContains(const string& source, const string& target);
	vector<string> Split(const string& text, const string& separators);
	bool StringIsNumeric(const string& s);
	string TrimString(string s);
#ifdef WIN32
	string Narrow(const wchar_t *s);
	wstring Widen(const char *s);
	string Narrow(const wstring &s);
	wstring Widen(const string &s);
#endif
#ifdef USE_LIBXML
	string XMLStrToStr(xmlChar* mbStr);
#endif
}
#endif
