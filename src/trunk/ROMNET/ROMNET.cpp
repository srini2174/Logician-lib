/*
This file is part of ROMNET.
Copyright (C) 2009-2015 Eric D. Schmidt, DigiRule Solutions LLC

    ROMNET is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    ROMNET is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ROMNET.  If not, see <http://www.gnu.org/licenses/>.
*/
// This is the main DLL file.

#include "stdafx.h"
#include "ROMNET.h"

struct ROM::LinearEngine::DispatchHelper
{
	static void _evaluateForAttribute(ROM::LinearEngine* p, const string& dictAttrName, vector<string>& newValues, bool bEvalDependents, void* context) { p->_evaluateForAttribute(dictAttrName, newValues, bEvalDependents, context); }
	static void _evaluateForAttribute(ROM::LinearEngine* p, const string& dictAttrName, const string& newValue, bool bEvalDependents, void* context) { p->_evaluateForAttribute(dictAttrName, newValue, bEvalDependents, context); }
	static void _evaluateAll(ROM::LinearEngine* p, void* context) { p->_evaluateAll(context); }
	static void _initializeEngine(ROM::LinearEngine* p, void* context) { p->_initializeEngine(context); }
	static void _resetEngine(ROM::LinearEngine* p, void* context) { p->_resetEngine(context); }
};

struct ROM::ROMNode::DispatchHelper
{
	static vector<string>		_evaluateTable(ROM::ROMNode* p, const string& evalTable, const string& output, bool bGetAll, void* context) { return p->_evaluateTable(evalTable, output, bGetAll, context); }
	static vector<string>		_evaluateTable(ROM::ROMNode* p, const string& evalTable, const string& output, void* context) { return p->_evaluateTable(evalTable, output, context); }
	static map<string, vector<string> > _evaluateTable(ROM::ROMNode* p, const string& evalTable, bool bGetAll, void* context) { return p->_evaluateTable(evalTable, bGetAll, context); } 
	static map<string, vector<string> > _evaluateTable(ROM::ROMNode* p, const string& evalTable, void* context) { return p->_evaluateTable(evalTable, context); }
	static vector<string>		_evaluateTableWithParam(ROM::ROMNode* p, const string& evalTable, const string& output, bool bGetAll, string& param, void* context) { return p->_evaluateTableWithParam(evalTable, output, bGetAll, param, context); }
	static vector<string>		_evaluateTableWithParam(ROM::ROMNode* p, const string& evalTable, const string& output, string& param, void* context) { return p->_evaluateTableWithParam(evalTable, output, param, context); }
	static map<string, vector<string> > _evaluateTableWithParam(ROM::ROMNode* p, const string& evalTable, bool bGetAll, string& param, void* context) { return p->_evaluateTableWithParam(evalTable, bGetAll, param, context); }
	static map<string, vector<string> > _evaluateTableWithParam(ROM::ROMNode* p, const string& evalTable, string& param, void* context) { return p->_evaluateTableWithParam(evalTable, param, context); }
	static string				_getFirstTableResult(ROM::ROMNode* p, const string& tableName, const string& output, void* context) { return p->_getFirstTableResult(tableName, output, context); }
	static vector<string>		_reverseEvaluateTable(ROM::ROMNode* p, const string& evalTable, const string& inputAttr, bool bGetAll, void* context) { return p->_reverseEvaluateTable(evalTable, inputAttr, bGetAll, context); }
	static vector<string>		_reverseEvaluateTable(ROM::ROMNode* p, const string& evalTable, const string& inputAttr, void* context) { return  p->_reverseEvaluateTable(evalTable, inputAttr, context); }
	static map<string, vector<string> > _reverseEvaluateTable(ROM::ROMNode* p, const string& evalTable, bool bGetAll, void* context) { return p->_reverseEvaluateTable(evalTable, bGetAll, context); }
	static map<string, vector<string> > _reverseEvaluateTable(ROM::ROMNode* p, const string& evalTable, void* context) { return p->_reverseEvaluateTable(evalTable, context); }
};

