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

#include <string>
#include <vector>
#include "ROMInterfaces.h"

namespace ROM
{
	class ROMDictionaryAttribute : public IROMDictionaryAttribute
	{
	public:
		ROMDictionaryAttribute() {CreateROMDictionaryAttribute();}
		virtual ~ROMDictionaryAttribute() {}
		virtual void CreateROMDictionaryAttribute() override { m_Visible = true; m_Valid = false; m_ValueChanged = false; m_ChangedByUser = false; m_Enabled = true; m_Index = 0; }
		
		virtual std::string GetName() override { return m_Name; } const
		virtual void SetName(const std::string& name) override { m_Name = name; }
		virtual std::string GetDescription() override { return m_Description; } const
		virtual void SetDescription(const std::string& desc) override { m_Description = desc; }
		virtual std::string GetDefaultValue() override { return m_DefaultValue; } const
		virtual void SetDefaultValue(const std::string& value) override { m_DefaultValue = value; }
		virtual std::string GetRuleTable() override { return m_RuleTable; } const
		virtual void SetRuleTable(const std::string& table) override { m_RuleTable = table; }
		virtual int GetAttributeType() override { return m_AttributeType; } const
		virtual void SetAttributeType(int type) override { m_AttributeType = type; }
		virtual size_t GetIndex() override { return m_Index; } const
		virtual void SetIndex(size_t index) override { m_Index = index; }
		virtual bool GetValueChanged() override { return m_ValueChanged; } const
		virtual void SetValueChanged(bool changed) override { m_ValueChanged = changed; }
		virtual bool GetChangedByUser() override { return m_ChangedByUser; } const
		virtual void SetChangedByUser(bool userChanged) override { m_ChangedByUser = userChanged; }
		virtual bool GetValid() override { return m_Valid; } const
		virtual void SetValid(bool valid) override { m_Valid = valid; }
		virtual bool GetVisible() override { return m_Visible; } const
		virtual void SetVisible(bool visible) override { m_Visible = visible; }
		virtual bool GetEnabled() override { return m_Enabled; } const
		virtual void SetEnabled(bool enabled) override { m_Enabled = enabled; }

		virtual std::vector<std::string> GetPossibleValues() override { return m_PossibleValues; } const
		virtual void SetPossibleValues(const std::vector<std::string>& values) override { m_PossibleValues = values; }
		virtual std::vector<std::string> GetAvailableValues() override { return m_AvailableValues; } const
		virtual void SetAvailableValues(const std::vector<std::string>& values) override { m_AvailableValues = values; }
		virtual std::string GetValue() override { return m_Value; } const
		virtual void SetValue(const std::string& value) override { m_Value = value; }

	private:
		std::string m_Name;
		std::string m_Description;
		std::string m_DefaultValue;
		std::string m_RuleTable;
		int m_AttributeType;
		size_t m_Index;
		bool m_ValueChanged;
		bool m_ChangedByUser;
		bool m_Valid;
		bool m_Visible;
		bool m_Enabled;

		std::vector<std::string> m_PossibleValues;
		std::vector<std::string> m_AvailableValues;
		std::string m_Value;
	};

	enum ATTRTYPE_E
	{
		SINGLESELECT = 0,
		MULTISELECT = 1,
		BOOLEANSELECT = 2,
		EDIT = 3,
		STATIC = 4
	};
}