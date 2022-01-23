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

#include <string>
#include <map>
#include "RuleTable.h"

using namespace std;

//the collection of all active rules tables + some useful functions
class CTableSet
{

public:
	CTableSet(void);
	~CTableSet(void);

	void AddTable(vector<pair<string, vector<CRuleCell> > > inputAttrsTests,
		vector<pair<string, vector<CRuleCell> > > outputAttrsValues,
		vector<string> formulaInputs, CBimapper *stringMap, string name = "defualt", bool GetAll = false);
	void Initialize();
	CRuleTable* GetTable(const string& tableName);
	CRuleTable GetTableCopy(const string& tableName, bool *found);
	vector<string> GetInputAttrs(const string& tableName);
	vector<string> GetInputDependencies(const string& tableName);
	vector<string> GetOutputAttrs(const string& tableName);
	size_t Count();
	void LoadTableInfo(CRuleTable *table);
	void SetMaxThreads(size_t threads);
private:
	vector<string> ParseTablesAndChainsForInputs(const string& tableName);
	map<string, CRuleTable> m_tables;
	map<string, vector<string> > m_inputAttrsByTable;
	map<string, vector<string> > m_inputDependenciesByTable;
	map<string, vector<string> > m_outputAttrsByTable;
	bool bInitialized;
};
