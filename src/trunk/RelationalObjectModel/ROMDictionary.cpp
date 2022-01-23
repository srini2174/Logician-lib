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
#include "ROMDictionary.h"
#include <algorithm>
#include <memory>

using namespace ROM;
using namespace ROMUTIL;

ROMDictionary::~ROMDictionary(void)
{
	for (auto kvp : m_dict)
	{
		delete kvp.second;
	}
}

void ROMDictionary::CreateROMDictionary(ROMNode* context)
{
	m_ROMContext = context;
	m_tableName = "";
}

/*A dictionary table should have the following outputs:
Name - ROM attribute name
Description - Visible attribute description in UI
DefaultValue - The initial value to set
AttributeType - default empty value(SINGLESELECT), SINGLESELECT, MULTISELECT, BOOLEAN, EDIT, STATIC
RuleTable - table to evaluate to obtain the value (will override default if exists). Each rule table should have
	an output column name that matches the attribute name
*/
void ROMDictionary::_loadDictionary(const string& dictionaryTable, void* context)
{
	m_tableName = dictionaryTable;
	m_dict.clear();
	map<string, vector<string> > res = m_ROMContext->_evaluateTable(m_tableName, true, context);
	vector<string> allNames = res["Name"];

	for (size_t i = 0; i < allNames.size(); i++)
	{
		IROMDictionaryAttribute* dictAttr = new ROMDictionaryAttribute();
		dictAttr->SetName(allNames[i]);
		dictAttr->SetIndex(i);
		if (res["DefaultValue"].size() > 0 && res["DefaultValue"][i] != "~") dictAttr->SetDefaultValue(res["DefaultValue"][i]);
		if (res["Description"].size() > 0 && res["Description"][i] != "~") dictAttr->SetDescription(res["Description"][i]);
		if (res["RuleTable"].size() > 0 && res["RuleTable"][i] != "~") dictAttr->SetRuleTable(res["RuleTable"][i]);

		string strAttrType;
		if (res["AttributeType"].size() > 0  && res["AttributeType"][i] != "~") strAttrType = res["AttributeType"][i];
		transform(strAttrType.begin(), strAttrType.end(), strAttrType.begin(), ::toupper);

		if (strAttrType.length() == 0 || strAttrType == "SINGLESELECT")
		{
			dictAttr->SetAttributeType(SINGLESELECT);
		}
		else if (strAttrType == "MULTISELECT")
		{
			dictAttr->SetAttributeType(MULTISELECT);
		}
		else if (strAttrType == "BOOLEAN")
		{
			dictAttr->SetAttributeType(BOOLEANSELECT);
		}
		else if (strAttrType == "EDIT")
		{
			dictAttr->SetAttributeType(EDIT);
		}
		else if (strAttrType == "STATIC")
		{
			dictAttr->SetAttributeType(STATIC);
		}

		//on load, just set default values and possibilities
		//only set a default if there is no rules table and no current value
		string value = m_ROMContext->GetAttribute(dictAttr->GetName());
		if (((value.length() == 0 && dictAttr->GetRuleTable().length() == 0) || dictAttr->GetAttributeType() == STATIC) && dictAttr->GetDefaultValue().length() > 0 && dictAttr->GetDefaultValue() != "~")
		{
			if (dictAttr->GetAttributeType() == BOOLEANSELECT)
				m_ROMContext->SetAttribute(dictAttr->GetName(), dictAttr->GetDefaultValue().substr(0, 1));
			else
				m_ROMContext->SetAttribute(dictAttr->GetName(), dictAttr->GetDefaultValue());
		}

		if (dictAttr->GetRuleTable().length() > 0)
			dictAttr->SetPossibleValues(m_ROMContext->_getPossibleValues(dictAttr->GetRuleTable(), dictAttr->GetName()));

		m_dict[dictAttr->GetName()] = dictAttr;
	}
}

IROMDictionaryAttribute* ROMDictionary::GetDictionaryAttr(const string& dictAttrName)
{
	IROMDictionaryAttribute* retval = nullptr;

	auto itFind = m_dict.find(dictAttrName);
	if (itFind != m_dict.end())
		retval = m_dict[dictAttrName];

	return retval;
}

