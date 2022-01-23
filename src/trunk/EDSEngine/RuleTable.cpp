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
    asize_t with EDSEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "RuleTable.h"
#include "Decode.h"
#include <set>
#include <algorithm>
#include <list>
#include <string>
//#ifndef CLR
#include <thread>
//#endif
#include <memory>
#include "utilities.h"

#define THREAD_THRESHOLD 20 //at least this many rules to trigger threading

using namespace std;

CRuleTable::CRuleTable(void)
{
	_init();
}

CRuleTable::~CRuleTable(void)
{

}

void CRuleTable::_init()
{
	m_DEBUGGING = false;
	m_Threads = 1;
	m_Tests = 0;
	bHasChain = false;
	bHasPython = false;
	bHasJavascript = false;
	bGetAll = false;
	m_ThreadingEnabled = false;
}

void CRuleTable::CreateRuleTable(vector<pair<string, vector<CRuleCell> > >& inputAttrsTests,
								vector<pair<string, vector<CRuleCell> > >& outputAttrsValues,
								vector<string>& formulaInputs, CBimapper *stringMap,
								const string& name, bool GetAll)
{
	_init();

	m_FormulaInputs = formulaInputs;
	m_InputAttrsTests = inputAttrsTests;
	m_OutputAttrsValues = outputAttrsValues;

	if (m_OutputAttrsValues.size() > 0)
    {
        m_Tests = m_OutputAttrsValues[0].second.size();
    }

	m_Name = name;
	m_stringsMap = stringMap;
	bHasChain = false;
	bHasPython = false;
	bHasJavascript = false;

	bGetAll = GetAll;
}

void CRuleTable::SetThreadCount(size_t threads)
{
	m_Threads = threads;
//#ifndef CLR
	if (m_Threads > 1)
		m_ThreadingEnabled = m_Tests >= THREAD_THRESHOLD;
//#else
//	m_ThreadingEnabled = false;
//#endif
}

map<string, vector<string> > CRuleTable::EvaluateTable(bool bGetAll, bool bForward, void* context)
{
	map<string, vector<string> > retval;
	vector<pair<string, vector<CRuleCell> > > *resultCollection;
	if (bForward)
		resultCollection = &m_OutputAttrsValues;
	else
		resultCollection = &m_InputAttrsTests;

	//for all the outputs get the results
	for (vector<pair<string, vector<CRuleCell> > >::iterator itOut = resultCollection->begin(); itOut != resultCollection->end(); itOut++)
	{
		vector<string> result = EvaluateTable((*itOut).first, bGetAll, bForward, context);
		retval[(*itOut).first] = result;
	}

	return retval;
}

