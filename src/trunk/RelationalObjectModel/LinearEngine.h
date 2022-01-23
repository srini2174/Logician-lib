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

#include "ROMDictionary.h"

#define MAX_RECURSION 1000

using namespace std;

namespace ROM
{
	enum INVALIDATEMODE_E
	{
		NORMALINVALIDATE,
		FLAGINVALIDATE
	};

	class LinearEngine : public ROMDictionary
	{
	friend struct DispatchHelper;
	public:
		LinearEngine(){ InvalidateMode = NORMALINVALIDATE; m_initialized = false; }
		LinearEngine(ROMNode* context, const string& dictionaryTable) : ROMDictionary(context) { CreateLinearEngine(dictionaryTable); }
		void CreateLinearEngine(const string& dictionaryTable);
		void InitializeEngine() { _initializeEngine(m_ROMContext); }
		void ResetEngine() { _resetEngine(m_ROMContext); }
		virtual ~LinearEngine(){}
		void EvaluateForAttribute(const string& dictAttrName, vector<string>& newValues, bool bEvalDependents = true) { _evaluateForAttribute(dictAttrName, newValues, bEvalDependents, m_ROMContext); }
		void EvaluateForAttribute(const string& dictAttrName, const string& newValue, bool bEvalDependents = true) { _evaluateForAttribute(dictAttrName, newValue, bEvalDependents, m_ROMContext); }
		void EvaluateAll() { _evaluateAll(m_ROMContext); }
		vector<IROMDictionaryAttribute*> GetEvalList() { return m_vEvalList; }
		map<string, vector<string> > GetTriggers() {return m_mapTriggers;}
		bool DictionaryIsValid();

		//behavioral properties
		int InvalidateMode;

#ifdef WIN32
		struct DispatchHelper;
	protected:
		friend DispatchHelper;
#endif
	
	private:
		//these internal methods are called by .NET to assist with passing of managed objects
		void _evaluateForAttribute(const string& dictAttrName, vector<string>& newValues, bool bEvalDependents, void* context);
		void _evaluateForAttribute(const string& dictAttrName, const string& newValue, bool bEvalDependents, void* context);
		void _evaluateAll(void* context);
		void _initializeEngine(void* context);
		void _resetEngine(void* context);
	
		void _orderDictionary();
		void _evalSingleSelect(const string& dictAttrName, const string& newValue, void* context);
		void _evalMultiSelect(const string& dictAttrName, const vector<string>& newValues, void* context);
		void _evalBoolean(const string& dictAttrName, const string& newValue, void* context);
		void _evalEdit(const string& dictAttrName, const string& newValue, void* context);
		void _evaluateDependencies(const string& dictAttrName, void* context);
		void _flagAttrInvalid(const string& dictAttrName);
		bool _isTouchedByUser(const string& dictAttrName);
		void _setTouchedByUser(const string& dictAttrName);
		void _removeTouchedByUser(const string& dictAttrName);
		void _loadTrackingAttrs();
		vector<string> _parseOutPrefixes(int AttributeType, const vector<string>& values, vector<string>& valuesWithoutPrefixes); //remove the special character flags from the values
		vector<string> _getSelectedValues(IROMDictionaryAttribute* attr);
		void _resetValueChanged();		

		vector<IROMDictionaryAttribute*> m_vEvalList;
		map<string, vector<string> > m_mapTriggers;
		int m_CurrentRecursion;
		vector<IROMDictionaryAttribute*> m_vEvalListRecursChecker;
		bool m_EvalInternal;
		bool m_initialized;

		string INVISPREFIX;
		string DEFAULTPREFIX;
		string DISABLEPREFIX;
		string TBUATTR;
	};
}
