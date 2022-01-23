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
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#if defined(WIN32)
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#endif

#ifndef ROMISDEF
#define ROMISDEF 1
#endif

// TODO: reference additional headers your program requires here
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;
/*
#ifdef _MSC_VER
	#if (_MSC_VER <= 1600)
		#include <hash_map>
		typedef stdext::hash_map<size_t, wstring> MAPUINTWSTR;
		typedef stdext::hash_map<wstring, size_t> MAPWSTRUINT;
		typedef stdext::hash_map<string, size_t> MAPSTRUINT;
		typedef stdext::hash_map<wstring, wstring> MAPWSTRS;
		typedef stdext::hash_map<size_t, stdext::hash_map<wstring, wstring> > MAPUINTMAP;
		typedef stdext::hash_map<wstring, stdext::hash_map<wstring, wstring> > MAPWSTRMAP;
	#else
		#include <unordered_map>
		typedef std::unordered_map<size_t, wstring> MAPUINTWSTR;
		typedef std::unordered_map<wstring, size_t> MAPWSTRUINT;
		typedef std::unordered_map<string, size_t> MAPSTRUINT;
		typedef std::unordered_map<wstring, wstring> MAPWSTRS;
		typedef std::unordered_map<size_t, std::unordered_map<wstring, wstring> > MAPUINTMAP;
		typedef std::unordered_map<wstring, std::unordered_map<wstring, wstring> > MAPWSTRMAP;
	#endif
#else
	#include <unordered_map>
	typedef std::unordered_map<size_t, wstring> MAPUINTWSTR;
	typedef std::unordered_map<wstring, size_t> MAPWSTRUINT;
	typedef std::unordered_map<string, size_t> MAPSTRUINT;
	typedef std::unordered_map<wstring, wstring> MAPWSTRS;
	typedef std::unordered_map<size_t, std::unordered_map<wstring, wstring> > MAPUINTMAP;
	typedef std::unordered_map<wstring, std::unordered_map<wstring, wstring> > MAPWSTRMAP;
#endif
	*/
#define ATTRIBUTE_NODE "Attribute"
#define OBJECT_NODE "Object"
#define XSLT_TOP "<?xml version=\"1.0\"?><xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\"><xsl:output method=\"text\" omit-xml-declaration=\"yes\" encoding=\"UTF-8\"/>"
#define XSLT_BOTTOM "\"/></xsl:for-each></xsl:template></xsl:stylesheet>"
inline void ReportROMError(std::string errorMsg)
{
	std::cerr << errorMsg << std::endl;
}
