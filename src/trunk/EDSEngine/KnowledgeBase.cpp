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
#include "KnowledgeBase.h"
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <string>
#ifndef DISABLE_DECISIONLOGIC_INTEGRATION
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
#endif
#ifdef USE_PYTHON
#include <boost/python.hpp>
#endif

#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#ifdef WIN32
#include <direct.h>
#include <comdef.h>
#endif

#ifdef USE_JAVASCRIPT
	#ifdef _MSC_VER
		#ifdef USE_WINDOWS_SCRIPTING
			#import "msscript.ocx"	//32bit apps only
			using namespace MSScriptControl;
		#else	//use Active Script Interfaces
			#include <activscp.h>
			#pragma warning(disable:4100)
			#include "ashost.h"
		#endif
	#else
		#define USE_SPIDERMONKEY
		#include <js/jsapi.h> //Mozilla SpiderMonkey
		static JSClass global_class = {
			"global", JSCLASS_GLOBAL_FLAGS,
			JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
			JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
			JSCLASS_NO_OPTIONAL_MEMBERS
		};
    #endif
#endif

#if defined(WIN32) && defined(_MSC_VER)
// ascii to unsigned long long
unsigned long long atoull(const char *str){
        return _strtoui64(str, nullptr, 10);
}
#else
#define atoull(x) strtoull(x, 0, 10);
#endif

using namespace std;
using namespace EDS;

CKnowledgeBase::~CKnowledgeBase(void)
{
#ifdef WIN32
	CoUninitialize();
#endif
}

CKnowledgeBase::CKnowledgeBase()
{
	m_DEBUGGING_MSGS = false;
	m_remoteDebugging = false;
	m_DEBUGGING_CON = "localhost:11000";
	m_IsOpen = false;
	mapBaseIDtoTranslations.clear();
	iRecursingDepth = 0;
	m_DebugHandlerPtr = nullptr;
	m_InputValueGetterPtr = nullptr;
#ifdef WIN32
	HRESULT hr = CoInitialize(nullptr);
#endif
}

string CKnowledgeBase::DeLocalize(const string& localeValue)
{
	string retval = localeValue;
	for (unordered_map<size_t, std::unordered_map<string, string> >::iterator itAllIndexes = mapBaseIDtoTranslations.begin(); itAllIndexes != mapBaseIDtoTranslations.end(); itAllIndexes++)
	{
		for (unordered_map<string, string>::iterator itTranslations = itAllIndexes->second.begin(); itTranslations != itAllIndexes->second.end(); itTranslations++)
		{
			if (itTranslations->second == localeValue)
			{
				retval = m_stringsMap.GetStringByID(itAllIndexes->first);
				return retval;
			}
		}
	}
	return retval;
}

string CKnowledgeBase::Translate(const string& source, const string& sourceLocale, const string& destLocale)
{
	string retval = source;
	size_t id = INVALID_STRING;
	if (sourceLocale.length() == 0)
		id = m_stringsMap.GetIDByString(source);
	else
	{
		for (unordered_map<size_t, std::unordered_map<string, string> >::iterator itAllIndexes = mapBaseIDtoTranslations.begin(); itAllIndexes!= mapBaseIDtoTranslations.end(); itAllIndexes++)
		{
			for (unordered_map<string, string>::iterator itTranslations = itAllIndexes->second.begin(); itTranslations != itAllIndexes->second.end(); itTranslations++)
			{
				if (itTranslations->first == sourceLocale && itTranslations->second == source)
				{
					id = itAllIndexes->first;
					break;
				}
			}
			if (id != INVALID_STRING)
				break;
		}
	}

	if (id != INVALID_STRING)
	{
		unordered_map<size_t, std::unordered_map<string, string> >::iterator itAllIndexes = mapBaseIDtoTranslations.find(id);
		if (itAllIndexes != mapBaseIDtoTranslations.end())
		{
			for (unordered_map<string, string>::iterator itTranslations = itAllIndexes->second.begin(); itTranslations != itAllIndexes->second.end(); itTranslations++)
			{
				if (itTranslations->first == destLocale)
				{
					retval = itAllIndexes->second[destLocale];
					return retval;
				}
			}
		}
	}
	return retval;
}

vector<string> CKnowledgeBase::GetAllPossibleOutputs(const string& tableName, const string& outputName)
{
	CRuleTable *table = m_TableSet.GetTable(tableName);
	if (table != nullptr)
		return m_TableSet.GetTable(tableName)->GetAllPossibleOutputs(outputName);
	else
		return vector<string>();
}

bool CKnowledgeBase::TableHasScript(const string& tableName)
{
	CRuleTable *table = m_TableSet.GetTable(tableName);
	if (table != nullptr)
		return (table->HasJS() || table->HasPython());
	else
		return false;
}

bool CKnowledgeBase::TableIsGetAll(const string& tableName)
{
	CRuleTable *table = m_TableSet.GetTable(tableName);
	if (table != nullptr)
		return table->IsGetAll();
	else
		return false;
}

//public functions
vector<string> CKnowledgeBase::EvaluateTable(const string& tableName, const string& outputAttr, bool bGetAll, void* context)
{
	string param = "";
	return EvaluateTableWithParam(tableName, outputAttr, bGetAll, param, context);
}

map<string, vector<string> > CKnowledgeBase::EvaluateTable(const string& tableName, bool bGetAll, void* context)
{
	string param = "";
	return EvaluateTableWithParam(tableName, bGetAll, param, context);
}

