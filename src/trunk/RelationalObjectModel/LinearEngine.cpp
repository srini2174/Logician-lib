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
#include "LinearEngine.h"
#include "utilities.h"

namespace ROM
{
	void LinearEngine::CreateLinearEngine(const string& dictionaryTable)
	{
		InvalidateMode = NORMALINVALIDATE;
		TBUATTR = "TBU_";
		m_tableName = dictionaryTable;
		m_initialized = false;
	}

	void LinearEngine::_initializeEngine(void* context)
	{
		EDS::IKnowledgeBase *knowledge = m_ROMContext->_getKnowledge();
		if (knowledge)
		{
			INVISPREFIX = "^";
			DEFAULTPREFIX = "@";
			DISABLEPREFIX = "#";

			m_vEvalList.clear();
			m_mapTriggers.clear();
			_loadDictionary(m_tableName, context);
			_loadTrackingAttrs();

			//open each attr in dict, load its dependency info to create m_vEvalList, m_mapTriggers
			//build an initial list that matches the dictionary order
			for (auto it = m_dict.begin(); it != m_dict.end(); it++)
			{
				m_vEvalList.push_back(it->second);
				//triggers
				vector<string> deps = knowledge->GetInputDependencies(it->second->GetRuleTable());
				for (vector<string>::iterator itDeps = deps.begin(); itDeps != deps.end(); itDeps++)
				{
					if (m_mapTriggers.find(*itDeps) != m_mapTriggers.end())
					{
						if (find(m_mapTriggers[*itDeps].begin(), m_mapTriggers[*itDeps].end(), it->second->GetName()) == m_mapTriggers[*itDeps].end())
							m_mapTriggers[*itDeps].push_back(it->second->GetName());
					}
					else if (m_dict.find(*itDeps) != m_dict.end())
					{
						vector<string> newVect;
						newVect.push_back(it->second->GetName());
						m_mapTriggers[*itDeps] = newVect;
					}
				}
			}

			//based on the triggers, re-order the dictionary
			m_CurrentRecursion = 0;
			m_EvalInternal = false;
			_orderDictionary();
			m_vEvalListRecursChecker.clear();
			m_initialized = true;
		}
	}

	void LinearEngine::_resetEngine(void* context)
	{
		_initializeEngine(context);
		for (auto it = m_dict.begin(); it != m_dict.end(); it++)
			m_ROMContext->SetAttribute(it->first, "");
		_loadDictionary(m_tableName, context);
		_evaluateAll(context);
	}

	void LinearEngine::_loadTrackingAttrs()
	{
		auto allAttrs = m_ROMContext->GetAllAttributes();
		for (auto it = allAttrs.begin(); it != allAttrs.end(); it++)
		{
			if (ROMUTIL::StringContains(it->first, TBUATTR))
			{
				string dictAttrName = ROMUTIL::FindAndReplace(it->first, TBUATTR, "");
				if (m_dict.find(dictAttrName) != m_dict.end())
					m_dict[dictAttrName]->SetChangedByUser(true);
			}
		}
	}