vector<string> CRuleTable::EvaluateTable(const string& outputAttr, bool bGetAll, bool bForward, void* context)
{
	vector<string> retval;
	vector<CToken> values;
	map<size_t, set<string> > solutions;
	vector<pair<string, vector<CRuleCell> > > *inputCollection;
	vector<pair<string, vector<CRuleCell> > > *outputCollection;
	if (bForward)
	{
		inputCollection = &m_InputAttrsTests;
		outputCollection = &m_OutputAttrsValues;
	}
	else
	{
		inputCollection = &m_OutputAttrsValues;
		outputCollection = &m_InputAttrsTests;
	}

	vector<bool> colResults (m_Tests, true); //a table need not have any inputs
	if (inputCollection->size() > 0)
	{
		//get the current values of all input attrs
		for (vector<pair<string, vector<CRuleCell> > >::iterator it = inputCollection->begin(); it != inputCollection->end(); it++)
		{
			string attrName = it->first;
			CToken token;
			token.Value = InputValueGetter(attrName, context);
			token.ID = m_stringsMap->GetIDByString(token.Value);
			values.push_back(token);
		}

		//sweep down the table for all inputs and do test(s)
		colResults = _runTests(bGetAll, inputCollection, values, context);

	} //done inputs
	else if (inputCollection->size() == 0 && !bGetAll)
	{
		if (colResults.size() > 1)
			for (size_t i = 1; i < colResults.size(); i++)
				colResults[i] = false;
	}

	//for the give output, the reuslts are
	vector<CRuleCell> results;
	for (size_t result = 0; result < outputCollection->size(); result++)
	{
		if ((*outputCollection)[result].first == outputAttr)
		{
			results = (*outputCollection)[result].second;
		}
	}

	//for each true result, add to the solution vector
	for (size_t result = 0; result < m_Tests; result++)
	{
		if (colResults[result] && result < results.size())
		{
			CRuleCell outputCell = results[result];
			CDecode decoder(outputCell, InputValueGetter, m_stringsMap, context);
			if (outputCell.Operation & CHAIN)
				bHasChain = true;
			if (outputCell.Operation & PYTHON)
				bHasPython = true;
			if (outputCell.Operation & JAVASCRIPT)
				bHasJavascript = true;
			vector<string> cellOutputs = decoder.EvaluateOutputCell();
			for (vector<string>::iterator itOutputs = cellOutputs.begin(); itOutputs != cellOutputs.end(); itOutputs++)
			{
				retval.push_back(*itOutputs);
				if (m_DEBUGGING)
				{
					map<size_t, set<string> >::iterator itFind = solutions.find(result);
					if (itFind != solutions.end())
					{
						itFind->second.insert(*itOutputs);
					}
					else
					{
						solutions[result].insert(*itOutputs);
					}
				}
			}
		}
	}


	//report the eval results over a tcp socket connection
	if (m_DEBUGGING)
	{
		DebugEval(outputAttr, values, solutions);
	}

	return retval;
}

vector<bool> CRuleTable::_runTests(bool bGetAll, vector<pair<string, vector<CRuleCell> > >* inputCollection, vector<CToken>& values, void* context)
{
    //sweep down the table for all inputs and do test(s)
    vector<bool> colResultsDefault (m_Tests, false);
    vector<bool> colResults = colResultsDefault;

//#ifndef CLR
	if (m_ThreadingEnabled && bGetAll)
    {
        size_t testsPerThread = m_Tests/m_Threads;
        unique_ptr<thread[]> threads = unique_ptr<thread[]>(new thread[m_Threads]);
        for (size_t i = 0; i < m_Threads; i++)
        {
            size_t startIndex = i * testsPerThread;
            size_t endIndex = i == m_Threads - 1 ? m_Tests : (i + 1) * testsPerThread;
            function<bool(void)> worker = [&]()
            {
                return _runTestGroup(bGetAll, startIndex, endIndex, inputCollection, values, colResults, context);
            };
            threads[i] = thread(worker);
        }

        for (size_t i = 0; i < m_Threads; i++)
        {
            threads[i].join();
#ifdef _DEBUG
			std::cout<<"Spawning eval thread"<<std::endl;
#endif
        }
    }
    else
//#endif
    {
        _runTestGroup(bGetAll, 0, m_Tests, inputCollection, values, colResults, context);
    }

    return colResults;
}

bool CRuleTable::_runTestGroup(bool bGetAll, size_t startIndex, size_t endIndex, vector<pair<string, vector<CRuleCell>>>* inputCollection, vector<CToken>& values, vector<bool>& colResults, void* context)
{
    bool bHaveSolution = true;
    for (size_t testIndex = startIndex; testIndex < endIndex; testIndex++)
    {
        //sweep through the inputs
        size_t inputCnt = 0;
        for (auto itTests = inputCollection->begin(); itTests != inputCollection->end(); itTests++)
        {
            if ( testIndex < (*itTests).second.size())
            {
				CDecode decoder(values[inputCnt], (*itTests).second[testIndex], InputValueGetter, m_stringsMap, context);
                bHaveSolution = decoder.EvaluateInputCell();
            }
            inputCnt++;
            if (!bHaveSolution)
                break;
        }
        colResults[testIndex] = bHaveSolution;
        if (bHaveSolution && !bGetAll)
            break;
    } //done column (rule)
    return bHaveSolution;
}

