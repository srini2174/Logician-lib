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
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include "ROMInterfaces.h"
#include <EDSEngine/IKnowledgeBase.h>
#include "utilities.h"

using namespace std;

namespace ROM
{
	class ROMNode;
	typedef function<ROMNode*(const string&)> ObjectFactory;

	class ROMNode
	{
	friend class ROMDictionary;
	friend class LinearEngine;
	public:
		virtual ~ROMNode(void);
		ROMNode(){_init();}
		ROMNode(const string id, ObjectFactory factory = nullptr) { CreateROMNode(id, factory); }
		ROMNode(const ROMNode&) = delete;             // Prevent copy-construction
		ROMNode& operator=(const ROMNode&) = delete;  // Prevent assignment
		void				CreateROMNode(const string id, ObjectFactory factory = nullptr);
		ObjectFactory		ROMObjectFactory;

		//relational functions
		ROMNode*			GetRoot();
		ROMNode*			GetParent() {return m_parent;}
		vector<ROMNode*>	GetAllChildren(bool recurs);
		vector<ROMNode*>	FindObjects(const string& xpath);
		ROMNode*			FindFirstObject(const string& xpath);
		vector<ROMNode*>	FindAllObjectsByID(const string& id, bool recurs);
		ROMNode*			FindObjectByGUID(const string& guid);
		bool				AddChildROMObject(ROMNode *child);
		bool				RemoveChildROMObject(ROMNode *child);
		bool				RemoveFromParent();
		vector<ROMNode*>	GetAllFriends();
		bool				AddFriend(ROMNode *friendObj);
		bool				RemoveFriend(ROMNode *friendObj);
		bool				RemoveAllFriends();
		bool				DestroyROMObject();
		ROMNode*			Clone();

		//attribute functions
		virtual string		GetAttribute(const string& id, const string& name, bool immediate = false) { return _getAttributeInternal(id, name, immediate); }
		virtual string		GetAttribute(const string& id, bool immediate = false) { return GetAttribute(id, "value", immediate); }
		virtual bool		GetAttributeExists(const string& id, const string& name = "value");
		virtual bool		SetAttribute(const string& id, const string& name, const string& value);
		virtual bool		SetAttribute(const string& id, const string& value) { return SetAttribute(id, "value", value); }
		virtual bool		SetAttributeValue(const string& id, const string& value) { return SetAttribute(id, value); }
		virtual bool		RemoveAttribute(const string& id, const string& name = "value");
		virtual bool		SetROMObjectValue(const string& name, const string& value);
		virtual string		GetROMObjectValue(const string& name);
		virtual bool		RemoveROMObjectValue(const string& name);
		string				GetROMObjectID() {return m_id;}
		void				SetROMObjectID(const string& id) { m_id = id; }
		string				GetROMGUID() {return m_guid;}
		unordered_map<string, std::unordered_map<string, string>> GetAllAttributes() { return m_attrs; }

		//rules		
		vector<string>		EvaluateTable(const string& evalTable, const string& output, bool bGetAll) { return _evaluateTable(evalTable, output, bGetAll, this); }
		vector<string>		EvaluateTable(const string& evalTable, const string& output) { return _evaluateTable(evalTable, output, this); }
		map<string, vector<string> > EvaluateTable(const string& evalTable, bool bGetAll) { return _evaluateTable(evalTable, bGetAll, this); }
		map<string, vector<string> > EvaluateTable(const string& evalTable) { return _evaluateTable(evalTable, this); }
		vector<string>		EvaluateTableWithParam(const string& evalTable, const string& output, bool bGetAll, string& param) { return _evaluateTableWithParam(evalTable, output, bGetAll, param, this); }
		vector<string>		EvaluateTableWithParam(const string& evalTable, const string& output, string& param) { return _evaluateTableWithParam(evalTable, output, param, this); }
		map<string, vector<string> > EvaluateTableWithParam(const string& evalTable, bool bGetAll, string& param) { return _evaluateTableWithParam(evalTable, bGetAll, param, this); }
		map<string, vector<string> > EvaluateTableWithParam(const string& evalTable, string& param) { return _evaluateTableWithParam(evalTable, param, this); }
		string				GetFirstTableResult(const string& tableName, const string& output) { return _getFirstTableResult(tableName, output, this); }
		vector<string>		ReverseEvaluateTable(const string& evalTable, const string& inputAttr, bool bGetAll) { return _reverseEvaluateTable(evalTable, inputAttr, bGetAll, this); }
		vector<string>		ReverseEvaluateTable(const string& evalTable, const string& inputAttr) { return _reverseEvaluateTable(evalTable, inputAttr, this); }
		map<string, vector<string> > ReverseEvaluateTable(const string& evalTable, bool bGetAll) { return _reverseEvaluateTable(evalTable, bGetAll, this); }
		map<string, vector<string> > ReverseEvaluateTable(const string& evalTable) { return _reverseEvaluateTable(evalTable, this); }
		EDS::IKnowledgeBase* GetKnowledgeBase() { ROMNode* owner = nullptr; return _getKnowledge(owner); }
		EDS::IKnowledgeBase* GetKnowledgeBase(ROMNode*& owner) { return _getKnowledge(owner); }
		void				SetKnowledgeBase(EDS::IKnowledgeBase* rules) { m_KnowledgeBase = rules; }