vector<string> CKnowledgeBase::EvaluateTableWithParam(const string& tableName, const string& outputAttr, bool bGetAll, std::string& param, void* context)
{
	vector<string> retval;
	try
	{
		//isolate the evaluation memory space for thead safety
		bool tableExists = false;
		CRuleTable table = m_TableSet.GetTableCopy(tableName, &tableExists);

		if (!tableExists)
			return retval;

		iRecursingDepth++;

		table.EnbleDebugging(_debugThisTable(tableName));
		table.InputValueGetter = m_InputValueGetterPtr;

		vector<string> results = table.EvaluateTable(outputAttr, bGetAll, true, context);
		//check for existance of table chain
		if (table.HasChain() == true)
		{
			vector<string> eraseResults;
			vector<string> newResults;
			for (vector<string>::iterator it = results.begin(); it != results.end(); it++)
			{
				if (EDSUTIL::StringContains(*it, "eval("))
				{
					string cmd((*it).begin() + 5, (*it).end() - 1);
					vector<string> args = EDSUTIL::Split(cmd, ",");
					vector<string> chainedResults;
					string debugVals;
					if (args.size() == 2)
					{
						string chainTableName = EDSUTIL::TrimString(args[0]);
						string chainAttrName = EDSUTIL::TrimString(args[1]);

						chainedResults = EvaluateTableWithParam(chainTableName, chainAttrName, TableIsGetAll(chainTableName), param, context);
						for (vector<string>::iterator itRes = chainedResults.begin(); itRes != chainedResults.end(); itRes++)
						{
							newResults.push_back((*itRes));
							if (_debugThisTable(chainTableName))
							{
								if (debugVals.size() > 0)
									debugVals+="|";
								else
									debugVals = ":";

								debugVals+=_XMLSafe(*itRes);
							}
						}
					}
					if (_debugThisTable(tableName) && chainedResults.size() > 0)
					{ //replace the eval( string with the actual value
						table.DebugMessage = EDSUTIL::FindAndReplace(table.DebugMessage, *it, *it + debugVals);
					}
				}
				else
				{
					newResults.push_back(*it);
				}
			}

			results.clear();
			for (vector<string>::iterator it = newResults.begin(); it != newResults.end(); it++)
			{
				results.push_back((*it));
			}
		}

		//check for existance of runtime scripting, JavaScript or Python
	#ifdef USE_JAVASCRIPT
		if (table.HasJS() == true)
		{
			vector<string> eraseResults;
			vector<string> newResults;

			for (vector<string>::iterator it = results.begin(); it != results.end(); it++)
			{
				if (EDSUTIL::StringContains(*it, "js("))
				{
					string val = "ERROR";
					#ifdef USE_WINDOWS_SCRIPTING
						IScriptControlPtr pScriptControl(__uuidof(ScriptControl));
						LPSAFEARRAY psa;
						SAFEARRAYBOUND rgsabound[]  = { 1, 0 }; // 1 element, 0-based
						psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
					#endif
					try
					{
						//everything must return as a string
						string customCode((*it).begin() + 3, (*it).end() - 1);
						vector<string> lines = EDSUTIL::Split(customCode, "\n");
						if (lines.size() == 1) //do this for covienience and table brevity
						{
							if (!EDSUTIL::StringContains(lines[0], "return"))
							{
								lines[0] = "return (" + lines[0] + ").toString();";
							}
						}
						string codeBody;
						for (vector<string>::iterator it = lines.begin(); it != lines.end(); it++)
						{
							codeBody += *it;
							codeBody += "\n";
						}
						string JSCode = "function myfunc(){\n" + codeBody + "}\n";
						JSCode += "var param = \"" + param + "\";\n";
						JSCode += "function getparam(){return param;}\n";
						JSCode += m_jsCode;
						#ifdef _MSC_VER
							#ifdef USE_WINDOWS_SCRIPTING
								if (psa)
								{
									pScriptControl->Language = "JScript";
									pScriptControl->AddCode(JSCode.c_str());

									val = VariantToMBCStr(pScriptControl->Run("myfunc", &psa));
									param = VariantToMBCStr(pScriptControl->Run("getparam", &psa));

									SafeArrayDestroy(psa);
								}
							#else
								IActiveScriptHost *activeScriptHost = nullptr;
								HRESULT hr = ScriptHost::Create(&activeScriptHost);
								if(!FAILED(hr))
								{
									activeScriptHost->AddScript(EDSUTIL::Widen(JSCode).c_str());
									VARIANT vRes, vStateParam;
									DISPPARAMS args;
									args.cNamedArgs = 0;
									args.cArgs = 1;
									VARIANTARG *pVariant = new VARIANTARG;
									args.rgvarg = pVariant;
									args.rgvarg[0].vt = VT_BOOL;
									args.rgvarg[0].boolVal = true;
									activeScriptHost->Run(L"myfunc", &args, &vRes);
									activeScriptHost->Run(L"getparam", &args, &vStateParam);
									val = VariantToMBCStr(vRes);
									param = VariantToMBCStr(vStateParam);
									delete pVariant;
								}
								if(activeScriptHost)
									activeScriptHost->Release();
							#endif
						#endif

						#ifdef USE_SPIDERMONKEY
							/* JS variables. */
							JSRuntime *rt;
							JSContext *cx;
							JSObject  *global;

							/* Create a JS runtime. */
							rt = JS_NewRuntime(8L * 1024L * 1024L);
							if (rt == nullptr)
								throw;

							/* Create a context. */
							cx = JS_NewContext(rt, 8192);
							if (cx == nullptr)
								throw;
							JS_SetOptions(cx, JSOPTION_VAROBJFIX);
							JS_SetVersion(cx, JSVERSION_DEFAULT);
							//JS_SetErrorReporter(cx, reportError);

							JS_BeginRequest(cx);

							/* Create the global object. */
							global = JS_NewCompartmentAndGlobalObject(cx, &global_class, nullptr);
							if (global == nullptr)
								throw;

							/* Populate the global object with the standard globals,
							   like Object and Array. */
							if (!JS_InitStandardClasses(cx, global))
								throw;

							jsval rval, stateval, funval;
							JS_EvaluateScript(cx, global, JSCode.c_str(), JSCode.length(), "EDS_JScript", 1, &funval);

							jsval argv;
							JSBool ok = JS_CallFunctionName(cx, global, "myfunc", 1, &argv, &rval);
							JSBool ok2 = JS_CallFunctionName(cx, global, "getparam", 1, &argv, &stateval);

							if (rval != 0 && ok)
							{
								JSString* jstr = JS_ValueToString(cx, rval);
								const char* s = (const char*)JS_EncodeString(cx, jstr);
								val = s;
							}
							if (stateval != 0 && ok2)
							{
								JSString* jstr = JS_ValueToString(cx, stateval);
								const char* s = (const char*)JS_EncodeString(cx, jstr);
								param = s;
							}

							// Cleanup.
							JS_DestroyContext(cx);
							JS_DestroyRuntime(rt);
							JS_ShutDown();
						#endif
					}
					#ifdef _MSC_VER
					catch(_com_error e)
					{
						string message = "Failed to evaluate javascript\n";
						message += to_string(e.Error());
						message += ":";
						message += e.Source();
						message += "\n";
						message += e.Description();
						OutputDebugString(EDSUTIL::Widen(message).c_str());
						val = "ERROR";
					}
					#endif
					catch(...)
					{
						val = "ERROR";
					}

					newResults.push_back(val);

					if (_debugThisTable(tableName))
					{ //replace the js( string with the actual value
						table.DebugMessage = EDSUTIL::FindAndReplace(table.DebugMessage, *it, *it + ":" + _XMLSafe(val));
					}
				}
				else
				{
					newResults.push_back(*it);
				}
			}

			results.clear();
			for (vector<string>::iterator it = newResults.begin(); it != newResults.end(); it++)
			{
				results.push_back((*it));
			}
		}
	#endif

	#ifdef USE_PYTHON
		if (table.HasPython() == true)
		{
			vector<string> eraseResults;
			vector<string> newResults;
			for (vector<string>::iterator it = results.begin(); it != results.end(); it++)
			{
				if (StringContains(*it, "py("))
				{
					string val = "ERROR";
					try
					{
						string customCode((*it).begin() + 3, (*it).end() - 1), indentedCode;

						//everything must return as a string
						vector<string> lines = EDSUTIL::Split(customCode, "\n");
						if (lines.size() == 1) //do this for covienience and table brevity
						{
							if (!EDSUTIL::StringContains(lines[0], "return"))
							{
								lines[0] = "return str(" + lines[0] + ")";
							}
						}

						//function code must be indented
						for (vector<string>::iterator it = lines.begin(); it != lines.end(); it++)
						{
							indentedCode += "   ";
							indentedCode += *it;
							indentedCode += "\n";
						}
						string PythonCode = "def myfunc():\n" + strToMBCStr(indentedCode) + "\n";
						PythonCode += strToMBCStr(m_pyCode);
						Py_Initialize();
						PyRun_SimpleString(Narrow(m_pyCode).c_str());
						PyRun_SimpleString(PythonCode.c_str());

						boost::python::object module(boost::python::handle<>(boost::python::borrowed(PyImport_AddModule("__main__"))));
						boost::python::object function = module.attr("myfunc");
						boost::python::object dictionary = module.attr("__dict__");
						dictionary["param"] = strToMBCStr(param);

						boost::python::object result = function();
						val = boost::python::extract<string>(result);
						param = boost::python::extract<string>(dictionary["param"]);

						Py_Finalize();
					}
					catch (boost::python::error_already_set)
					{
						//PyErr_Print();
						val = "ERROR";
					}
					newResults.push_back(val);

					if (_debugThisTable(tableName))
					{	//replace the py( string with the actual value
						table.DebugMessage = EDSUTIL::FindAndReplace(table.DebugMessage, *it, *it + ":" + _XMLSafe(val));
					}
				}
				else
				{
					newResults.push_back(*it);
				}
			}

			results.clear();
			for (vector<string>::iterator it = newResults.begin(); it != newResults.end(); it++)
			{
				results.push_back((*it));
			}
		}
	#endif

		iRecursingDepth--;

		if (_debugThisTable(tableName) == true)
		{
			_sendToDebugServer(table.DebugMessage);
		}

		retval = results;
	}
	catch (exception& except)
	{
		ReportError(except.what());
	}
	catch (...)
	{
		ReportError("CKnowledgeBase::EvaluateTableWithParam\nCheck your table name.");
	}
	return retval;
}