void CRuleTable::DebugEval(const string& outputAttr, const vector<CToken>& inputValues, const map<size_t, set<string>>& solutions)
{
	string xmlBlob;
	xmlBlob += "<TableEval name=\"";
	xmlBlob += this->m_Name;
	xmlBlob += "\" output=\"";
	xmlBlob += outputAttr;
	xmlBlob += "\">";

	if (m_InputAttrsTests.size() == inputValues.size())
	{
		xmlBlob += "<Inputs>";
		for (size_t i = 0; i < m_InputAttrsTests.size(); i++)
		{
			pair<string, vector<CRuleCell> > currentPair = m_InputAttrsTests[i];
			string attr = currentPair.first;
			xmlBlob += "<Input name = \"";
			xmlBlob += attr;
			xmlBlob += "\" value=\"";
			xmlBlob += inputValues[i].Value;
			xmlBlob += "\"/>";
		}
		xmlBlob += "</Inputs>";
	} //else something is wrong

	xmlBlob+= "<Outputs>";
	for (auto it = solutions.begin(); it != solutions.end(); it++)
	{
		size_t index = (*it).first;
		for (set<string>::iterator itOut = (*it).second.begin(); itOut != (*it).second.end(); itOut++)
		{
			xmlBlob += "<Output value=\"";
			xmlBlob += *itOut;
			xmlBlob += "\" index=\"";
			xmlBlob += to_string(index);
			xmlBlob += "\"/>";
		}
	}
	xmlBlob+= "</Outputs>";
	xmlBlob+= "</TableEval>";

	DebugMessage = xmlBlob;
}

vector<string> CRuleTable::GetAllPossibleOutputs(const string& outputName)
{
	vector<string> retval;
	std::list<size_t> allValues;

	for (vector<pair<string, vector<CRuleCell> > >::iterator it = m_OutputAttrsValues.begin(); it != m_OutputAttrsValues.end(); it++)
	{
		pair<string, vector<CRuleCell> > row = *it;

		if (row.first == outputName)
		{
			for (vector<CRuleCell>::iterator itRow = row.second.begin(); itRow != row.second.end(); itRow++)
			{
				for (vector<size_t>::iterator itValue = (*itRow).Values.begin(); itValue != (*itRow).Values.end(); itValue++)
					allValues.push_back(*itValue);
			}
			break;
		}
	}

	allValues.sort();
	allValues.unique();

	for (list<size_t>::iterator it = allValues.begin(); it != allValues.end(); it++)
	{
		retval.push_back(m_stringsMap->GetStringByID(*it));
	}

	return retval;
}

vector<string> CRuleTable::GetAllInputAttrNames()
{
	vector<string> retval;

	for (vector<pair<string, vector<CRuleCell> > >::iterator it = m_InputAttrsTests.begin(); it != m_InputAttrsTests.end(); it++)
	{
		if ((*it).first.size() > 0)
			retval.push_back((*it).first);
	}

	return retval;
}

vector<string> CRuleTable::GetAllInputDependencies()
{
	vector<string> retval;

	retval = GetAllInputAttrNames();
	for (vector<string>::iterator it = m_FormulaInputs.begin(); it != m_FormulaInputs.end(); it++)
	{
		retval.push_back(*it);
	}

	return retval;
}

vector<string> CRuleTable::GetAllOutputAttrNames()
{
	vector<string> retval;

	for (vector<pair<string, vector<CRuleCell> > >::iterator it = m_OutputAttrsValues.begin(); it != m_OutputAttrsValues.end(); it++)
	{
		if ((*it).first.size() > 0)
			retval.push_back((*it).first);
	}

	return retval;
}

