/*
This file is part of EDSEngineNET.
Copyright (C) 2009-2015 Eric D. Schmidt, DigiRule Solutions LLC

    EDSEngineNET is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    EDSEngineNET is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EDSEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
// This is the main DLL file.

#include "stdafx.h"

#include "EDSEngine.h"
#include <vector>
#include <map>
#include <hash_map>
#include <functional>

using namespace std;
using namespace msclr::interop;
using namespace System::Runtime::InteropServices;

namespace EDSNET
{
	bool EDSEngine::CreateKnowledgeBase(System::String^ knowledge_file)
	{
		string file = MarshalString(knowledge_file);
		DebugDelegate = nullptr;
		m_KnowledgeBase = new EDS::CKnowledgeBase();	
		return m_KnowledgeBase->CreateKnowledgeBase(file);
	}

	bool EDSEngine::CreateKnowledgeBaseFromString(System::String^ rules)
	{
		string rulesStr = MarshalString(rules);
		DebugDelegate = nullptr;
		m_KnowledgeBase = new EDS::CKnowledgeBase();
		return m_KnowledgeBase->CreateKnowledgeBaseFromString(rulesStr);
	}

	bool EDSEngine::CreateKnowledgeBase(IntPtr ptr)
	{
		if (ptr.ToPointer() != nullptr)
		{
			DebugDelegate = nullptr;
			m_KnowledgeBase = (EDS::CKnowledgeBase*)ptr.ToPointer();
			return true;
		}
		else
			return false;
	}

	void EDSEngine::SetMaxThreads(size_t threads)
	{
		if (m_KnowledgeBase)
		{
			m_KnowledgeBase->SetMaxThreads(threads);
		}
	}

	size_t EDSEngine::TableCount()
	{
		if (m_KnowledgeBase)
		{
			return m_KnowledgeBase->TableCount();
		}
		else
			return 0;
	}

	bool EDSEngine::IsOpen()
	{
		if (m_KnowledgeBase)
			return m_KnowledgeBase->IsOpen();
		else 
			return false;
	}

	bool EDSEngine::TableHasScript(String^ tableName)
	{
		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			return m_KnowledgeBase->TableHasScript(table);
		}
		else
			return false;
	}

	bool EDSEngine::TableIsGetAll(String^ tableName)
	{
		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			return m_KnowledgeBase->TableIsGetAll(table);
		}
		else
			return false;
	}

	cli::array<String^>^ EDSEngine::EvaluateTableWithParam(String^ tableName, String^ outputAttr, bool bGetAll, String^% param, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string output = MarshalString(outputAttr);
			string para = MarshalString(param);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			vector<string> res = m_KnowledgeBase->EvaluateTableWithParam(table, output, bGetAll, para, voidPtr);
			retval = GetArrayFromVectorStrings(res);
			param = gcnew String(para.c_str());
		}

		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ EDSEngine::EvaluateTableWithParam(String^ tableName, bool bGetAll, String^% param, Object^ context)
	{
		Dictionary<String^,	cli::array<String^>^>^ retval = nullptr;
		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string para = MarshalString(param);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			map<string, vector<string> > res = m_KnowledgeBase->EvaluateTableWithParam(table, bGetAll, para, voidPtr);
			retval = GetDictionaryFromMapStrings(res);
			param = gcnew String(para.c_str());
		}
		return retval;
	}

	cli::array<String^>^ EDSEngine::EvaluateTable(String^ tableName, String^ outputAttr, bool bGetAll, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string output = MarshalString(outputAttr);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			vector<string> res = m_KnowledgeBase->EvaluateTable(table, output, bGetAll, voidPtr);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ EDSEngine::EvaluateTable(String^ tableName, bool bGetAll, Object^ context)
	{
		Dictionary<String^,	cli::array<String^>^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			map<string, vector<string> > res = m_KnowledgeBase->EvaluateTable(table, bGetAll, voidPtr);
			retval = GetDictionaryFromMapStrings(res);
		}

		return retval;
	}

	String^ EDSEngine::GetFirstTableResult(String^ tableName, String^ outputAttr, Object^ context)
	{
		String^ retval = gcnew String("");
		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string output = MarshalString(outputAttr);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			string res = m_KnowledgeBase->GetFirstTableResult(table, output, voidPtr);
			retval = gcnew String(res.c_str());
		}
		return retval;
	}

	cli::array<String^>^	EDSEngine::ReverseEvaluateTable(String^ tableName, String^ inputAttr, bool bGetAll, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string input = MarshalString(inputAttr);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			vector<string> res = m_KnowledgeBase->ReverseEvaluateTable(table, input, bGetAll, voidPtr);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ EDSEngine::ReverseEvaluateTable(String^ tableName, bool bGetAll, Object^ context)
	{
		Dictionary<String^,	cli::array<String^>^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			void* voidPtr = nullptr;
			if (context)
				voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();
			map<string, vector<string> > res = m_KnowledgeBase->ReverseEvaluateTable(table, bGetAll, voidPtr);
			retval = GetDictionaryFromMapStrings(res);
		}

		return retval;
	}

	cli::array<String^>^ EDSEngine::GetInputAttrs(String^ tableName)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			vector<string> res = m_KnowledgeBase->GetInputAttrs(table);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	cli::array<String^>^ EDSEngine::GetInputDependencies(String^ tableName)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			vector<string> res = m_KnowledgeBase->GetInputDependencies(table);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	cli::array<String^>^ EDSEngine::GetOutputAttrs(String^ tableName)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			vector<string> res = m_KnowledgeBase->GetOutputAttrs(table);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	cli::array<String^>^ EDSEngine::GetAllPossibleOutputs(String^ tableName, String^ outputName)
	{
		cli::array<String^>^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string table = MarshalString(tableName);
			string output = MarshalString(outputName);
			vector<string> res = m_KnowledgeBase->GetAllPossibleOutputs(table, output);
			retval = GetArrayFromVectorStrings(res);
		}

		return retval;
	}

	String^	EDSEngine::DeLocalize(String^ localeValue)
	{
		String^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string val = MarshalString(localeValue);
			string res = m_KnowledgeBase->DeLocalize(val);
			retval = gcnew String(res.c_str());
		}

		return retval;
	}

	String^	EDSEngine::Translate(String^ source, String^ sourceLocale, String^ destLocale)
	{
		String^ retval = nullptr;

		if (m_KnowledgeBase)
		{
			string src = MarshalString(source);
			string srcLocale = MarshalString(sourceLocale);
			string dest = MarshalString(destLocale);
			string res = m_KnowledgeBase->Translate(src, srcLocale, dest);
			retval = gcnew String(res.c_str());
		}

		return retval;
	}

	string	EDSEngine::_getValue(const string& attrName, void* context)
	{
		string retval;
		if (InputGetterDelegate != nullptr)
		{
			Object^ ctxt = nullptr;
			GCHandle handle;
			if (context != nullptr)
			{
				handle = GCHandle::FromIntPtr(IntPtr(context));
				ctxt = handle.Target;
			}
			String^ attrValue = InputGetterDelegate(gcnew String(attrName.c_str()), ctxt);

			retval = MarshalString(attrValue);
		}
		return retval;
	}

	void EDSEngine::_fireDebug(const string& msg)
	{
		if (DebugDelegate != nullptr)
		{
			DebugDelegate(gcnew String(msg.c_str()));
		}
	}
}