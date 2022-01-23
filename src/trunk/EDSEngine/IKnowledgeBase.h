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
#include <vector>
#include <map>
#include <functional>
namespace EDS
{

	class IKnowledgeBase
	{
	public:
		virtual bool CreateKnowledgeBase(std::string knowledge_file) = 0;
		virtual bool CreateKnowledgeBaseFromString(std::string xmlStr) = 0;
		virtual size_t TableCount() = 0;
		virtual bool IsOpen() = 0;
		virtual void EnableRemoteDebugger(bool enable) = 0;
		virtual void SetMaxThreads(size_t threads) = 0;

		virtual bool TableHasScript(const std::string& tableName) = 0;
		virtual bool TableIsGetAll(const std::string& tableName) = 0;
		virtual std::vector<std::string> EvaluateTableWithParam(const std::string& tableName, const std::string& outputAttr, bool bGetAll, std::string& param, void* context = nullptr) = 0;
		virtual std::vector<std::string> EvaluateTableWithParam(const std::string& tableName, const std::string& outputAttr, std::string& param, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > EvaluateTableWithParam(const std::string& tableName, bool bGetAll, std::string& param, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > EvaluateTableWithParam(const std::string& tableName, std::string& param, void* context = nullptr) = 0;
		virtual std::vector<std::string> EvaluateTable(const std::string& tableName, const std::string& outputAttr, bool bGetAll, void* context = nullptr) = 0;
		virtual std::vector<std::string> EvaluateTable(const std::string& tableName, const std::string& outputAttr, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > EvaluateTable(const std::string& tableName, bool bGetAll, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > EvaluateTable(const std::string& tableName, void* context = nullptr) = 0;
		virtual std::string GetFirstTableResult(const std::string& tableName, const std::string& outputAttr, void* context = nullptr) = 0;
		virtual std::vector<std::string> ReverseEvaluateTable(const std::string& tableName, const std::string& inputAttr, bool bGetAll, void* context = nullptr) = 0;
		virtual std::vector<std::string> ReverseEvaluateTable(const std::string& tableName, const std::string& inputAttr, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > ReverseEvaluateTable(const std::string& tableName, bool bGetAll, void* context = nullptr) = 0;
		virtual std::map<std::string, std::vector<std::string> > ReverseEvaluateTable(const std::string& tableName, void* context = nullptr) = 0;


		virtual std::vector<std::string> GetInputAttrs(const std::string& tableName) = 0;
		virtual std::vector<std::string> GetInputDependencies(const std::string& tableName) = 0;
		virtual std::vector<std::string> GetOutputAttrs(const std::string& tableName) = 0;
		virtual std::vector<std::string> GetAllPossibleOutputs(const std::string& tableName, const std::string& outputName) = 0;

		//Translations
		virtual std::string Localize(const std::string& baseValue, const std::string& locale) = 0;
		virtual std::string DeLocalize(const std::string& localeValue) = 0;
		virtual std::string Translate(const std::string& source, const std::string& sourceLocale, const std::string& destLocale) = 0;

		virtual void SetDebugHandler(std::function<void(const std::string&)> handler) = 0;
		virtual std::function<void(const std::string&)> GetDebugHandler() = 0;

		virtual void SetInputValueGetter(std::function<std::string(const std::string&, void*)> handler) = 0;
		virtual std::function<std::string(const std::string&, void*)> GetInputValueGetter() = 0;
	};
}