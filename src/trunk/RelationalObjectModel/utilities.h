/*
This file is part of the Relational Object Model Library.
Copyright (C) 2009-2015 Eric D. Schmidt, DigiRule Solutions LLC

    Relational Object Model is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Relational Object Model is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Relational Object Model.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#ifndef UTILITIES_ROM
#define UTILITIES_ROM

#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifndef _MSC_VER
#include <time.h>
#endif
#include <EDSEngine/XMLWrapper.h>

using namespace std;
namespace ROMUTIL
{
	string FindAndReplace(const string& source, const string target, const string replacement);
	bool StringContains(const string& source, const string& target);
	vector<string> Split(const string& text, const string& separators);
	bool StringIsNumeric(const string& s);
#ifdef WIN32
	string Narrow(const wchar_t *s);
	wstring Widen(const char *s);
	string Narrow(const wstring &s);
	wstring Widen(const string &s);
#endif
	string MakeGUID();
	string encodeForXml(const string& sSrc);
#if USE_LIBXML
	string XMLStrToStr(xmlChar* mbStr);
#endif

}
#endif
