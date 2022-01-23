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
#include "stdafx.h"
#include "utilities.h"
#include <sstream>
#include <cstdlib>

using namespace std;

string EDSUTIL::FindAndReplace (const string& source, const string& target, const string& replacement)
{
	string str = source;
	string::size_type pos = 0,   // where we are now
					found;     // where the found data is

	if (target.size () > 0)   // searching for nothing will cause a loop
	{
		while ((found = str.find (target, pos)) != string::npos)
		{
		  str.replace (found, target.size (), replacement);
		  pos = found + replacement.size ();
		}
	}
	return str;
}

bool EDSUTIL::StringContains(const string& source, const string& target)
{
	bool retval = false;

	if (source.find(target, 0) != string::npos)
		retval = true;

	return retval;
}

vector<string> EDSUTIL::Split(const string& text, const string& separators)
{
	vector<string> retval;
	int n = text.length();
	int start, stop;
	start = text.find_first_not_of(separators);
	while ((start >= 0) && (start < n))
	{
		stop = text.find_first_of(separators, start);
		if ((stop < 0) || (stop > n)) stop = n;
		retval.push_back(text.substr(start, stop - start));
		start = text.find_first_not_of(separators, stop+1);
	}

	if (retval.size() == 0)
		retval.push_back(text);

	return retval;
}

bool EDSUTIL::StringIsNumeric(const string& s)
{
	bool retval = false;
	std::istringstream inpStream(s);
	double inpValue = 0.0;
	if (inpStream >> inpValue || s.length() == 0) //let null string go to a 0
	{
		retval = true;
	}

	return retval;
}

string EDSUTIL::TrimString(string s)
{
  string::size_type pos = s.find_last_not_of(' ');
  if(pos != string::npos) {
    s.erase(pos + 1);
    pos = s.find_first_not_of(' ');
    if(pos != string::npos) s.erase(0, pos);
  }
  else s.erase(s.begin(), s.end());

  return s;
}

#ifdef WIN32
string EDSUTIL::Narrow(const wchar_t *s)
{
	size_t len = wcslen(s);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, s, len, nullptr, 0, nullptr, nullptr);
	string ret(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0,s, len, &ret[0], size_needed, nullptr, nullptr);
	return ret;
}

wstring EDSUTIL::Widen(const char *s)
{
	size_t len = strlen(s);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
	wstring ret(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, s, len, &ret[0], size_needed);
	return ret;
}

string EDSUTIL::Narrow(const wstring &s)
{
	return Narrow(s.c_str());
}

wstring EDSUTIL::Widen(const string &s)
{
	return Widen(s.c_str());
}
#endif

#ifdef USE_LIBXML
string EDSUTIL::XMLStrToStr(xmlChar* mbStr)
{
	if (mbStr != nullptr)
	{
		string retval = (char*)mbStr;
		xmlFree(mbStr);
		return retval;
	}
	return "";
}
#endif