struct ROM::IDictionaryInterface::DispatchHelper
{
	static void _loadDictionary(ROM::IDictionaryInterface* p, const std::string& dictionaryTable, void* context) { p->_loadDictionary(dictionaryTable, context); }
};

namespace ROMNET
{
	bool ROMNode::CreateROMNode(System::String^ id, ROMObjectFactoryDelegate^ factory, IntPtr ptr)
	{
		string sID = MarshalString(id);	
		if (ptr.ToPointer() == nullptr)
			m_ROMNode = new ROM::ROMNode(sID);
		else
			m_ROMNode = (ROM::ROMNode*)ptr.ToPointer();

		if (factory == nullptr)
			ROMObjectFactory = gcnew ROMObjectFactoryDelegate(&ROMNode::_managedFactory);
		else
			ROMObjectFactory = factory;

		m_managedTreeObjects = gcnew Dictionary<String^, ROMNode^>();
		m_managedTreeObjects[GetROMGUID()] = this;
		
		m_canDelete = true;

		return true;
	}

	ROMNode^ ROMNode::_managedFactory(String^ id)
	{
		return gcnew ROMNode(id);
	}

	String^ ROMNode::_managedGetter(String^ attrName, Object^ context)
	{
		return ((ROMNode^)context)->GetAttribute(attrName, false);
	}

	ROMNode^ ROMNode::GetRoot()
	{
		ROMNode^ retval = nullptr;
		if (m_ROMNode)
		{
			ROM::ROMNode *rootNode = m_ROMNode->GetRoot();
			if (rootNode)
			{
				retval = m_managedTreeObjects[gcnew String(rootNode->GetROMGUID().c_str())];
			}
		}
		return retval;
	}

	ROMNode^ ROMNode::Parent()
	{
		ROMNode^ retval = nullptr;
		if (m_ROMNode)
		{
			ROM::ROMNode *parentNode = m_ROMNode->GetParent();
			if (parentNode)
			{
				retval = m_managedTreeObjects[gcnew String(parentNode->GetROMGUID().c_str())];
			}
		}
		return retval;
	}

	cli::array<ROMNode^>^ ROMNode::GetAllChildren(bool recurs)
	{
		cli::array<ROMNode^>^ retval = gcnew cli::array<ROMNode^>(0);
		if (m_ROMNode)
		{
			vector<ROM::ROMNode*> vChildren = m_ROMNode->GetAllChildren(recurs);
			retval = _getArrayFromVectorROM(vChildren);
		}
		
		return retval;
	}

	cli::array<ROMNode^>^ ROMNode::FindObjects(String^ xpath)
	{
		cli::array<ROMNode^>^ retval = gcnew cli::array<ROMNode^>(0);
		if (m_ROMNode)
		{
			string sxpath = MarshalString(xpath);
			vector<ROM::ROMNode*> vChildren = m_ROMNode->FindObjects(sxpath);
			retval = _getArrayFromVectorROM(vChildren);
		}

		return retval;
	}

	ROMNode^ ROMNode::FindFirstObject(String^ xpath)
	{
		auto objs = FindObjects(xpath);
		if (objs->Length > 0)
			return objs[0];
		else
			return nullptr;
	}

	cli::array<ROMNode^>^ ROMNode::FindAllObjectsByID(String^ id, bool recurs)
	{
		cli::array<ROMNode^>^ retval = gcnew cli::array<ROMNode^>(0);
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			vector<ROM::ROMNode*> vChildren = m_ROMNode->FindAllObjectsByID(sID, recurs);
			retval = _getArrayFromVectorROM(vChildren);
		}
		