	void LinearEngine::_orderDictionary()
	{
		m_CurrentRecursion++;
		//make a copy to see if the order has changed during recursion
		vector<IROMDictionaryAttribute*> evalOrderCopy = m_vEvalList;

		//check for circular logic, outputs that are also inputs must go above the attrs that are dependent on them
		for (auto it = m_mapTriggers.begin(); it != m_mapTriggers.end(); it++)
		{
			for (auto itDeps = it->second.begin(); itDeps != it->second.end(); itDeps++)
			{
				size_t lowestIndex = 0;
				bool bFoundInputAttr = false;
				for (lowestIndex = 0; lowestIndex < m_vEvalList.size(); lowestIndex++)
				{
					if (m_vEvalList[lowestIndex]->GetName() == it->first)
					{
						bFoundInputAttr = true;
						break;
					}
				}
				size_t origIndex = lowestIndex;
				if (bFoundInputAttr)
				{
					for (vector<string>::iterator itDeps2 = it->second.begin(); itDeps2 != it->second.end(); itDeps2++)
					{
						if (itDeps2->length() > 0)
						{
							size_t currentIndex = 0;
							for (currentIndex = 0; currentIndex < m_vEvalList.size(); currentIndex++)
							{
								if (m_vEvalList[currentIndex]->GetName() == *itDeps2)
									break;
							}
							if (currentIndex < lowestIndex)
								lowestIndex = currentIndex;
						}
					}

					//make the current input index lower than lowest output
					if (origIndex != lowestIndex)
					{
						auto attr = m_vEvalList[origIndex];
						m_vEvalList.erase(m_vEvalList.begin() + origIndex);
						m_vEvalList.insert(m_vEvalList.begin() + lowestIndex, attr);
					}
				}
			}
		}

		for (size_t i = 0; i < evalOrderCopy.size(); i++)
		{
			//if the lists differ, do another sort, otherwise we should be done
			if (evalOrderCopy[i] != m_vEvalList[i] && m_CurrentRecursion < MAX_RECURSION)
			{
				//does it match a previous result (are we flipping between a couple values, circular logic)
				if (m_CurrentRecursion % 2 == 0 && m_vEvalListRecursChecker.size() > 0)
				{
					for (size_t j = 0; j < m_vEvalListRecursChecker.size(); j++)
					{
						if (m_vEvalList[j] != m_vEvalListRecursChecker[j])
						{
							_orderDictionary();
							break;
						}
					}
				}
				else
					_orderDictionary();

				break;
			}
		}

		if (m_CurrentRecursion % 2 == 0)
		{
			m_vEvalListRecursChecker = m_vEvalList;
		}
	}

	void LinearEngine::_evaluateForAttribute(const string& dictAttrName, const string& newValue, bool bEvalDependents, void* context)
	{
		vector<string> newValues;
		newValues.push_back(newValue);
		_evaluateForAttribute(dictAttrName, newValues, bEvalDependents, context);
	}

	void LinearEngine::_evaluateForAttribute(const string& dictAttrName, vector<string>& newValues, bool bEvalDependents, void* context)
	{
		if (!m_EvalInternal)
		{
			_resetValueChanged();

			//resets other atts when setting this one
			m_ROMContext->SetAttribute("currentattr", dictAttrName);
			vector<string> attrsToReset = m_ROMContext->_evaluateTable((string)"reset", (string)"attr", context);
			for (auto it = attrsToReset.begin(); it != attrsToReset.end(); it++)
			{
				m_ROMContext->SetAttribute(*it, "");
				_removeTouchedByUser(*it);
				if (m_dict[*it]->GetAvailableValues().size() > 0)
					m_dict[*it]->SetValid(false);
			}

			//resets an attr if it has not been touched by the user
			attrsToReset = m_ROMContext->_evaluateTable((string)"reset_INCBU", (string)"attr", context);
			for (auto it = attrsToReset.begin(); it != attrsToReset.end(); it++)
			{
				if (!_isTouchedByUser(*it))
				{
					m_ROMContext->SetAttribute(*it, "");
					_removeTouchedByUser(*it);
					if (m_dict[*it]->GetAvailableValues().size() > 0)
						m_dict[*it]->SetValid(false);
				}
			}
		}


		auto itFind = m_dict.find(dictAttrName);
		if (itFind != m_dict.end())
		{
			m_dict[dictAttrName]->SetValueChanged(true);
			bool bUserChanged = !m_EvalInternal;
			if (bUserChanged)
				_setTouchedByUser(dictAttrName);

			switch (m_dict[dictAttrName]->GetAttributeType())
			{
			case SINGLESELECT:
				_evalSingleSelect(dictAttrName, newValues[0], context);
				break;

			case MULTISELECT:
				_evalMultiSelect(dictAttrName, newValues, context);
				break;

			case BOOLEANSELECT:
				_evalBoolean(dictAttrName, newValues[0], context);
				break;

			case EDIT:
				_evalEdit(dictAttrName, newValues[0], context);
				break;

			default: //STATIC
				_evalSingleSelect(dictAttrName, newValues[0], context);
				break;
			}

			if (bEvalDependents)
				_evaluateDependencies(dictAttrName, context);
		}
	}

	void LinearEngine::_resetValueChanged()
	{
		for (auto it = m_vEvalList.begin(); it != m_vEvalList.end(); it++)
		{
			(*it)->SetValueChanged(false);
		}
	}

	bool LinearEngine::_isTouchedByUser(const string& dictAttrName)
	{
		return m_ROMContext->GetAttributeExists(TBUATTR + dictAttrName);
	}

