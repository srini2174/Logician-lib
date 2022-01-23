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
/**
* Numerics.h
http://www.daniweb.com/code/snippet993.html
 */
#include "stdafx.h"
#include <sstream>
#include <string>

using std::stringstream;
using std::string;

/**
 * Convenience class for enabling easy
 * conversions between strings and primitive types.
 *
 * This class is not meant to support non-primitive types,
 * but it should if target class Type has istream and ostream
 * operators overloaded.
 */
namespace Numerics{

	template<class T>
	class Numerical{
	
		private:
			T number;

		public:
			Numerical(T value = 0) : number(value){}
			Numerical(const string& arg){
				const_cast<Numerical&>(*this)(arg);
			}

			/**
			 * Attempts to assign the argument value to the value
			 * of this Object's type.
			 * If the value is invalid, nothing happens.
			 */
			string operator()(const string& arg){
				stringstream ss (stringstream::in | stringstream::out);
				try{
					ss << arg;
					ss >> number;
				}catch(...){
					// currently unsupported
				}
				return ss.str();
			}

			/**
			 * Attempts to assign the argument value to the value of
			 * this Object's type.
			 */
			T operator()(T value){
				return (number = value);
			}

			/**
			 * Returns a string representation of this Object's number
			 */
			string getString(){
				stringstream ss (stringstream::in | stringstream::out);
				ss << number;
				return ss.str();
			}

			/**
			 * Returns a copy of this Object's number
			 */
			T getValue(){
				return number;
			}

			/**
			 * Extraction operator used to return the underlying value
			 * during operations assosciated with primitive types.
			 */
			operator T& (){
				return number;
			}

			/**
			 * const version of the above operator. Returns a copy
			 * of this Object's number.
			 */
			operator T () const{
				return number;
			}
	};

	/* Some meaningful typedefs for Numerical types */
	typedef Numerical<short> Short;
	typedef Numerical<unsigned short> U_Short;
	typedef Numerical<int> Integer;
	typedef Numerical<unsigned int> U_Integer;
	typedef Numerical<double> Double;
	typedef Numerical<float> Float;
	typedef Numerical<char> Byte;
	typedef Numerical<unsigned char> U_Byte;
	typedef Numerical<wchar_t> Wide_Byte;
	typedef Numerical<long int> Long;
	typedef Numerical<unsigned long int> U_Long;

	/* For non-standard types, like __int8, __int16, __int32, and __int64 */
	#ifdef ALLOW_NONSTANDARD_PRIMITIVE_TYPES

		#if (ALLOW_NONSTANDARD_PRIMITIVE_TYPES == 0x01)
			typedef Numerical < __int8 > __Int8;
			typedef Numerical < unsigned __int8 > U___Int8;
			typedef Numerical < __int16 > __Int16;
			typedef Numerical < unsigned __int16 > U___Int16;
			typedef Numerical < __int32 > __Int32;
			typedef Numerical < unsigned __int32 > U___Int32;
			typedef Numerical < __int64 > __Int64;
			typedef Numerical < unsigned __int64 > U___Int64;
		#endif

	#endif
}


/**
 * DriverProgram.cpp


 ****************************************
 * @Author: Mark Alexander Edwards Jr.
 *
 * This program uses a utility class to solve
 * complex equations represented by string 
 * expressions
 ****************************************
#include <iostream>
#include <ctime>
#include <string>
#include "Numerics.cpp"
#include "EquationSolver.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using namespace Numerics;
using namespace EquationHelper;

int main(){
	
	cout << ES::solve("5 + 3 * (8 - 4)") << endl << endl;
	cout << ES::solve("12 / (3 * 4) + 5") << endl << endl;

	while(true){
		cout << "Enter an equation you would \nlike to solve (enter 'exit' to quit): " << flush;
		try{
			string temp;
			getline(cin, temp);
			if(temp.compare("exit") == 0) exit(0);
			clock_t t1 = clock();
			cout << "Answer: " + ES::solve(temp, 4) << endl;
			clock_t t2 = clock();
			cout << "\nTime taken to calculate value: " <<
				(t2-t1) << " Clock cycles" <<endl;
		}catch(...){
			cout << "Invalid expression! Please try again!" << endl;
		}
	}

	cin.ignore();
	cin.get();
	return 0;
}
*/