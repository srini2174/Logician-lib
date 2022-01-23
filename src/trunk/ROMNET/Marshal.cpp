/*
This file is part of ROMNET.
Copyright (C) 2009-2015 Eric D. Schmidt, DigiRule Solutions LLC

    ROMNET is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    ROMNET is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ROMNET.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "Marshal.h"

string MarshalString ( String ^ s) {
   using namespace Runtime::InteropServices;
   const wchar_t* chars = 
      (const wchar_t*)(Marshal::StringToHGlobalUni(s)).ToPointer();
   string retval = ROMUTIL::Narrow(chars);
   Marshal::FreeHGlobal(IntPtr((void*)chars));
   return retval;
}

stdext::hash_map<string, size_t> MarshalDictionaryStringUInt (Dictionary<String^, size_t>^ dict)
{
	stdext::hash_map<string, size_t> retval;
	for each(KeyValuePair<String^, size_t> kvp in dict)
	{		
		string key = MarshalString(kvp.Key);
		retval[key] = kvp.Value;
	}
	return retval;
}

cli::array<String^>^ GetArrayFromVectorStrings(const vector<string> &vect)
{
	cli::array<String^>^ arr = gcnew cli::array<String^>(vect.size());
	for (size_t i = 0; i < vect.size(); i++)
	{
		arr[i] = gcnew String(ROMUTIL::Widen(vect[i]).c_str());
	}
	return arr;
}

vector<string> GetVectorFromArrayStrings(cli::array<String^>^ arr)
{
	vector<string> retval;
	for each (String^ s in arr)
	{
		string ws = MarshalString(s);
		retval.push_back(ws);
	}
	return retval;
}

Dictionary<String^, cli::array<String^>^>^ GetDictionaryFromMapStrings(const map<string, vector<string> > &mp)
{
	Dictionary<String^,	cli::array<String^>^>^ dict = gcnew Dictionary<String^, cli::array<String^>^>();

	for (auto it = mp.begin(); it != mp.end(); it++)
	{
		cli::array<String^>^ arr = gcnew cli::array<String^>(it->second.size());
		for (size_t i = 0; i < it->second.size(); i++)
		{
			arr[i] = gcnew String(ROMUTIL::Widen(it->second[i]).c_str());
		}
		String^ key = gcnew String(ROMUTIL::Widen(it->first).c_str());
		dict->Add(key, arr);
	}

	return dict;
}