	void LinearEngine::_setTouchedByUser(const string& dictAttrName)
	{
		m_dict[dictAttrName]->SetChangedByUser(true);
		m_ROMContext->SetAttribute(TBUATTR + dictAttrName, "Y");
	}

	void LinearEngine::_removeTouchedByUser(const string& dictAttrName)
	{
		m_dict[dictAttrName]->SetChangedByUser(false);
		m_ROMContext->RemoveAttribute(TBUATTR + dictAttrName);
	}

	void LinearEngine::_evaluateAll(void* context)
	{
		if (!m_initialized)
			_initializeEngine(context);

		m_EvalInternal = true;
		_resetValueChanged();
		for (auto it = m_vEvalList.begin(); it != m_vEvalList.end(); it++)
		{
			vector<string> selectedValues = _getSelectedValues(*it);
			_evaluateForAttribute((*it)->GetName(), selectedValues, false, context);
		}
		m_EvalInternal = false;
	}

	bool LinearEngine::DictionaryIsValid()
	{
		bool retval = true;
		for (auto it = m_vEvalList.begin(); it != m_vEvalList.end(); it++)
		{
			if (!(*it)->GetValid())
			{
				retval = false;
				break;
			}
		}
		return retval;
	}

	void LinearEngine::_flagAttrInvalid(const string& dictAttrName)
	{
		m_dict[dictAttrName]->SetChangedByUser(false);
		m_dict[dictAttrName]->SetValid(false);
	}

	void LinearEngine::_evalBoolean(const string& dictAttrName, const string& value, void* context)
	{
		string newValue = value;
		vector<string> res = m_ROMContext->_evaluateTable(m_dict[dictAttrName]->GetRuleTable(), dictAttrName, false, context);
		vector<string> availableValues;

		vector<string> prefixes = _parseOutPrefixes(BOOLEANSELECT, res, availableValues);
		m_dict[dictAttrName]->SetAvailableValues(availableValues);

		if (prefixes.size() > 0 && prefixes[0].length() > 0 && prefixes[0][0] == L'^')
			m_dict[dictAttrName]->SetVisible(false);
		else
			m_dict[dictAttrName]->SetVisible(true);

		string currentValue = m_ROMContext->GetAttribute(dictAttrName, true);
		m_dict[dictAttrName]->SetValid(true);
		m_dict[dictAttrName]->SetEnabled(true);

		//set a default
		bool bOverrideDisabled = false;
		if (currentValue.length() == 0)
		{
			if (m_dict[dictAttrName]->GetDefaultValue().length() > 0)
				currentValue = m_dict[dictAttrName]->GetDefaultValue();
			else
			{
				currentValue = "N";
				if (availableValues.size() == 1 && availableValues[0].length() > 0 &&
					availableValues[0][0] == 'Y')
					bOverrideDisabled = true;
			}
		}

		if (availableValues.size() == 1) //you should only have one value
		{
			if (availableValues[0].length() == 0 || availableValues[0][0] == L'N')
			{
				m_ROMContext->SetAttribute(dictAttrName, "N");
				_removeTouchedByUser(dictAttrName);
				m_dict[dictAttrName]->SetEnabled(false);
			}
			else if (availableValues[0] == "YN") //allow Yes or No with a default of Y
			{
				if (!_isTouchedByUser(dictAttrName))
				{
					m_ROMContext->SetAttribute(dictAttrName, "Y");
				}
				else
				{
					m_ROMContext->SetAttribute(dictAttrName, newValue);
				}
			}
			else if (availableValues[0] == "YY") //force Yes, no other choice
			{
				m_ROMContext->SetAttribute(dictAttrName, "Y");
				_removeTouchedByUser(dictAttrName);
				m_dict[dictAttrName]->SetEnabled(false);
			}
			else if (newValue.length() == 1) //Y or N
			{
				m_ROMContext->SetAttribute(dictAttrName, newValue);
			}
			else if (currentValue.length() == 0 && newValue.length() == 0)
			{
				m_ROMContext->SetAttribute(dictAttrName, "N");
				_removeTouchedByUser(dictAttrName);
				m_dict[dictAttrName]->SetEnabled(false);
			}
			else
			{
				if (currentValue == "N" && !bOverrideDisabled)
					m_dict[dictAttrName]->SetEnabled(false);
				m_ROMContext->SetAttribute(dictAttrName, currentValue);
			}
		}
		else if (newValue.length() == 1) //Y or N
		{
			m_ROMContext->SetAttribute(dictAttrName, newValue);
		}
		else if (currentValue.length() == 0 && newValue.length() == 0)
		{
			m_ROMContext->SetAttribute(dictAttrName, "N");
			_removeTouchedByUser(dictAttrName);
			m_dict[dictAttrName]->SetEnabled(false);
		}
		else
		{
			if (currentValue == "N" && !bOverrideDisabled)
				m_dict[dictAttrName]->SetEnabled(false);
			m_ROMContext->SetAttribute(dictAttrName, currentValue);
		}

		m_dict[dictAttrName]->SetValue(m_ROMContext->GetAttribute(dictAttrName, true));
	}