string CKnowledgeBase::GetFirstTableResult(const string& tableName, const string& outputAttr, void* context)
{
	string retval = "";
	vector<string> retAll = EvaluateTable(tableName, outputAttr, context);
	if (retAll.size() > 0)
		retval = retAll[0];

	return retval;
}

vector<string> CKnowledgeBase::ReverseEvaluateTable(const string& tableName, const string& inputAttr, bool bGetAll, void* context)
{
	vector<string> retval;
	//no chaining or scripting in reverse
	try
	{
		bool exists = false;
		CRuleTable table = m_TableSet.GetTableCopy(tableName, &exists);
		if (exists)
		{
			table.EnbleDebugging(_debugThisTable(tableName));
			table.InputValueGetter = m_InputValueGetterPtr;
			retval = table.EvaluateTable(inputAttr, bGetAll, false, context);
		}
	}
	catch (...)
	{
		ReportError("CKnowledgeBase::ReverseEvaluateTable\nCheck your table name.");
	}
	return retval;
}

map<string, vector<string> > CKnowledgeBase::ReverseEvaluateTable(const string& tableName, bool bGetAll, void* context)
{
	map<string, vector<string> > retval;

	try
	{
		bool exists = false;
		CRuleTable table = m_TableSet.GetTableCopy(tableName, &exists);
		if (!exists)
			return retval;
		table.EnbleDebugging(_debugThisTable(tableName));
		table.InputValueGetter = m_InputValueGetterPtr;
		vector<pair<string, vector<CRuleCell> > > outputCollection = table.GetInputAttrsTests();
		//for all the outputs get the results
		for (vector<pair<string, vector<CRuleCell> > >::iterator itOut = outputCollection.begin(); itOut != outputCollection.end(); itOut++)
		{
			vector<string> result = table.EvaluateTable((*itOut).first, bGetAll, false, context);
			retval[(*itOut).first] = result;
		}
	}
	catch (...)
	{
		ReportError("CKnowledgeBase::ReverseEvaluateTable\nCheck your table name.");
	}

	return retval;
}

