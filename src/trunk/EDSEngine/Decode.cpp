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
#include "Decode.h"
#include "utilities.h"

using namespace EDSUTIL;

#define _CRT_SECURE_NO_DEPRECATE 1

CDecode::~CDecode(void)
{
}

CDecode::CDecode(CRuleCell& outputCell, function<string(const string&, void*)> inputAttrGetter, CBimapper* stringMap, void* context)
{
	m_tests = &outputCell.Values;
	m_stringsMap = stringMap;
	m_operator = outputCell.Operation;
	m_inputAttrGetter = inputAttrGetter;
	m_context = context;
}

CDecode::CDecode(CToken& inputValue, CRuleCell& inputCell, function<string(const string&, void*)> inputAttrGetter, CBimapper* stringMap, void* context)
{
	m_value = &inputValue;
	m_tests = &inputCell.Values;
	m_stringsMap = stringMap;
	m_operator = inputCell.Operation;
	m_inputAttrGetter = inputAttrGetter;
	m_context = context;

	CheckForInputGets();
}

string CDecode::GetString(size_t lKey)
{
	if (lKey > EMPTY_STRING)
	{
		string s = m_stringsMap->GetStringByID(lKey);
		if(s.length() > 0)
			return s;
		else
		{
			ReportError("string not found for index: " + to_string(lKey));
		}
	}
	return "";
}

bool CDecode::EvaluateInputCell()
{
	bool		retval = true;
	try
	{
		//check by operator
		//no string compares needed here for speed
		if (m_operator & EQUALS)
		{
			for (size_t i = 0; i < m_tests->size(); i++) //possible OR
			{
				size_t test = (*m_tests)[i];

				if (test == m_value->ID && test > 0)
				{
					retval = true;
					break;
				}
				else
					retval = false;

				//explict NULL check
				if (retval == false && test == EXPLICIT_NULL_STRING && m_value->ID == EMPTY_STRING)
					retval = true;
			}
		}
		else if (m_operator & NOT_EQUAL)
		{
			for (size_t i = 0; i < m_tests->size(); i++) //possible OR
			{
				size_t test = (*m_tests)[i];

				if (test == m_value->ID)
				{
					retval = false;
				}

				//explict NULL check
				if (retval == true && test == EXPLICIT_NULL_STRING && m_value->ID == EMPTY_STRING)
					retval = false;
			}
		}
		//the other operations require the strings to be de-tokenized for values tests,
		//there are no "ORs" in these cases
		else if (m_operator & LESS_THAN || m_operator & LESS_THAN_EQUAL || m_operator & GREATER_THAN || m_operator & GREATER_THAN_EQUAL)
		{
			string currentTest = GetString((*m_tests)[0]);
			string currentValue = m_value->Value;
			bool bIsNum = false;

			if (EDSUTIL::StringIsNumeric(currentTest) == true && EDSUTIL::StringIsNumeric(currentValue) == true)
			{
				bIsNum = true;
			}
			string	testValue;
			double dCurrentValue = 0;
			double dTestValue = 0;

			testValue.assign(currentTest.begin(), currentTest.end());
			if (bIsNum)
			{
				try
				{
					dCurrentValue = atof(currentValue.c_str());
					dTestValue = atof(testValue.c_str());
				}
				catch(...)
				{
					string error = "Could not create numeric values from string data";
					bIsNum = false;
				}
			}


			if (m_operator & LESS_THAN)
			{
				if (bIsNum)
				{
					retval = dCurrentValue < dTestValue;
				}
				else
				{
					retval = currentValue < testValue;
				}
			}
			else if (m_operator & LESS_THAN_EQUAL)
			{
				if (bIsNum)
				{
					retval = dCurrentValue <= dTestValue;
				}
				else
				{
					retval = currentValue <= testValue;
				}
			}
			else if (m_operator & GREATER_THAN)
			{
				if (bIsNum)
				{
					retval = dCurrentValue > dTestValue;
				}
				else
				{
					retval = currentValue > testValue;
				}
			}
			else if (m_operator & GREATER_THAN_EQUAL)
			{
				if (bIsNum)
				{
					retval = dCurrentValue >= dTestValue;
				}
				else
				{
					retval = currentValue >= testValue;
				}
			}
		}
		//will have to deal with the comma seperator to get values
		else if (m_operator & RANGE_INCLUSIVE || m_operator & RANGE_END_INCLUSIVE ||
			m_operator & RANGE_START_INCLUSIVE ||m_operator & RANGE_NOT_INCLUSIVE)
		{
			string testString = GetString((*m_tests)[0]);
			string currentValue = m_value->Value;
			double min = 0, max = 0, dCurrentValue = 0;
			vector<string> parts = Split(testString.c_str(), ",");

			if (parts.size() == 2)
			{
				try
				{
					min = atof(parts[0].c_str());
					max = atof(parts[1].c_str());
					dCurrentValue = atof(currentValue.c_str());
				}
				catch(...)
				{
					string error = "Could not create numeric values from string data";
					throw error;
				}


				if (m_operator & RANGE_INCLUSIVE)
				{
					retval = (dCurrentValue >= min && dCurrentValue <= max);
				}
				else if (m_operator & RANGE_END_INCLUSIVE)
				{
					retval = (dCurrentValue > min && dCurrentValue <= max);
				}
				else if (m_operator & RANGE_START_INCLUSIVE)
				{
					retval = (dCurrentValue >= min && dCurrentValue < max);
				}
				else if (m_operator & RANGE_NOT_INCLUSIVE)
				{
					retval = (dCurrentValue > min && dCurrentValue < max);
				}

			}
		}

		else if(m_operator & PYTHON)
		{
		    string error = "scripting not supported as an input";
			throw error;
		}

	}
	catch (string)
	{
		retval = false;
	}
	catch(...)
	{
		retval = false;
	}
	return retval;
}