	void LinearEngine::_evalEdit(const string& dictAttrName, const string& value, void* context)
	{
		string newValue = value;
		vector<string> res = m_ROMContext->_evaluateTable(m_dict[dictAttrName]->GetRuleTable(), dictAttrName, context);
		vector<string> availableValues;

		vector<string> prefixes = _parseOutPrefixes(EDIT, res, availableValues);

		m_dict[dictAttrName]->SetAvailableValues(availableValues);
		m_dict[dictAttrName]->SetEnabled(true);
		m_dict[dictAttrName]->SetValid(true);
		//if no rules table defined, allow any value
		if (m_dict[dictAttrName]->GetRuleTable().size() == 0)
		{
			m_ROMContext->SetAttribute(dictAttrName, newValue);
			m_dict[dictAttrName]->SetValue(newValue);
			return;
		}

		bool setTheValue = true;
		string currentValue = m_ROMContext->GetAttribute(dictAttrName, true);
		if (InvalidateMode != NORMALINVALIDATE)
		{			
			setTheValue = currentValue.length() == 0;
		}

		//set the dictionary default on load
		if (newValue.length() == 0)
		{
			if (m_dict[dictAttrName]->GetDefaultValue().length() > 0)
				newValue = m_dict[dictAttrName]->GetDefaultValue();
		}

		if (availableValues.size() == 1)
		{
			if (ROMUTIL::StringContains(prefixes[0], DISABLEPREFIX))
			{
				m_dict[dictAttrName]->SetEnabled(false);
				_removeTouchedByUser(dictAttrName);
				m_ROMContext->SetAttribute(dictAttrName, availableValues[0]);
				m_dict[dictAttrName]->SetValue(availableValues[0]);
				return;
			}
			else if (ROMUTIL::StringContains(prefixes[0], DEFAULTPREFIX) && 
				ROMUTIL::StringContains(availableValues[0], "["))
			{
				string defaultValue = availableValues[0].substr(0, availableValues[0].find("["));
				availableValues[0] = availableValues[0].substr(availableValues[0].find("["));				
				if (currentValue.length() == 0)
				{
					newValue = defaultValue;
					_removeTouchedByUser(dictAttrName);
				}
				m_dict[dictAttrName]->SetEnabled(true);
			}
			else
				m_dict[dictAttrName]->SetEnabled(true);

			//check table result for range
			if (availableValues[0][0] == L'[')
			{
				double dNewValue = 0, dMin = 0, dMax = 0;
				string val = availableValues[0];
				val = ROMUTIL::FindAndReplace(val, "[", "");
				val = ROMUTIL::FindAndReplace(val, "]", "");
				vector<string> vals = ROMUTIL::Split(val, ",");

				dNewValue = atof(newValue.c_str());
				dMin = atof(vals[0].c_str());
				dMax = atof(vals[1].c_str());

				if (dNewValue <= dMax && dNewValue >= dMin)
				{
					if (newValue.length() == 0 || !ROMUTIL::StringIsNumeric(newValue))
					{
						string wstrMin(vals[0].begin(), vals[0].end());
						m_ROMContext->SetAttribute(dictAttrName, wstrMin);
					}
					else
					{
						m_ROMContext->SetAttribute(dictAttrName, newValue);
					}
				}
				else if (dNewValue > dMax)
				{
					if (setTheValue)
					{
						string wstrMax(vals[1].begin(), vals[1].end());
						m_ROMContext->SetAttribute(dictAttrName, wstrMax);
					}
					else
					{
						_flagAttrInvalid(dictAttrName);
					}
				}
				else if (dNewValue < dMin)
				{
					if (setTheValue)
					{
						string wstrMin(vals[0].begin(), vals[0].end());
						m_ROMContext->SetAttribute(dictAttrName, wstrMin);
					}
					else
					{
						_flagAttrInvalid(dictAttrName);
					}
				}
			}
			else if (availableValues[0].length() == 1 && availableValues[0][0] == L'Y')
			{
				m_ROMContext->SetAttribute(dictAttrName, newValue);
			}
			else if (availableValues[0].length() == 1 && availableValues[0][0] == L'N')
			{
				m_ROMContext->SetAttribute(dictAttrName, "");
				_removeTouchedByUser(dictAttrName);
				m_dict[dictAttrName]->SetEnabled(false);
			}
			else if (availableValues[0].length() == 2 && availableValues[0] == "YY") //user must enter something
			{
				m_ROMContext->SetAttribute(dictAttrName, newValue);
				m_dict[dictAttrName]->SetValid(newValue.length() > 0);
			}
			else
			{
				m_ROMContext->SetAttribute(dictAttrName, availableValues[0]);
				_removeTouchedByUser(dictAttrName);
			}
		}
		else if (availableValues.size() == 0)
		{
			m_ROMContext->SetAttribute(dictAttrName, newValue);
			_removeTouchedByUser(dictAttrName);
			m_dict[dictAttrName]->SetEnabled(false);
		}
		else if (availableValues.size() == 1 && availableValues[0].length() > 0)
		{
			m_ROMContext->SetAttribute(dictAttrName, availableValues[0]);
			_removeTouchedByUser(dictAttrName);
		}
		else if (availableValues.size() > 0)
		{
			vector<string>::iterator itFind = find(availableValues.begin(), availableValues.end(), newValue);
			if (itFind != availableValues.end())
			{
				m_ROMContext->SetAttribute(dictAttrName, newValue);
			}
			else
			{
				m_ROMContext->SetAttribute(dictAttrName, "");
				_removeTouchedByUser(dictAttrName);
			}
		}

		if (prefixes.size() > 0 && ROMUTIL::StringContains(prefixes[0], INVISPREFIX))
			m_dict[dictAttrName]->SetVisible(false);
		else
			m_dict[dictAttrName]->SetVisible(true);

		m_dict[dictAttrName]->SetValue(m_ROMContext->GetAttribute(dictAttrName, true));
	}