string CKnowledgeBase::_XMLSafe(const string& str)
{
	//replace any illegal characters with escapes
	string retval = EDSUTIL::FindAndReplace(str, "\"", "&quot;");
	retval = EDSUTIL::FindAndReplace(retval, "'", "&apos;");
	retval = EDSUTIL::FindAndReplace(retval, "<", "&lt;");
	retval = EDSUTIL::FindAndReplace(retval, ">", "&gt;");
	retval = EDSUTIL::FindAndReplace(retval, "&", "&amp;");
	return retval;
}

void CKnowledgeBase::_sendToDebugServer(const string& msg)
{
	try
	{
		if (m_DebugHandlerPtr)
		{
			m_DebugHandlerPtr(msg);
		}

		if (m_remoteDebugging)
		{
#ifndef DISABLE_DECISIONLOGIC_INTEGRATION
			boost::asio::io_service io_service;
			tcp::resolver resolver(io_service);
			vector<string> con = EDSUTIL::Split(m_DEBUGGING_CON, ":");
			tcp::resolver::query query(tcp::v4(), con[0], con[1]);
			tcp::resolver::iterator iterator = resolver.resolve(query);
			tcp::socket sock(io_service);
			sock.connect(*iterator);
			boost::asio::write(sock, boost::asio::buffer(msg.c_str(), msg.length()*sizeof(wchar_t)));
#endif
		}
	}
	catch (std::exception& e)
	{
		string msg = "CKnowledgeBase::ConnectToDebugServer()\n";
		msg += e.what();
		ReportError(msg);
	}
}

bool CKnowledgeBase::_debugThisTable(const string& tableName)
{
	if (m_DEBUGGING_MSGS && (m_remoteDebugging || m_DebugHandlerPtr != nullptr))
	{
		if (m_DebugTables.size() > 0)
		{
			vector<string>::iterator it = find(m_DebugTables.begin(), m_DebugTables.end(), tableName);
			if (it != m_DebugTables.end())
				return true;
			else
				return false;
		}
		else
			return true;
	}
	return false;
}

map<string, vector<string> > CKnowledgeBase::EvaluateTableWithParam(const string& tableName, bool bGetAll, std::string& param, void* context)
{
	map<string, vector<string> > retval;

	CRuleTable *table = m_TableSet.GetTable(tableName);
	if (table != nullptr)
	{
		vector<pair<string, vector<CRuleCell> > > outputAttrsValues = table->GetOutputAttrsValues();
		//for all the outputs get the results
		for (vector<pair<string, vector<CRuleCell> > >::iterator itOut = outputAttrsValues.begin(); itOut != outputAttrsValues.end(); itOut++)
		{
			vector<string> result = EvaluateTableWithParam(tableName, (*itOut).first, bGetAll, param, context);
			retval[(*itOut).first] = result;
		}
	}

	return retval;
}

//loading
CKnowledgeBase::CKnowledgeBase(string knowledge_file)
{
	CreateKnowledgeBase(knowledge_file);
}

