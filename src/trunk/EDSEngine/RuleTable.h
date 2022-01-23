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
#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <functional>

#include "Bimapper.h"
#include "RuleCell.h"

using namespace std;

class CRuleTable
{
	friend class CTableSet;

public:
	CRuleTable(void);
	~CRuleTable(void);

	vector<string> EvaluateTable(const string& outputAttr, bool bGetAll, bool bForward, void* context = nullptr); //results come back as the full strings
	map<string, vector<string> > EvaluateTable(bool bGetAll, bool bForward, void* context = nullptr);
	bool HasChain() {return bHasChain;}
	bool HasJS() {return bHasJavascript;}
	bool HasPython() {return bHasPython;}
	bool IsGetAll() {return bGetAll;}
	string GetTableName() {return m_Name;}

	vector<string> GetAllOutputAttrNames();
	vector<string> GetAllInputAttrNames();
	vector<string> GetAllInputDependencies(); //input columns + formulas/gets
	vector<string> GetAllPossibleOutputs(const string& outputName);
	vector<pair<string, vector<CRuleCell> > > GetOutputAttrsValues() {return m_OutputAttrsValues;}
	vector<pair<string, vector<CRuleCell> > > GetInputAttrsTests() {return m_InputAttrsTests;}

	//ordered vector of (string)inputAttrName, (vector)inputAttrTest pairs, vector of outputAttrs
	void CreateRuleTable(vector<pair<string, vector<CRuleCell> > >& inputAttrsTests,
		vector<pair<string, vector<CRuleCell> > >& outputAttrsValues,
		vector<string>& formulaInputs, CBimapper *stringMap,
		const string& name = "default", bool GetAll = false);
	CRuleTable(vector<pair<string, vector<CRuleCell> > >& inputAttrsTests,
		vector<pair<string, vector<CRuleCell> > >& outputAttrsValues,
		vector<string>& formulaInputs, CBimapper *stringMap,
		const string& name = "default", bool GetAll = false) { CreateRuleTable(inputAttrsTests, outputAttrsValues, formulaInputs, stringMap, name, GetAll); }
	void EnbleDebugging(bool enable) { m_DEBUGGING = enable; }
	void SetThreadCount(size_t threads);

	string DebugMessage;
	function<string(const string&, void*)> InputValueGetter;

private:
	void DebugEval(const string& outputAttr, const vector<CToken>& inputValues, const map<size_t, set<string>>& solutions);
    vector<bool> _runTests(bool bGetAll, vector<pair<string, vector<CRuleCell> > >* inputCollection, vector<CToken>& values, void* context);
    bool _runTestGroup(bool bGetAll, size_t startIndex, size_t endIndex, vector<pair<string, vector<CRuleCell> > >* inputCollection, vector<CToken>& values, vector<bool>& colResults, void* context);
	void _init();

	vector<pair<string, vector<CRuleCell> > > m_InputAttrsTests; //the test table, input rows
	vector<string> m_FormulaInputs;
	vector<pair<string, vector<CRuleCell> > > m_OutputAttrsValues; //output rows
	string m_Name;
	size_t m_Tests;
	CBimapper *m_stringsMap;
	bool bHasChain;
	bool bHasPython;
	bool bHasJavascript;
	bool bGetAll;
	bool m_DEBUGGING;
	size_t m_Threads;
	bool m_ThreadingEnabled;
};


/*A table contains a matrix of indexes that represent string constants
tables read top->down left->right as conceptualized in code (linear, inference based rules engine)

input	attr1	101
input	attr2	231
input	attr3	255					where 101,131,255 mark hash map to strings
output	result1	356
output	result2	896					ditto for results

There could be multiple columns of indexes to represent multiple results, hence: vector<size_t>

Each "row" of attr is stored as a pair, the attrname is the key for the indexes, hence: pair<wsring, vector<size_t>>

A full table is the assembly of all input rows + all output rows, hence: vector<pair<string, vector<size_t>>>

Test values are fed into the table for testing by index as well, so a true "column" is when all cells are true.
An empty input cell is always true.  Missing/invalid inputs are 0.  Empty string id is always 1.  Explicit NULL input is 2.
*/