	void LinearEngine::_evalMultiSelect(const string& dictAttrName, const vector<string>& values, void* context)
	{
		vector<string> newValues = values;
		//multi-select lists, checkbox lists
		vector<string> res = m_ROMContext->_evaluateTable(m_dict[dictAttrName]->GetRuleTable(), dictAttrName, context);
		vector<string> availableValues;
		m_dict[dictAttrName]->SetEnabled(true);
		m_dict[dictAttrName]->SetValid(true);

		vector<string> prefixes = _parseOutPrefixes(MULTISELECT, res, availableValues);
		m_dict[dictAttrName]->SetAvailableValues(availableValues);

		string currentValue = m_ROMContext->GetAttribute(dictAttrName, true);
		vector<string> currentValues = ROMUTIL::Split(currentValue, "|");
		vector<string> selectedValues;

		bool setTheValue = true;
		bool bFound = true;
		if (InvalidateMode != NORMALINVALIDATE)
		{
			setTheValue = currentValues.size() == 0;
			for (vector<string>::iterator it = currentValues.begin(); it != currentValues.end(); it++)
			{
				bFound = find(availableValues.begin(), availableValues.end(), *it) != availableValues.end();
				if (bFound)
					break;
			}
		}

		//set the dictionary default on load
		if (newValues.size() == 0)
		{
			if (m_dict[dictAttrName]->GetDefaultValue().length() > 0)
			{
				newValues.push_back(m_dict[dictAttrName]->GetDefaultValue());
				_removeTouchedByUser(dictAttrName);
			}
		}

		//if only one is available, force that selection now
		if (availableValues.size() == 1)
		{
			selectedValues.push_back(availableValues[0]);
			_removeTouchedByUser(dictAttrName);
			bFound = true;
		}
		//if the current value is "" or will become invalid, and an available value is prefixed with a "@" default, set it now
		else if (currentValues.size() == 1 && (currentValues[0].length() == 0 || find(availableValues.begin(), availableValues.end(), currentValues[0]) == availableValues.end()) && prefixes.size() > 0)
		{
			for (size_t i = 0; i < prefixes.size(); i++)
			{
				if (ROMUTIL::StringContains(prefixes[i], DEFAULTPREFIX))
				{
					if (InvalidateMode == NORMALINVALIDATE || currentValue.length() == 0)
					{
						selectedValues.push_back(availableValues[i]);
						_removeTouchedByUser(dictAttrName);
						bFound = true;
					}
				}
			}
		}
		else if (availableValues.size() == 0)
		{
			m_dict[dictAttrName]->SetEnabled(false);
		}

		if (selectedValues.size() == 0 && currentValues.size() > 0) //compare the new values to what is really available
		{
			for (size_t i = 0; i < newValues.size(); i++)
			{
				vector<string>::iterator itFind = find(availableValues.begin(), availableValues.end(), newValues[i]);
				if (itFind != availableValues.end())
				{
					selectedValues.push_back(newValues[i]);
				}
			}
		}

		string finalValue = "";
		for (vector<string>::iterator it = selectedValues.begin(); it != selectedValues.end(); it++)
		{
			if (finalValue.length() > 0)
				finalValue+= "|";
			finalValue+=*it;
		}
		if (finalValue != currentValue)
		{
			if (!setTheValue)
				setTheValue = bFound;

			if (setTheValue)
				m_ROMContext->SetAttribute(dictAttrName, finalValue);
			else
			{
				if (availableValues.size() > 0)
					_flagAttrInvalid(dictAttrName);
				else
					m_ROMContext->SetAttribute(dictAttrName, finalValue);
			}
		}

		m_dict[dictAttrName]->SetValue(m_ROMContext->GetAttribute(dictAttrName, true));
	}