bool CKnowledgeBase::CreateKnowledgeBaseFromString(string xmlStr)
{
	bool retval = false;

#ifdef USE_MSXML
	Document	xmlDocument;
	xmlDocument = nullptr;
#ifdef USEATL
	xmlDocument.CoCreateInstance("MSXML2.DOMDocument.6.0");
#else
	xmlDocument.CreateInstance("MSXML2.DOMDocument.6.0");
#endif
	xmlDocument->async = VARIANT_FALSE;
	xmlDocument->resolveExternals = VARIANT_FALSE;
	xmlDocument->setProperty("SelectionLanguage", "XPath");
	xmlDocument->setProperty("SelectionNamespaces", "");
	//// Turn on the new parser in MSXML6 for better standards compliance (leading whitespaces in attr values);
	////this must be done prior to loading the document
	xmlDocument->setProperty("NewParser", VARIANT_TRUE);

	try
	{
		VARIANT_BOOL ok = xmlDocument->loadXML(_bstr_t(xmlStr.c_str()));
		if (ok == VARIANT_TRUE)
		{
			retval = _parseXML(xmlDocument);
		}
	}
	catch(const _com_error& e)
	{
		ReportError(EDSUTIL::Narrow((wstring)(e.Description())));
	}
	xmlDocument.Release();
#endif

#ifdef USE_LIBXML
	xmlInitParser();
	Document xmlDocument = nullptr;
	xmlDocument = xmlParseMemory(xmlStr.c_str(), (int)xmlStr.size());
	if (xmlDocument != nullptr)
	{
		retval = _parseXML(xmlDocument);

		if (xmlDocument)
			xmlFreeDoc(xmlDocument);
	}
	xmlCleanupParser();
#endif
	m_TableSet.Initialize();
	return retval;
}

void CKnowledgeBase::SetMaxThreads(size_t threads)
{
	m_TableSet.SetMaxThreads(threads);
}