vector<string> CDecode::EvaluateOutputCell()
{
	vector<string> retval;

	try
	{
		for (vector<size_t>::iterator it = m_tests->begin(); it != m_tests->end(); it++)
		{
			if (m_operator & GETS)
			{
				retval.push_back(ParseStringForGets(*it, false));
			}
			else
			{
				retval.push_back(GetString(*it));
			}
		}
	}
	catch(...)
	{

	}

	return retval;
}

string CDecode::ReplaceAGet(const string& s, bool bForceZero)
{
	string retval = "";
	//find the get(xxx) substring.  attrName xxx is between ().
	int iStartPos = s.find("get(", 0);
	int iEndPos = s.find_first_of(")", iStartPos);
	if (iStartPos >= 0 && iEndPos > iStartPos)
	{
		string attrName(s.begin() + iStartPos + 4, s.begin() + iEndPos);
		string getText = "get(" + attrName + ")";
		//get the value of the input attr
		bool bFoundAttr = false;
		if (m_inputAttrGetter != nullptr)
		{
			string value = m_inputAttrGetter(attrName, m_context);
			if (value.size() > 0)
			{
				bFoundAttr = true;
				retval = EDSUTIL::FindAndReplace(s, getText, value);
			}			
		}

		if (!bFoundAttr)
		{
			if (bForceZero)
				retval = EDSUTIL::FindAndReplace(s, getText, "0");
			else
				retval = EDSUTIL::FindAndReplace(s, getText, "");
		}
	}

	return retval;
}

string CDecode::ParseStringForGets(size_t lKey, bool bForceZero)
{
	string retval = "";

	string fullString = GetString(lKey);
	//replace with the actual value
	if (StringContains(fullString, "get("))
	{
		do
		{
			fullString = ReplaceAGet(fullString, bForceZero);
			retval = fullString;
		} while(StringContains(fullString, "get("));
	}
	else
		retval = fullString;

	return retval;
}

void CDecode::CheckForInputGets()
{
	if (m_operator & GETS)
	for (size_t i = 0; i < m_tests->size(); i++)
	{
		//replace the GET with the actual value (pay a performance penalty here, but convienient)
		string val = ParseStringForGets((*m_tests)[i], false);
		(*m_tests)[i] = m_stringsMap->GetIDByString(val);
	}
}
