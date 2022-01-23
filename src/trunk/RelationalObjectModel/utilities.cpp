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
#include "stdafx.h"
#include "utilities.h"
#include <sstream>
#include <string.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#ifdef USE_LIBXML
#include <libxml/tree.h>
#endif
#ifdef WIN32
#include <comdef.h>
#endif

using namespace std;

string ROMUTIL::FindAndReplace (const string& source, const string target, const string replacement)
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

bool ROMUTIL::StringContains(const string& source, const string& target)
{
	bool retval = false;

	if (source.find(target, 0) != string::npos)
		retval = true;

	return retval;
}

vector<string> ROMUTIL::Split(const string& text, const string& separators)
{
	vector<string> retval;
	size_t n = text.length();
	size_t start, stop;
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

bool ROMUTIL::StringIsNumeric(const string& s)
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

string ROMUTIL::MakeGUID()
{
	//guid for each ObjectNode
	string guid;
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	std::stringstream ss;
	ss << u;
	guid = ss.str();
	return guid;
}

std::string ROMUTIL::encodeForXml(const string& sSrc)
{
    stringstream sRet;
    for( string::const_iterator iter = sSrc.begin(); iter!=sSrc.end(); iter++ )
    {
         char c = (char)*iter;
         switch( c )
         {
             case '&': sRet << "&amp;"; break;
             case '<': sRet << "&lt;"; break;
             case '>': sRet << "&gt;"; break;
             case '"': sRet << "&quot;"; break;
             case '\'': sRet << "&apos;"; break;
             default:
				sRet << c;
         }
    }
    return sRet.str();
}

#ifdef WIN32
string ROMUTIL::Narrow(const wchar_t *s)
{
	size_t len = wcslen(s);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, s, len, nullptr, 0, nullptr, nullptr);
	string ret(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, s, len, &ret[0], size_needed, nullptr, nullptr);
	return ret;
}

wstring ROMUTIL::Widen(const char *s)
{
	size_t len = strlen(s);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
	wstring ret(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, s, len, &ret[0], size_needed);
	return ret;
}

string ROMUTIL::Narrow(const wstring &s)
{
	return Narrow(s.c_str());
}

wstring ROMUTIL::Widen(const string &s)
{
	return Widen(s.c_str());
}
#endif

#if USE_LIBXML
string ROMUTIL::XMLStrToStr(xmlChar* mbStr)
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
