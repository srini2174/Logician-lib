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
// EDSEngine.h

#pragma once
#pragma unmanaged
#include "KnowledgeBase.h"
#include <vector>
#include <array>
#include <map>
#include "Marshal.h"
using namespace std;

typedef string(__stdcall *VALUECB)(const string&, void*);
typedef void(__stdcall *DEBUGCB)(const string&);

#pragma managed
#using <mscorlib.dll>
#using <System.dll>
#include <vcclr.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace EDSNET {
	delegate void FireDebugMessageDelegate(const string&);
	delegate string GetTheValueDelegate(const string&, void* context);

	public delegate void DebugHandlerDelegate(String^ msg);
	public delegate String^ InputValueGetterDelegate(String^ attrName, Object^ context);

	public ref class EDSEngine
	{
	public:
		EDSEngine() { DebugDelegate = nullptr; InputGetterDelegate = nullptr; }
		EDSEngine(String^ knowledge_file) {CreateKnowledgeBase(knowledge_file);}
		EDSEngine(IntPtr ptr) { CreateKnowledgeBase(ptr); }
		virtual bool CreateKnowledgeBase(String^ knowledge_file);
		virtual bool CreateKnowledgeBase(IntPtr ptr);
		virtual bool EDSEngine::CreateKnowledgeBaseFromString(System::String^ rules);
		virtual ~EDSEngine() 
		{
			if (m_gchInput.IsAllocated)			
				m_gchInput.Free();
			if (m_gchDebug.IsAllocated)
				m_gchDebug.Free();
			this->!EDSEngine();
		}		
		!EDSEngine() {if (m_KnowledgeBase) delete m_KnowledgeBase; m_KnowledgeBase = NULL;}
		virtual void SetMaxThreads(size_t threads);

		virtual property DebugHandlerDelegate^			DebugDelegate
		{
			DebugHandlerDelegate^ get()
			{
				return m_debugger;
			}
			void set(DebugHandlerDelegate^ value)
			{
				m_debugger = value;
				if (m_KnowledgeBase != nullptr)
				{
					if (m_debugger != nullptr)
					{
						FireDebugMessageDelegate^ fp = gcnew FireDebugMessageDelegate(this, &EDSEngine::_fireDebug);
						m_gchDebug = GCHandle::Alloc(fp);
						IntPtr ip = Marshal::GetFunctionPointerForDelegate(fp);
						DEBUGCB cb = static_cast<DEBUGCB>(ip.ToPointer());
						m_KnowledgeBase->SetDebugHandler(cb);
					}
					else
					{
						m_KnowledgeBase->SetDebugHandler(nullptr);
					}
				}
			}
		}

		virtual property InputValueGetterDelegate^		InputGetterDelegate
		{
			InputValueGetterDelegate^ get()
			{
				return m_getter;
			}
			void set(InputValueGetterDelegate^ value)
			{
				m_getter = value;
				if (m_KnowledgeBase != nullptr)
				{
					if (m_getter != nullptr)
					{
						GetTheValueDelegate^ fp = gcnew GetTheValueDelegate(this, &EDSEngine::_getValue);
						m_gchInput = GCHandle::Alloc(fp);
						IntPtr ip = Marshal::GetFunctionPointerForDelegate(fp);
						VALUECB cb = static_cast<VALUECB>(ip.ToPointer());
						m_KnowledgeBase->SetInputValueGetter(cb);
					}
					else
					{
						m_KnowledgeBase->SetInputValueGetter(nullptr);
					}
				}
			}
		}
		virtual void									EnableRemoteDebugger(bool enable) { if (m_KnowledgeBase) m_KnowledgeBase->EnableRemoteDebugger(enable); }

		virtual size_t									TableCount();
		virtual bool									IsOpen();
		virtual bool									TableHasScript(String^ tableName);
		virtual bool									TableIsGetAll(String^ tableName);

		virtual cli::array<String^>^							EvaluateTableWithParam(String^ tableName, String^ outputAttr, bool bGetAll, String^% param, Object^ context);
		virtual cli::array<String^>^							EvaluateTableWithParam(String^ tableName, String^ outputAttr, String^% param, Object^ context) { return EvaluateTableWithParam(tableName, outputAttr, TableIsGetAll(tableName), param, context); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTableWithParam(String^ tableName, String^% param, Object^ context) { return EvaluateTableWithParam(tableName, TableIsGetAll(tableName), param, context); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTableWithParam(String^ tableName, bool bGetAll, String^% param, Object^ context);
		virtual cli::array<String^>^							EvaluateTable(String^ tableName, String^ outputAttr, Object^ context) { return EvaluateTable(tableName, outputAttr, TableIsGetAll(tableName), context); }
		virtual cli::array<String^>^							EvaluateTable(String^ tableName, String^ outputAttr, bool bGetAll, Object^ context);
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTable(String^ tableName, Object^ context) { return EvaluateTable(tableName, TableIsGetAll(tableName), context); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTable(String^ tableName, bool bGetAll, Object^ context);
		virtual String^									GetFirstTableResult(String^ tableName, String^ ouputAttr, Object^ context);
		virtual cli::array<String^>^							ReverseEvaluateTable(String^ tableName, String^ inputAttr, Object^ context) { return ReverseEvaluateTable(tableName, inputAttr, TableIsGetAll(tableName), context); }
		virtual cli::array<String^>^							ReverseEvaluateTable(String^ tableName, String^ inputAttr, bool bGetAll, Object^ context);
		virtual Dictionary<String^, cli::array<String^>^>^	ReverseEvaluateTable(String^ tableName, Object^ context) { return ReverseEvaluateTable(tableName, TableIsGetAll(tableName), context); }
		virtual Dictionary<String^, cli::array<String^>^>^	ReverseEvaluateTable(String^ tableName, bool bGetAll, Object^ context);

		virtual cli::array<String^>^							EvaluateTableWithParam(String^ tableName, String^ outputAttr, bool bGetAll, String^% param) { return EvaluateTableWithParam(tableName, outputAttr, TableIsGetAll(tableName), param, nullptr); }
		virtual cli::array<String^>^							EvaluateTableWithParam(String^ tableName, String^ outputAttr, String^% param) { return EvaluateTableWithParam(tableName, outputAttr, TableIsGetAll(tableName), param, nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTableWithParam(String^ tableName, String^% param) { return EvaluateTableWithParam(tableName, TableIsGetAll(tableName), param, nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTableWithParam(String^ tableName, String^% param, bool bGetAll) { return EvaluateTableWithParam(tableName, bGetAll, param, nullptr); }
		virtual cli::array<String^>^							EvaluateTable(String^ tableName, String^ outputAttr) { return EvaluateTable(tableName, outputAttr, TableIsGetAll(tableName), nullptr); }
		virtual cli::array<String^>^							EvaluateTable(String^ tableName, String^ outputAttr, bool bGetAll) { return EvaluateTable(tableName, outputAttr, bGetAll, nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTable(String^ tableName) { return EvaluateTable(tableName, TableIsGetAll(tableName), nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	EvaluateTable(String^ tableName, bool bGetAll) { return EvaluateTable(tableName, bGetAll, nullptr); }
		virtual String^									GetFirstTableResult(String^ tableName, String^ ouputAttr) { return GetFirstTableResult(tableName, ouputAttr, nullptr); }
		virtual cli::array<String^>^							ReverseEvaluateTable(String^ tableName, String^ inputAttr) { return ReverseEvaluateTable(tableName, inputAttr, TableIsGetAll(tableName), nullptr); }
		virtual cli::array<String^>^							ReverseEvaluateTable(String^ tableName, String^ inputAttr, bool bGetAll) { return ReverseEvaluateTable(tableName, inputAttr, bGetAll, nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	ReverseEvaluateTable(String^ tableName) { return ReverseEvaluateTable(tableName, TableIsGetAll(tableName), nullptr); }
		virtual Dictionary<String^, cli::array<String^>^>^	ReverseEvaluateTable(String^ tableName, bool bGetAll) { return ReverseEvaluateTable(tableName, bGetAll, nullptr); }

		virtual cli::array<String^>^							GetInputAttrs(String^ tableName);
		virtual cli::array<String^>^							GetInputDependencies(String^ tableName);
		virtual cli::array<String^>^							GetOutputAttrs(String^ tableName);
		virtual cli::array<String^>^							GetAllPossibleOutputs(String^ tableName, String^ outputName);

		virtual String^									Localize(String^ baseValue, String^ locale) { return Translate(baseValue, gcnew String(""), locale); }
		virtual String^									DeLocalize(String^ localeValue);
		virtual String^									Translate(String^ source, String^ sourceLocale, String^ destLocale);

		virtual IntPtr									GetEDSPtr() { return (IntPtr)m_KnowledgeBase; }

	private:		
		string									_getValue(const string& attrName, void* context);
		void									_fireDebug(const string& msg);

		DebugHandlerDelegate^					m_debugger;
		InputValueGetterDelegate^				m_getter;
		EDS::IKnowledgeBase						*m_KnowledgeBase;
		GCHandle								m_gchInput, m_gchDebug;
	};
}
