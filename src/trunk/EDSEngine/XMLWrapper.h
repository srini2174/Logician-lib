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

#ifdef USE_LIBXML
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <assert.h>

	#include <libxml/tree.h>
	#include <libxml/parser.h>
	#include <libxml/xpath.h>
	#include <libxml/xpathInternals.h>

    typedef xmlAttrPtr Attribute;
	typedef xmlNodePtr Node;
	typedef xmlDocPtr Document;
	typedef xmlNodeSetPtr NodeList;
#endif

#ifdef USE_MSXML
#ifndef _WINRT_DLL
	#import "msxml6.dll"

	#ifdef USEATL
		#include <atlbase.h>
		typedef CComPtr<MSXML2::IXMLDOMAttribute> Attribute;
		typedef CComPtr<MSXML2::IXMLDOMNode> Node;
		typedef CComPtr<MSXML2::IXMLDOMNodeList> NodeList;
		typedef CComPtr<MSXML2::IXMLDOMDocument2> Document;
		typedef CComPtr<MSXML2::IXMLDOMElement> Element;
		typedef CComPtr<MSXML2::IXMLDOMNamedNodeMap> NamedNodeMap;
		typedef CComPtr<MSXML2::IXMLDOMProcessingInstruction> ProcessingInstruction;
	#else
		typedef MSXML2::IXMLDOMAttributePtr Attribute;
		typedef MSXML2::IXMLDOMNodePtr Node;
		typedef MSXML2::IXMLDOMNodeListPtr NodeList;
		typedef MSXML2::IXMLDOMDocument2Ptr Document;
		typedef MSXML2::IXMLDOMElementPtr Element;
		typedef MSXML2::IXMLDOMNamedNodeMapPtr NamedNodeMap;
		typedef MSXML2::IXMLDOMProcessingInstructionPtr ProcessingInstruction;
	#endif
#else
	typedef void* Attribute;
	typedef void* Node;
	typedef void* NodeList;
	typedef void* Document;
	typedef void* Element;
	typedef void* NamedNodeMap;
	typedef void* ProcessingInstruction;
#endif

	#define X(str) _bstr_t(str)
#endif