		return retval;
	}

	ROMNode^ ROMNode::FindObjectByGUID(String^ guid)
	{
		ROMNode^ retval = nullptr;
		if (m_ROMNode)
		{
			string sguid = MarshalString(guid);
			ROM::ROMNode* node = m_ROMNode->FindObjectByGUID(sguid);
			if (node != nullptr)
				retval = m_managedTreeObjects[gcnew String(sguid.c_str())];
		}

		return retval;
	}

	bool ROMNode::AddChildROMObject(ROMNode^ child)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			retval = m_ROMNode->AddChildROMObject(child->m_ROMNode);
			for each (KeyValuePair<String^, ROMNode^> kvp in child->m_managedTreeObjects)
			{
				m_managedTreeObjects[kvp.Key] = kvp.Value;				
			}
			child->m_managedTreeObjects  = m_managedTreeObjects;
			child->m_canDelete = false;
		}
		return retval;
	}

	bool ROMNode::RemoveChildROMObject(ROMNode^ child)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			retval = m_ROMNode->RemoveChildROMObject(child->m_ROMNode);
			m_managedTreeObjects->Remove(child->GetROMGUID());
			child->m_managedTreeObjects = gcnew Dictionary<String^, ROMNode^>();
			for each (ROMNode^ obj in child->GetAllChildren(true))
			{
				String^ guid = obj->GetROMGUID();
				m_managedTreeObjects->Remove(guid);
				child->m_managedTreeObjects[guid] = obj;
			}
			child->m_canDelete = true;
		}
		return retval;
	}

	bool ROMNode::RemoveFromParent()
	{
		bool retval = false;
		if (m_ROMNode)
		{
			ROMNode^ parent = Parent();
			retval = m_ROMNode->RemoveFromParent();

			if (retval == true && parent != nullptr)
			{
				parent->m_managedTreeObjects->Remove(GetROMGUID());
				m_managedTreeObjects = gcnew Dictionary<String^, ROMNode^>();
				for each (ROMNode^ obj in GetAllChildren(true))
				{
					String^ guid = obj->GetROMGUID();
					parent->m_managedTreeObjects->Remove(guid);
					m_managedTreeObjects[guid] = obj;
				}
				m_canDelete = true;
			}			
		}
		return retval;
	}

	cli::array<ROMNode^>^ ROMNode::GetAllFriends()
	{
		cli::array<ROMNode^>^ retval = gcnew cli::array<ROMNode^>(0);
		if (m_ROMNode)
		{
			vector<ROM::ROMNode*> vFriends = m_ROMNode->GetAllFriends();
			retval = _getArrayFromVectorROM(vFriends);
		}
		return retval;
	}

	bool ROMNode::AddFriend(ROMNode^ friendObj)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			retval = m_ROMNode->AddFriend(friendObj->m_ROMNode);
		}
		return retval;
	}

	bool ROMNode::RemoveFriend(ROMNode^ friendObj)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			retval = m_ROMNode->RemoveFriend(friendObj->m_ROMNode);
		}
		return retval;
	}

	bool ROMNode::RemoveAllFriends()
	{
		bool retval = false;
		if (m_ROMNode)
		{
			retval = m_ROMNode->RemoveAllFriends();
		}
		return retval;
	}

	bool ROMNode::DestroyROMObject()
	{
		if (m_ROMNode)
		{			
			if (m_canDelete)					
				delete m_ROMNode;			
				
			m_managedTreeObjects->Clear();
			m_ROMNode = nullptr;
			return true;
		}
		
		return false;
	}

	ROMNode^ ROMNode::Clone()
	{
		ROMNode^ retval = nullptr;
		if (m_ROMNode)
		{
			ROM::ROMNode* node = m_ROMNode->Clone();
			retval = ROMObjectFactory(gcnew String(ROMUTIL::Widen(node->GetROMObjectID()).c_str()));
			
			if (retval->m_ROMNode != nullptr)
				delete retval->m_ROMNode;
			retval->m_ROMNode = node;
			retval->m_managedTreeObjects = gcnew Dictionary<String^, ROMNode^>();
			retval->m_managedTreeObjects[gcnew String(node->GetROMGUID().c_str())] = retval;

			for each (ROM::ROMNode* obj in node->GetAllChildren(true))
			{
				String^ guid = gcnew String(obj->GetROMGUID().c_str());
				ROMNode^ managedNode = ROMObjectFactory(gcnew String(ROMUTIL::Widen(obj->GetROMObjectID()).c_str()));
				if (managedNode->m_ROMNode != nullptr)
					delete managedNode->m_ROMNode;
				managedNode->m_ROMNode = obj;

				retval->m_managedTreeObjects[guid] = managedNode;
				managedNode->m_managedTreeObjects = retval->m_managedTreeObjects;
				managedNode->m_canDelete = false;
			}

			m_canDelete = true;
		}
		return retval;
	}

	//attribute functions
	String^ ROMNode::GetAttribute(String^ id, String^ name, bool immediate)
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			string sName = MarshalString(name);
			string res = m_ROMNode->GetAttribute(sID, sName, immediate);
			retval = gcnew String(ROMUTIL::Widen(res).c_str());
		}
		return retval;
	}

	bool ROMNode::GetAttributeExists(String^ id, String^ name)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			string sName = MarshalString(name);
			retval = m_ROMNode->GetAttributeExists(sID, sName);
		}
		return retval;
	}

	bool ROMNode::SetAttribute(String^ id, String^ name, String^ value)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			string sName = MarshalString(name);
			string sValue = MarshalString(value);
			retval = m_ROMNode->SetAttribute(sID, sName, sValue);
		}
		return retval;
	}

	bool ROMNode::RemoveAttribute(String^ id, String^ name)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			string sName = MarshalString(name);
			retval = m_ROMNode->RemoveAttribute(sID, sName);
		}
		return retval;
	}

	bool ROMNode::SetROMObjectValue(String^ name, String^ value)
	{
		bool retval = false;
		if (m_ROMNode)
		{			
			string sName = MarshalString(name);
			string sValue = MarshalString(value);
			retval = m_ROMNode->SetROMObjectValue(sName, sValue);
		}
		return retval;
	}

	String^ ROMNode::GetROMObjectValue(String^ name)
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string sName = MarshalString(name);
			string res = m_ROMNode->GetROMObjectValue(sName);
			retval = gcnew String(ROMUTIL::Widen(res).c_str());
		}
		return retval;
	}

	bool ROMNode::RemoveROMObjectValue(String^ name)
	{
		bool retval = false;
		if (m_ROMNode)
		{
			string sName = MarshalString(name);
			retval = m_ROMNode->RemoveROMObjectValue(sName);
		}
		return retval;
	}

	String^ ROMNode::GetROMObjectID()
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string res = m_ROMNode->GetROMObjectID();
			retval = gcnew String(ROMUTIL::Widen(res).c_str());
		}
		return retval;
	}

	void ROMNode::SetROMObjectID(String^ id)
	{
		if (m_ROMNode)
		{
			string sID = MarshalString(id);
			m_ROMNode->SetROMObjectID(sID);
		}
	}

	String^ ROMNode::GetROMGUID()
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string res = m_ROMNode->GetROMGUID();
			retval = gcnew String(res.c_str());
		}
		return retval;
	}

	Dictionary<String^, Dictionary<String^, String^>^>^	ROMNode::GetAllAttributes()
	{
		Dictionary<String^, Dictionary<String^, String^>^>^ retval = gcnew Dictionary<String^, Dictionary<String^, String^>^>();
		if (m_ROMNode)
		{
			auto attrs = m_ROMNode->GetAllAttributes();
			for (auto it = attrs.begin(); it != attrs.end(); it++)
			{
				Dictionary<String^, String^>^ dictAttrs = gcnew Dictionary<String^, String^>();
				String^ key = gcnew String(ROMUTIL::Widen(it->first).c_str());
				for (auto itValues = it->second.begin(); itValues != it->second.end(); itValues++)
				{
					dictAttrs->Add(gcnew String(ROMUTIL::Widen(itValues->first).c_str()), gcnew String(ROMUTIL::Widen(itValues->second).c_str()));
				}
				retval->Add(key, dictAttrs);
			}
		}
		return retval;
	}

	//rules		
	EDSNET::EDSEngine^ ROMNode::_getManagedRules()
	{
		ROM::ROMNode* owner = nullptr;
		EDS::IKnowledgeBase* rules = m_ROMNode->GetKnowledgeBase(owner);
		if (rules != nullptr && owner != nullptr)
		{
			return m_managedTreeObjects[gcnew String(owner->GetROMGUID().c_str())]->m_KnowledgeBase;
		}
		else
		{
			return nullptr;
		}
	}

	cli::array<String^>^ ROMNode::EvaluateTable(String^ evalTable, String^ output, bool bGetAll, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_evaluateTable(m_ROMNode, sTable, sOutput, bGetAll, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetArrayFromVectorStrings(res);
		}
		return retval;
	}

	cli::array<String^>^ ROMNode::EvaluateTable(String^ evalTable, String^ output, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_evaluateTable(m_ROMNode, sTable, sOutput, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetArrayFromVectorStrings(res);
		}
		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ ROMNode::EvaluateTable(String^ evalTable, bool bGetAll, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_evaluateTable(m_ROMNode, sTable, bGetAll, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetDictionaryFromMapStrings(res);
		}		
		return retval;
	}	

	Dictionary<String^, cli::array<String^>^>^ ROMNode::EvaluateTable(String^ evalTable, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_evaluateTable(m_ROMNode, sTable, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetDictionaryFromMapStrings(res);
		}		
		return retval;
	}

	cli::array<String^>^ ROMNode::EvaluateTableWithParam(String^ evalTable, String^ output, bool bGetAll, String^ param, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			string par = MarshalString(param);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_evaluateTableWithParam(m_ROMNode, sTable, sOutput, bGetAll, par, ctx);
			
			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetArrayFromVectorStrings(res);
		}
		return retval;
	}

	cli::array<String^>^ ROMNode::EvaluateTableWithParam(String^ evalTable, String^ output, String^ param, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			string par = MarshalString(param);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_evaluateTableWithParam(m_ROMNode, sTable, sOutput, par, ctx);
			
			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetArrayFromVectorStrings(res);
		}
		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ ROMNode::EvaluateTableWithParam(String^ evalTable, bool bGetAll, String^ param, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string par = MarshalString(param);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_evaluateTableWithParam(m_ROMNode, sTable, bGetAll, par, ctx);
			
			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();

			retval = GetDictionaryFromMapStrings(res);
		}		
		return retval;
	}	

	Dictionary<String^, cli::array<String^>^>^ ROMNode::EvaluateTableWithParam(String^ evalTable, String^ param, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string par = MarshalString(param);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_evaluateTableWithParam(m_ROMNode, sTable, par, ctx);
			
			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free(); 
			
			retval = GetDictionaryFromMapStrings(res);
		}		
		return retval;
	}

	String^ ROMNode::GetFirstTableResult(String^ evalTable, String^ output, Object^ context)
	{
		String^ retval = gcnew String("");

		cli::array<String^>^ resAll = EvaluateTable(evalTable, output, false, context);
		
		if (resAll != nullptr && resAll->Length > 0)
			retval = resAll[0];
		return retval;
	}

	cli::array<String^>^ ROMNode::ReverseEvaluateTable(String^ evalTable, String^ output, bool bGetAll, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_reverseEvaluateTable(m_ROMNode, sTable, sOutput, bGetAll, ctx);
			retval = GetArrayFromVectorStrings(res);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
		return retval;
	}

	cli::array<String^>^ ROMNode::ReverseEvaluateTable(String^ evalTable, String^ output, Object^ context)
	{
		cli::array<String^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			string sOutput = MarshalString(output);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			vector<string> res = ROM::ROMNode::DispatchHelper::_reverseEvaluateTable(m_ROMNode, sTable, sOutput, ctx);
			retval = GetArrayFromVectorStrings(res);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ ROMNode::ReverseEvaluateTable(String^ evalTable, bool bGetAll, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_reverseEvaluateTable(m_ROMNode, sTable, bGetAll, ctx);
			retval = GetDictionaryFromMapStrings(res);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ ROMNode::ReverseEvaluateTable(String^ evalTable, Object^ context)
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_ROMNode)
		{
			string sTable = MarshalString(evalTable);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(context)).ToPointer();

			map<string, vector<string> > res = ROM::ROMNode::DispatchHelper::_reverseEvaluateTable(m_ROMNode, sTable, ctx);
			retval = GetDictionaryFromMapStrings(res);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
		return retval;
	}

	//IO
	String^ ROMNode::SaveXML(bool indented)
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string res = m_ROMNode->SaveXML(indented);
			retval = gcnew String(ROMUTIL::Widen(res).c_str());
		}
		return retval;
	}

	ROMNode^ ROMNode::LoadXML(String^ xmlFile, ROMObjectFactoryDelegate^ factory)
	{
		if (factory == nullptr)
		{
			factory = gcnew ROMObjectFactoryDelegate(&ROMNode::_managedFactory);
		}
		
		string sXMLFile = MarshalString(xmlFile);
		ROM::ROMNode* node = ROM::ROMNode::LoadXML(sXMLFile, nullptr);

		return _loadNode(node, factory);
	}

	ROMNode^ ROMNode::LoadXMLFromString(String^ xmlStr, ROMObjectFactoryDelegate^ factory)
	{
		if (factory == nullptr)
		{
			factory = gcnew ROMObjectFactoryDelegate(&ROMNode::_managedFactory);
		}

		string sXML = MarshalString(xmlStr);
		ROM::ROMNode* node = ROM::ROMNode::LoadXMLFromString(sXML, nullptr);

		return _loadNode(node, factory);
	}

	ROMNode^ ROMNode::_loadNode(ROM::ROMNode* node, ROMObjectFactoryDelegate^ factory)
	{
		ROMNode^ retval = factory(gcnew String(node->GetROMObjectID().c_str()));

		if (retval->m_ROMNode != nullptr)
			delete retval->m_ROMNode;
		retval->m_ROMNode = node;
		retval->m_managedTreeObjects = gcnew Dictionary<String^, ROMNode^>();
		retval->m_managedTreeObjects[gcnew String(node->GetROMGUID().c_str())] = retval;

		for each (ROM::ROMNode* obj in node->GetAllChildren(true))
		{
			String^ guid = gcnew String(obj->GetROMGUID().c_str());
			ROMNode^ managedNode = factory(gcnew String(obj->GetROMObjectID().c_str()));
			if (managedNode->m_ROMNode != nullptr)
				delete managedNode->m_ROMNode;
			managedNode->m_ROMNode = obj;

			retval->m_managedTreeObjects[guid] = managedNode;
			managedNode->m_managedTreeObjects = retval->m_managedTreeObjects;

			managedNode->m_canDelete = false;
		}

		return retval;
	}

	//XPATH
	String^ ROMNode::EvaluateXPATH(String^ xpath, String^ guid)
	{
		String^ retval = nullptr;
		if (m_ROMNode)
		{
			string sXPATH = MarshalString(xpath);
			string sGuid = MarshalString(guid);
			string res = m_ROMNode->EvaluateXPATH(sXPATH, sGuid);
			retval = gcnew String(ROMUTIL::Widen(res).c_str());
		}
		return retval;
	}


	cli::array<ROMNode^>^ ROMNode::_getArrayFromVectorROM(vector<ROM::ROMNode*> vect)
	{
		cli::array<ROMNode^>^ arr = gcnew cli::array<ROMNode^>(vect.size());
		for (size_t i = 0; i < vect.size(); i++)
		{
			String^ guid = gcnew String(vect[i]->GetROMGUID().c_str());
			arr[i] = m_managedTreeObjects[guid];
		}
		return arr;
	}

	//dictionary
	void ROMDictionary::LoadDictionary(String^ dictionaryTable)
	{
		if (m_ROMDictionary)
		{
			string sDict = MarshalString(dictionaryTable);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::IDictionaryInterface::DispatchHelper::_loadDictionary(m_ROMDictionary, sDict, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	ROMDictionaryAttribute^ ROMDictionary::GetDictionaryAttr(String^ dictAttrName)
	{
		ROMDictionaryAttribute^ retval = nullptr;
		if (m_ROMDictionary)
		{
			string sName = MarshalString(dictAttrName);
			ROM::IROMDictionaryAttribute* attr = m_ROMDictionary->GetDictionaryAttr(sName);
			if (attr)
			{
				retval = gcnew ROMDictionaryAttribute((IntPtr)attr);
			}
		}
		return retval;
	}

	Dictionary<String^, ROMDictionaryAttribute^>^ ROMDictionary::GetAllDictionaryAttrs()
	{
		Dictionary<String^, ROMDictionaryAttribute^>^ retval = gcnew Dictionary<String^, ROMDictionaryAttribute^>();
		if (m_ROMDictionary)
		{
			auto allAttrs = m_ROMDictionary->GetAllDictionaryAttrs();
			for (auto it = allAttrs->begin(); it != allAttrs->end(); it++)
			{
				String^ key = gcnew String(ROMUTIL::Widen(it->first).c_str());
				ROMDictionaryAttribute^ value = gcnew ROMDictionaryAttribute((IntPtr)(it->second));
				retval->Add(key, value);
			}
		}
		return retval;
	}
	
	void LinearEngine::InitializeEngine()
	{
		if (m_LinearEngine)
		{
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::LinearEngine::DispatchHelper::_initializeEngine(m_LinearEngine, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	void LinearEngine::ResetEngine()
	{
		if (m_LinearEngine)
		{
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::LinearEngine::DispatchHelper::_resetEngine(m_LinearEngine, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	void LinearEngine::EvaluateForAttribute(String^ dictAttrName, cli::array<String^>^ newValues, bool bEvalDependents)
	{
		if (m_LinearEngine)
		{
			string name = MarshalString(dictAttrName);
			vector<string> vals = GetVectorFromArrayStrings(newValues);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::LinearEngine::DispatchHelper::_evaluateForAttribute(m_LinearEngine, name, vals, bEvalDependents, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	void LinearEngine::EvaluateForAttribute(String^ dictAttrName, String^ newValue, bool bEvalDependents)
	{
		if (m_LinearEngine)
		{
			string name = MarshalString(dictAttrName);
			string val = MarshalString(newValue);
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::LinearEngine::DispatchHelper::_evaluateForAttribute(m_LinearEngine, name, val, bEvalDependents, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	void LinearEngine::EvaluateAll()
	{
		if (m_LinearEngine)
		{
			void* ctx = GCHandle::ToIntPtr(GCHandle::Alloc(m_ROMContext)).ToPointer();

			ROM::LinearEngine::DispatchHelper::_evaluateAll(m_LinearEngine, ctx);

			if (ctx != nullptr)
				GCHandle::FromIntPtr(IntPtr(ctx)).Free();
		}
	}

	cli::array<ROMDictionaryAttribute^>^ LinearEngine::GetEvalList()
	{
		cli::array<ROMDictionaryAttribute^>^ retval = nullptr;
		if (m_LinearEngine)
		{
			vector<ROM::IROMDictionaryAttribute*> res = m_LinearEngine->GetEvalList();
			retval = gcnew cli::array<ROMDictionaryAttribute^>(res.size());
			size_t i = 0;
			for (vector<ROM::IROMDictionaryAttribute*>::iterator it = res.begin(); it != res.end(); it++)
			{
				retval[i] = gcnew ROMDictionaryAttribute((IntPtr)(*it));
				i++;
			}			
		}
		return retval;
	}

	Dictionary<String^, cli::array<String^>^>^ LinearEngine::GetTriggers()
	{
		Dictionary<String^, cli::array<String^>^>^ retval = nullptr;
		if (m_LinearEngine)
		{
			map<string, vector<string> > res = m_LinearEngine->GetTriggers();
			retval = gcnew Dictionary<String^, cli::array<String^>^>();
			for (map<string, vector<string> >::iterator it = res.begin(); it != res.end(); it++)
			{
				cli::array<String^>^ attrs = gcnew cli::array<String^>(it->second.size());
				size_t i = 0;
				for (vector<string>::iterator itAttr = it->second.begin(); itAttr != it->second.end(); itAttr++)
				{
					attrs[i] = gcnew String(ROMUTIL::Widen((*itAttr)).c_str());
					i++;
				}
				retval->Add(gcnew String(ROMUTIL::Widen(it->first).c_str()), attrs);
			}			
		}
		return retval;
	}
}