bool CKnowledgeBase::_parseXML(Document xmlDocument)
{
	//parse the table data and create tables in the tableset
	bool retval = false;
	vector<string> FormulaInputs;
	m_IsOpen = true;
#ifdef USE_MSXML
	Node TablesNode = xmlDocument->selectSingleNode("//Tables");
	string debug = VariantToMBCStr(TablesNode->attributes->getNamedItem("debug")->nodeValue);
	string debugTables = VariantToMBCStr(TablesNode->attributes->getNamedItem("debugtables")->nodeValue);
	if (debug == "true")
	{
		m_DEBUGGING_MSGS = true;
		string con = VariantToMBCStr(TablesNode->attributes->getNamedItem("connection")->nodeValue);
		if (con.length() > 0)
		{
			m_DEBUGGING_CON = con;
		}

		if (debugTables.length() > 0)
			m_DebugTables = EDSUTIL::Split(debugTables, ",");
	}
	else
		m_DEBUGGING_MSGS = false;

	NodeList allTables = xmlDocument->selectNodes("Tables/Table");
	vector<pair<string, vector<CRuleCell> > > InputAttrsTests;
	vector<pair<string, vector<CRuleCell> > > OutputAttrsValues;

	for (int i = 0; i < allTables->length; i++)
	{
		Node TableNode = allTables->item[i];

		NodeList inputList = TableNode->selectNodes("Inputs");
		InputAttrsTests = _getTableRowFromXML(inputList, xmlDocument);
		NodeList outputList = TableNode->selectNodes("Outputs");
		OutputAttrsValues = _getTableRowFromXML(outputList, xmlDocument);

		string name = VariantToMBCStr(TableNode->attributes->getNamedItem("name")->nodeValue);
		string sGetAll = VariantToMBCStr(TableNode->attributes->getNamedItem("getall")->nodeValue);
		bool bGetAll = false;
		if (sGetAll.length() > 0 && sGetAll[0] == L't')
			bGetAll = true;

		NodeList formulaInputNodes = TableNode->selectNodes("FormulaInput");
		if (formulaInputNodes != nullptr)
		{
			for (int j = 0; j < formulaInputNodes->length; j++)
			{
				Node formulaInputNode = formulaInputNodes->item[j];
				FormulaInputs.push_back(EDSUTIL::Narrow(formulaInputNode->Gettext()));
			}
		}

		m_TableSet.AddTable(InputAttrsTests, OutputAttrsValues, FormulaInputs, &m_stringsMap, name, bGetAll);
		FormulaInputs.clear();
		retval = true;
	}

	NodeList allTranslations = xmlDocument->selectNodes("//Translations/String");
	for (int i = 0; i < allTranslations->length; i++)
	{
		Node StringNode = allTranslations->item[i];
		NamedNodeMap attrs = StringNode->attributes;
		if (attrs)
		{
			size_t id = atoull(VariantToMBCStr(StringNode->attributes->getNamedItem("id")->nodeValue).c_str());
			for (int childAttr = 0; childAttr < attrs->length; childAttr++)
			{
				Node childNode = attrs->item[childAttr];
				string name = VariantToMBCStr(childNode->nodeName);
				if (name != "id")
				{
					string langType = name;
					string langValue = VariantToMBCStr(StringNode->attributes->getNamedItem(name.c_str())->nodeValue);
					pair<string, string> kvp;
					kvp.first = langType;
					kvp.second = langValue;
					unordered_map<size_t, std::unordered_map<string, string> >::iterator itFind = mapBaseIDtoTranslations.find(id);
					if (itFind != mapBaseIDtoTranslations.end())
					{
						unordered_map<string, string> *newTranlation = &itFind->second;
						newTranlation->insert(kvp);
					}
					else
					{
						unordered_map<string, string> newTranslation;
						newTranslation.insert(kvp);
						pair<size_t, unordered_map<string, string> > idTrans_kvp;
						idTrans_kvp.first = id;
						idTrans_kvp.second = newTranslation;
						mapBaseIDtoTranslations.insert(idTrans_kvp);
					}
				}
			}
		}
	}

	Node nodeJS = xmlDocument->selectSingleNode("//Javascript");
	Node nodePY = xmlDocument->selectSingleNode("//Python");
	if (nodeJS != nullptr)
		m_jsCode = EDSUTIL::Narrow(nodeJS->Gettext()) + "\n";
	if (nodePY != nullptr)
		m_pyCode = EDSUTIL::Narrow(nodePY->Gettext()) + "\n";

#endif

#ifdef USE_LIBXML
	xmlXPathContextPtr xpathCtx = xmlXPathNewContext(xmlDocument);

	xmlChar* tablesXPath = (xmlChar*)"//Tables";
	xmlXPathObjectPtr xpathTables = xmlXPathEvalExpression(tablesXPath, xpathCtx);

	Node tablesNode = xpathTables->nodesetval->nodeTab[0];
	string debug = EDSUTIL::XMLStrToStr(xmlGetProp(tablesNode, (xmlChar*)"debug"));
	string debugTables = EDSUTIL::XMLStrToStr(xmlGetProp(tablesNode, (xmlChar*)"debugtables"));
	if (debug == "true")
	{
		m_DEBUGGING_MSGS = true;
		string con = EDSUTIL::XMLStrToStr(xmlGetProp(tablesNode, (xmlChar*)"connection"));
		if (con.length() > 0)
		{
			m_DEBUGGING_CON = con;
		}

		if (debugTables.length() > 0)
			m_DebugTables = EDSUTIL::Split(debugTables, ",");
	}
	else
		m_DEBUGGING_MSGS = false;


	xmlChar* tableXPath = (xmlChar*)"//Tables/Table";
	xmlXPathObjectPtr xpathTable = xmlXPathEvalExpression(tableXPath, xpathCtx);
	NodeList allTables = xpathTable->nodesetval;

	if (allTables != nullptr)
	{
		for (int i = 0; i < allTables->nodeNr; i++)
		{
			Node TableNode = allTables->nodeTab[i];
			xpathCtx->node = TableNode;

			xmlXPathObjectPtr xpathObjInputs = xmlXPathEvalExpression((xmlChar*)"Inputs", xpathCtx);
			xmlXPathObjectPtr xpathObjOutputs = xmlXPathEvalExpression((xmlChar*)"Outputs", xpathCtx);

			NodeList inputList = xpathObjInputs->nodesetval;
			NodeList outputList = xpathObjOutputs->nodesetval;

			string name = EDSUTIL::XMLStrToStr(xmlGetProp(TableNode, (xmlChar*)"name"));
			string sGetAll = EDSUTIL::XMLStrToStr(xmlGetProp(TableNode, (xmlChar*)"getall"));
			bool bGetAll = false;
			if (sGetAll.length() > 0 && sGetAll[0] == L't')
				bGetAll = true;

			xmlXPathObjectPtr xpathObjFormulas = xmlXPathEvalExpression((xmlChar*)"FormulaInput", xpathCtx);
			NodeList formulaInputNodes = xpathObjFormulas->nodesetval;
			for (int j = 0; j < formulaInputNodes->nodeNr; j++)
			{
				Node formulaInputNode = formulaInputNodes->nodeTab[j];
				FormulaInputs.push_back(EDSUTIL::XMLStrToStr(xmlNodeGetContent(formulaInputNode)));
			}

			vector<pair<string, vector<CRuleCell> > > InputAttrsTests = _getTableRowFromXML(inputList, xmlDocument);
			vector<pair<string, vector<CRuleCell> > > OutputAttrsValues = _getTableRowFromXML(outputList, xmlDocument);


			m_TableSet.AddTable(InputAttrsTests, OutputAttrsValues, FormulaInputs, &m_stringsMap, name, bGetAll);
			FormulaInputs.clear();
			retval = true;

			xmlXPathFreeObject(xpathObjInputs);
			xmlXPathFreeObject(xpathObjOutputs);
			xmlXPathFreeObject(xpathObjFormulas);
		}
	}



	xpathCtx->node = xmlDocGetRootElement(xmlDocument);
	xmlChar* stringXPath = (xmlChar*)"//Translations/String";
	xmlXPathObjectPtr xpathStrings = xmlXPathEvalExpression(stringXPath, xpathCtx);
	if (xpathStrings != nullptr)
	{
		NodeList allTranslations = xpathStrings->nodesetval;
		if (allTranslations != nullptr)
		{
			for (int i = 0; i < allTranslations->nodeNr; i++)
			{
				Node StringNode = allTranslations->nodeTab[i];
				size_t id = atoull(EDSUTIL::XMLStrToStr(xmlGetProp(StringNode, (xmlChar*)"id")).c_str());
				for (Attribute childAttr = StringNode->properties; childAttr != nullptr; childAttr = childAttr->next)
				{
                    string name = (char*)childAttr->name;
                    if (name != "id")
                    {
                        string langType = name;
                        string langValue = EDSUTIL::XMLStrToStr(xmlGetProp(StringNode, (xmlChar*)name.c_str()));
                        pair<string, string> kvp;
                        kvp.first = langType;
                        kvp.second = langValue;
                        unordered_map<size_t, std::unordered_map<string, string> >::iterator itFind = mapBaseIDtoTranslations.find(id);
                        if (itFind != mapBaseIDtoTranslations.end())
                        {
                            unordered_map<string, string> *newTranlation = &itFind->second;
                            newTranlation->insert(kvp);
                        }
                        else
                        {
                            unordered_map<string, string> newTranslation;
                            newTranslation.insert(kvp);
                            pair<size_t, unordered_map<string, string> > idTrans_kvp;
                            idTrans_kvp.first = id;
                            idTrans_kvp.second = newTranslation;
                            mapBaseIDtoTranslations.insert(idTrans_kvp);
                        }
                    }
				}
			}
		}
	}

	xmlXPathObjectPtr xpathJS = xmlXPathEvalExpression((xmlChar*)"//Javascript", xpathCtx);
	xmlXPathObjectPtr xpathPY = xmlXPathEvalExpression((xmlChar*)"//Python", xpathCtx);
	if (xpathJS != nullptr && xpathJS->nodesetval != nullptr && xpathJS->nodesetval->nodeNr == 1)
		m_jsCode = EDSUTIL::XMLStrToStr(xmlNodeGetContent(xpathJS->nodesetval->nodeTab[0])) + "\n";
	if (xpathJS != nullptr && xpathJS->nodesetval != nullptr && xpathPY->nodesetval->nodeNr == 1)
		m_pyCode = EDSUTIL::XMLStrToStr(xmlNodeGetContent(xpathPY->nodesetval->nodeTab[0])) + "\n";

	xmlXPathFreeObject(xpathJS);
	xmlXPathFreeObject(xpathPY);
	xmlXPathFreeObject(xpathTables);
	xmlXPathFreeObject(xpathTable);
	xmlXPathFreeContext(xpathCtx);
#endif
	return retval;
}