	//drop down list, radio button groups, etc
	void LinearEngine::_evalSingleSelect(const string& dictAttrName, const string& value, void* context)
	{
		string newValue = value;
		vector<string> res = m_ROMContext->_evaluateTable(m_dict[dictAttrName]->GetRuleTable(), dictAttrName, context);
		vector<string> availableValues;
		m_dict[dictAttrName]->SetEnabled(true);
		m_dict[dictAttrName]->SetValid(true);

		//if no rules table defined, allow any value
		if (m_dict[dictAttrName]->GetRuleTable().size() == 0)
		{
			m_ROMContext->SetAttribute(dictAttrName, newValue);
			m_dict[dictAttrName]->SetValue(newValue);
			availableValues.push_back(newValue);
			m_dict[dictAttrName]->SetAvailableValues(availableValues);
			return;
		}

		//the list of results is what is available for selection in the control
		vector<string> prefixes = _parseOutPrefixes(SINGLESELECT, res, availableValues);
		m_dict[dictAttrName]->SetAvailableValues(availableValues);
		bool bFound = find(availableValues.begin(), availableValues.end(), newValue) != availableValues.end();

		string currentValue = m_ROMContext->GetAttribute(dictAttrName, true);
		bool setTheValue = currentValue.length() == 0 || InvalidateMode == NORMALINVALIDATE;

		//set the dictionary default on load
		if (newValue.length() == 0)
		{
			if (m_dict[dictAttrName]->GetDefaultValue().length() > 0)
			{
				newValue = m_dict[dictAttrName]->GetDefaultValue();
			}
		}

		//if only one is available, force that selection now
		if (availableValues.size() == 1)
		{
			newValue = availableValues[0];
			bFound = true;
			_removeTouchedByUser(dictAttrName);
		}

		//if the current value is "" or will become invalid, and an available value is prefixed with a "@" default, set it now
		if ((currentValue.length() == 0 || (find(availableValues.begin(), availableValues.end(), currentValue) == availableValues.end())) && prefixes.size() > 0)
		{
			for (size_t i = 0; i < prefixes.size(); i++)
			{
				if (ROMUTIL::StringContains(prefixes[i], DEFAULTPREFIX))
				{
					if (InvalidateMode == NORMALINVALIDATE || currentValue.length() == 0)
					{
						newValue = availableValues[i];
						bFound = true;
						_removeTouchedByUser(dictAttrName);
					}
					break;
				}
			}
		}
		else if (availableValues.size() == 0)
		{
			m_dict[dictAttrName]->SetEnabled(false);
		}

		if (prefixes.size() > 0 && prefixes[0].length() > 0 && ROMUTIL::StringContains(prefixes[0], INVISPREFIX))
			m_dict[dictAttrName]->SetVisible(false);
		else
			m_dict[dictAttrName]->SetVisible(true);

		if (newValue.length() > 0 && bFound)
		{
			if (currentValue != newValue)
			{
				if (!setTheValue)
					setTheValue = bFound;

				if (setTheValue)
					m_ROMContext->SetAttribute(dictAttrName, newValue);
				else
					_flagAttrInvalid(dictAttrName);
			}
		}
		else
		{
			if (m_dict[dictAttrName]->GetEnabled() == true)
				m_dict[dictAttrName]->SetValid(false);
			if (setTheValue)
				m_ROMContext->SetAttribute(dictAttrName, "");
			else
			{
				if (availableValues.size() > 0)
					_flagAttrInvalid(dictAttrName);
				else
					m_ROMContext->SetAttribute(dictAttrName, "");
			}

			_removeTouchedByUser(dictAttrName);
		}

		m_dict[dictAttrName]->SetValue(m_ROMContext->GetAttribute(dictAttrName, true));
	}