		//IO
		string				SaveXML(bool prettyprint);
		static ROMNode*		LoadXML(const string& xmlFile, ObjectFactory factory) { return ROMNode::_loadXML(xmlFile, true, factory); }
		static ROMNode*		LoadXMLFromString(const string& xmlStr, ObjectFactory factory) { return ROMNode::_loadXML(xmlStr, false, factory); }

		//XPATH
		string				EvaluateXPATH(const string& xpath, const string& guid);
		string				EvaluateXPATH(const string& xpath) { return EvaluateXPATH(xpath, m_guid); }

#ifdef WIN32
		struct DispatchHelper;
	protected:		
		friend DispatchHelper;
#endif
				
	private:
		//these internal methods are called by .NET to assist with passing of managed objects
		vector<string>		_evaluateTable(const string& evalTable, const string& output, bool bGetAll, void* context);
		vector<string>		_evaluateTable(const string& evalTable, const string& output, void* context);
		map<string, vector<string> > _evaluateTable(const string& evalTable, bool bGetAll, void* context);
		map<string, vector<string> > _evaluateTable(const string& evalTable, void* context);
		vector<string>		_evaluateTableWithParam(const string& evalTable, const string& output, bool bGetAll, string& param, void* context);
		vector<string>		_evaluateTableWithParam(const string& evalTable, const string& output, string& param, void* context);
		map<string, vector<string> > _evaluateTableWithParam(const string& evalTable, bool bGetAll, string& param, void* context);
		map<string, vector<string> > _evaluateTableWithParam(const string& evalTable, string& param, void* context);
		string				_getFirstTableResult(const string& tableName, const string& output, void* context);
		vector<string>		_reverseEvaluateTable(const string& evalTable, const string& inputAttr, bool bGetAll, void* context);
		vector<string>		_reverseEvaluateTable(const string& evalTable, const string& inputAttr, void* context);
		map<string, vector<string> > _reverseEvaluateTable(const string& evalTable, bool bGetAll, void* context);
		map<string, vector<string> > _reverseEvaluateTable(const string& evalTable, void* context);

		string					_getAttributeInternal(const string& id, const string& name, bool immediate);
		string					_getAttribute(const string& id, const string& name, bool immediate);
		string					_getSpecialAttribute(const string& input, bool* bFound);
		vector<string>			_getPossibleValues(const string& evalTable, const string& outputName);
		ROMNode*				_findObjectGUID(const string& guid);
		void					_findAllChildObjects(vector<ROMNode*>* res);
		void					_findObjects(const string& id, bool recurs, vector<ROMNode*>* res);
		bool					_anyHasChanged();
		void					_setAllUnchanged();
		string					_generateXML(bool bRegen, bool prettyprint);
		string					_generateAttrNode(const string& id);
		static ROMNode*			_buildObject(Node objectNode, ObjectFactory factory);
		void					_createXMLDoc(bool bForceLoad, bool prettyprint);
		static string			_convertXMLDocToString(bool prettyprint, Document xmlDoc);
		EDS::IKnowledgeBase*	_getKnowledge(ROMNode*& owner);
		EDS::IKnowledgeBase*	_getKnowledge() { ROMNode* owner = nullptr; return _getKnowledge(owner); }
		ROMNode*				_getActiveContext();
		void					_init(ObjectFactory factory = nullptr);
		static ROMNode*			_loadXML(const string& xmlStr, bool isFile, ObjectFactory factory);
#ifdef USE_MSXML
		static Document			_createMSXMLDoc();
#endif

		string m_id;
		string m_guid;
		bool m_bChanged;
		Document m_xmlDoc;
		string m_lastContents;
		string m_lastAttrContents;
		vector<ROMNode*> m_children;
		vector<ROMNode*> m_friends;
		ROMNode* m_parent;
		unordered_map<string, std::unordered_map<string, string> > m_attrs;
		unordered_map<string, string> m_nodeValues;

		EDS::IKnowledgeBase* m_KnowledgeBase;
	};
}