bool CKnowledgeBase::CreateKnowledgeBase(string knowledge_file)
{
	bool retval = false;
	m_IsOpen = false;
	iRecursingDepth = 0;
	m_DebugHandlerPtr = nullptr;
	m_InputValueGetterPtr = nullptr;
	m_DEBUGGING_MSGS = false;
	m_remoteDebugging = false;
	mapBaseIDtoTranslations.clear();
#ifdef WIN32
	HRESULT hr = CoInitialize(nullptr);
#endif

	try
	{
	    string file_name, pathSep;
		#ifdef POSIX
		pathSep = "/";
		#else
		pathSep = "\\";
		#endif

		string sFileName = knowledge_file, sPath, sExtension, unzippedFileName;
		size_t pos = sFileName.find_last_of(pathSep);
		sPath = sFileName.substr(0, pos);
		pos = sFileName.find_last_of(".");
		sExtension = sFileName.substr(pos + 1);

		unzippedFileName = sFileName;

#ifndef DISABLE_ZIP
		if (sExtension == "gz")
		{
			//get the directory to extract files to
            char * pcPath = nullptr;
#ifdef POSIX
            pcPath = getenv("TMPDIR");
#ifdef P_tmpdir
            if (pcPath == nullptr)
                pcPath = P_tmpdir;
#else
            if (pcPath == nullptr)
                pcPath = _PATH_TMP;
#endif //P_tmpdir
#else //POSIX
			pcPath = getenv("TEMP");
#endif //POSIX
			unzippedFileName = pcPath;

			file_name = knowledge_file.substr(knowledge_file.find_last_of(pathSep) + 1);
			file_name = EDSUTIL::FindAndReplace(file_name, ".gz", ".xml");

			//unzip
			ifstream file(knowledge_file.c_str(), std::ifstream::in | std::ifstream::binary);
			if (file.is_open())
			{
				boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
				in.push(boost::iostreams::gzip_decompressor());
				in.push(file);
				unzippedFileName += pathSep;
				unzippedFileName += file_name;
				ofstream outfile(unzippedFileName.c_str(), std::ofstream::out);
				if (outfile.is_open())
				{
                    boost::iostreams::copy(in, outfile);
                    outfile.close();
				}
				file.close();
			}
		}
#endif //DISABLE_ZIP



		//parse the table from xml
		#ifdef USE_MSXML
			Document	xmlDocument;
			xmlDocument = nullptr;
			#ifdef USEATL
				xmlDocument.CoCreateInstance("MSXML2.DOMDocument.6.0");
			#else
				xmlDocument.CreateInstance("MSXML2.DOMDocument.6.0");
			#endif
			xmlDocument->async = VARIANT_FALSE;
			xmlDocument->resolveExternals = VARIANT_FALSE;
			xmlDocument->setProperty("SelectionLanguage", "XPath");
			xmlDocument->setProperty("SelectionNamespaces", "");
			//// Turn on the new parser in MSXML6 for better standards compliance (leading whitespaces in attr values);
			////this must be done prior to loading the document
			xmlDocument->setProperty("NewParser", VARIANT_TRUE);

			try
			{
				VARIANT_BOOL ok = xmlDocument->load(unzippedFileName.c_str());

				if (ok)
				{
					retval = _parseXML(xmlDocument);
				}
			}
			catch(const _com_error& e)
			{
				ReportError(EDSUTIL::Narrow((wstring)(e.Description())));
			}
			xmlDocument.Release();
		#endif

		#ifdef USE_LIBXML
			xmlInitParser();
			Document xmlDocument = nullptr;

			std::string strBuff(unzippedFileName.begin(), unzippedFileName.end());
			xmlDocument = xmlParseFile(strBuff.c_str());

			if (xmlDocument != nullptr)
			{
				retval = _parseXML(xmlDocument);

				if (xmlDocument)
					xmlFreeDoc(xmlDocument);
			}
			xmlCleanupParser();
		#endif

#ifndef DISABLE_ZIP
		//delete extracted file
		if (sExtension == "gz")
			remove(unzippedFileName.c_str());
#endif
		m_TableSet.Initialize();
	}
	catch (std::exception &ex)
	{
		ReportError(ex.what());
	}
	catch (...)
	{
		ReportError("CKnowledgeBase::CreateKnowledgeBase");
		retval = false;
	}
	return retval;
}

