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
#include <map>

namespace ROM
{
	class IROMDictionaryAttribute
	{
	public:
		virtual void CreateROMDictionaryAttribute() = 0;
		virtual std::string GetName() = 0; const
		virtual void SetName(const std::string& name) = 0;
		virtual std::string GetDescription() = 0; const
		virtual void SetDescription(const std::string& desc) = 0;
		virtual std::string GetDefaultValue() = 0; const
		virtual void SetDefaultValue(const std::string& value) = 0;
		virtual std::string GetRuleTable() = 0; const
		virtual void SetRuleTable(const std::string& table) = 0;
		virtual int GetAttributeType() = 0; const
		virtual void SetAttributeType(int type) = 0;
		virtual size_t GetIndex() = 0; const
		virtual void SetIndex(size_t index) = 0;
		virtual bool GetValueChanged() = 0; const
		virtual void SetValueChanged(bool changed) = 0;
		virtual bool GetChangedByUser() = 0; const
		virtual void SetChangedByUser(bool userChanged) = 0;
		virtual bool GetValid() = 0; const
		virtual void SetValid(bool valid) = 0;
		virtual bool GetVisible() = 0; const
		virtual void SetVisible(bool visible) = 0;
		virtual bool GetEnabled() = 0; const
		virtual void SetEnabled(bool enabled) = 0;

		virtual std::vector<std::string> GetPossibleValues() = 0; const
		virtual void SetPossibleValues(const std::vector<std::string>& values) = 0;
		virtual std::vector<std::string> GetAvailableValues() = 0; const
		virtual void SetAvailableValues(const std::vector<std::string>& values) = 0;
		virtual std::string GetValue() = 0; const
		virtual void SetValue(const std::string& value) = 0;
	};

	class IDictionaryInterface
	{
	public:
		virtual void LoadDictionary(const std::string& dictionaryTable) = 0;
		virtual IROMDictionaryAttribute* GetDictionaryAttr(const std::string& dictAttrName) = 0;
		virtual std::map<std::string, IROMDictionaryAttribute*>* GetAllDictionaryAttrs() = 0;

#ifdef WIN32
		struct DispatchHelper;
	protected:
		friend DispatchHelper;
#endif

	private:
		//these internal methods are called by .NET to assist with passing of managed objects
		virtual void _loadDictionary(const std::string& dictionaryTable, void* context) = 0;
	};
}