	vector<string> LinearEngine::_parseOutPrefixes(int AttributeType, const vector<string>& origValues, vector<string> &valuesWithoutPrefixes)
	{
		vector<string> prefixes;
		valuesWithoutPrefixes.clear();

		for (auto it = origValues.begin(); it != origValues.end(); it++)
		{
			string val = *it;
			string fullPrefix;

			//check for leadoff ^ indicating an invisible control
			if (ROMUTIL::StringContains(val, INVISPREFIX))
			{
				fullPrefix+=INVISPREFIX;
				val = val.substr(INVISPREFIX.length());
			}

			//check for leadoff @ indicating a default
			if (ROMUTIL::StringContains(val, DEFAULTPREFIX))
			{
				fullPrefix+=DEFAULTPREFIX;
				val = val.substr(DEFAULTPREFIX.length());
			}

			//check for leadoff # indicating a locked edit box
			if (AttributeType == EDIT && ROMUTIL::StringContains(val, DISABLEPREFIX))
			{
				fullPrefix+=DISABLEPREFIX;
				val = val.substr(DISABLEPREFIX.length());
			}

			prefixes.push_back(fullPrefix);
			valuesWithoutPrefixes.push_back(val);
		}

		return prefixes;
	}

	void LinearEngine::_evaluateDependencies(const string& dictAttrName, void* context)
	{
		m_EvalInternal = true;
		if (m_mapTriggers.find(dictAttrName) != m_mapTriggers.end())
		{
			vector<string> attrsToEval = m_mapTriggers[dictAttrName];
			for (auto it = attrsToEval.begin(); it != attrsToEval.end(); it++)
			{
				auto itFind = m_dict.find(*it);
				if (itFind != m_dict.end())
				{
					vector<string> selectedValues = _getSelectedValues(itFind->second);
					bool bWasChangedByUser = itFind->second->GetChangedByUser();
					_evaluateForAttribute(*it, selectedValues, true, context);
					m_EvalInternal = true;
					if (bWasChangedByUser)
					{
						bool bValuesRemainSame = true;
						vector<string> newSelectedValues = _getSelectedValues(itFind->second);
						if (newSelectedValues.size() != selectedValues.size())
							bValuesRemainSame = false;
						else for (size_t i = 0; i < selectedValues.size(); i++)
						{
							auto itFind = find(newSelectedValues.begin(), newSelectedValues.end(), selectedValues[i]);
							if (itFind == newSelectedValues.end())
							{
								bValuesRemainSame = false;
								break;
							}
						}

						itFind->second->SetChangedByUser(bValuesRemainSame);
					}
				}
			}
		}
		m_EvalInternal = false;
	}

	vector<string> LinearEngine::_getSelectedValues(IROMDictionaryAttribute* attr)
	{
		vector<string> retval;
		string currentValue = m_ROMContext->GetAttribute(attr->GetName(), true);

		switch (attr->GetAttributeType())
		{
		case SINGLESELECT:
			retval.push_back(currentValue);
			break;

		case MULTISELECT:
			retval = ROMUTIL::Split(currentValue, "|");
			break;

		case BOOLEANSELECT:
			retval.push_back(currentValue);
			break;

		case EDIT:
			retval.push_back(currentValue);
			break;

		default:
			retval.push_back(currentValue);
			break;
		}

		return retval;
	}
}