//private
vector<pair<string, vector<CRuleCell> > > CKnowledgeBase::_getTableRowFromXML(NodeList nodes, Document xmlDocument)
{
	vector<pair<string, vector<CRuleCell> > > retval;
	try
	{
	#ifdef USE_MSXML
		long nodeCnt = nodes->length;
		for (long i = 0; i < nodeCnt; i++)
		{
			pair<string, vector<CRuleCell> > currentAttrRow;

			Node currentInputAttr = nodes->item[i];
			NodeList values = currentInputAttr->selectNodes("Value");
			Node attrNode = currentInputAttr->selectSingleNode("Attr");
			if (attrNode != nullptr)
			{
				string attrName = EDSUTIL::Narrow(attrNode->Gettext());
				currentAttrRow.first = attrName;
				for (long j = 0; j < values->length; j++)
				{
					Node currentValue = values->item[j];
					Node idNode = currentValue->attributes->getNamedItem("id");
					string sIDs;
					CRuleCell cell;

					if (idNode != nullptr)
					{
						sIDs = VariantToMBCStr(idNode->nodeValue);
						vector<string> cellValues = EDSUTIL::Split(EDSUTIL::Narrow(currentValue->Gettext()), "|");
						vector<string> ids = EDSUTIL::Split(sIDs, ",");
						if (ids.size() != cellValues.size())
							throw "Bad OR";

						for (auto idCnt = 0; idCnt < ids.size(); idCnt++)
						{
							long id = atoull(ids[idCnt].c_str());
							string value = cellValues[idCnt];
							m_stringsMap.AddString(id, value);
							cell.Values.push_back(id);
						}
					}

					Node operNode = nullptr;
					operNode = currentValue->Getattributes()->getNamedItem("operation");
					string sOper = "";
					long lOper = 0;
					if (operNode != nullptr)
					{
						sOper = VariantToMBCStr(operNode->nodeValue);
						string sOper(sOper.begin(), sOper.end());
						lOper = atoull(sOper.c_str());
					}
					cell.Operation = lOper;
					currentAttrRow.second.push_back(cell);

				}
				if (attrName.length() > 0)
					retval.push_back(currentAttrRow);
			}
		}
	#endif

	#ifdef USE_LIBXML
		xmlXPathContextPtr xpathCtx = xmlXPathNewContext(xmlDocument);
		size_t nodeCnt = nodes->nodeNr;
		for (size_t i = 0; i < nodeCnt; i++)
		{
			pair<string, vector<CRuleCell> > currentAttrRow;

			Node currentInputAttr = nodes->nodeTab[i];
			xpathCtx->node = currentInputAttr;

			xmlXPathObjectPtr xmlXPathObjValues = xmlXPathEvalExpression((xmlChar*)"Value", xpathCtx);
			xmlXPathObjectPtr xmlXPathObjAttr = xmlXPathEvalExpression((xmlChar*)"Attr", xpathCtx);
			NodeList values = xmlXPathObjValues->nodesetval;
			if (xmlXPathObjAttr->nodesetval->nodeNr > 0)
			{
				Node attrNode = xmlXPathObjAttr->nodesetval->nodeTab[0];

				string attrName = EDSUTIL::XMLStrToStr(xmlNodeGetContent(attrNode));
				currentAttrRow.first = attrName;
				if (values != nullptr)
				{
					for (int j = 0; j < values->nodeNr; j++)
					{
						Node currentValue = values->nodeTab[j];
						string sIDs = EDSUTIL::XMLStrToStr(xmlGetProp(currentValue, (xmlChar*)"id"));

						CRuleCell cell;
						if (sIDs.length() > 0)
						{
							vector<string> cellValues = EDSUTIL::Split(EDSUTIL::XMLStrToStr(xmlNodeGetContent(currentValue)), "|");
							vector<string> ids = EDSUTIL::Split(sIDs, ",");
							if (ids.size() != cellValues.size())
								throw "Bad OR";

							for (auto idCnt = 0; idCnt < ids.size(); idCnt++)
							{
								size_t id = atoull(ids[idCnt].c_str());
								string value = cellValues[idCnt];
								m_stringsMap.AddString(id, value);
								cell.Values.push_back(id);
							}
						}

						string sOper = EDSUTIL::XMLStrToStr(xmlGetProp(currentValue, (xmlChar*)"operation"));
						long lOper = 0;
						if (sOper.length() > 0)
						{
							lOper = atoull(sOper.c_str());
						}
						cell.Operation = lOper;
						currentAttrRow.second.push_back(cell);
					}

				}
				if (attrName.length() > 0)
					retval.push_back(currentAttrRow);
			}

			xmlXPathFreeObject(xmlXPathObjValues);
			xmlXPathFreeObject(xmlXPathObjAttr);
		}
		xmlXPathFreeContext(xpathCtx);
	#endif

	}
	catch (exception& ex)
	{
		ReportError(ex.what());
	}
	catch(...)
	{
		ReportError("CKnowledgeBase::_getTableRowFromXML");
	}

	return retval;
}
