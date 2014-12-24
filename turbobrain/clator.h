/*
 * This file is part of 'turbobrain'.
 *
 * 'turbobrain' is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 'turbobrain' is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 'turbobrain'. If not, see http://www.gnu.org/licenses/.
 */

#ifndef turbobrain_clator_h
#define turbobrain_clator_h

#include <string>
#include <unordered_map>
#include <sstream>

using namespace std;

// Translates Brainfuck code into C code using this map:
// '>' to '++ptr'
// '<' to '--ptr'
// '+' to '++*ptr'
// '-' to '--*ptr'
// '.' to 'printf("%c", *ptr)'
// ',' to 'scanf("%c", ptr)'
// '[' to 'while(*ptr) {'
// ']' to '}'
class clator {
	
	public:
	clator(const string &program) {
		
		// strip all non-standard characters since we don't care about them
		for (auto it = program.begin(); it != program.end(); it++) {
			if (_basic_assoc.find(*it) != _basic_assoc.end())
				_program.push_back(*it);
		}
		
		_unbalanced = false;
		if (_translate_to_c() != 0)
			_unbalanced = true;
	}
	
	int compile_to(const string &path,
				   const string &oargs = string(),
				   const string &comp = string("gcc")) {
		
		// Create a .c file containing the generated C program, and send
		// this file as the input file to the specified compiler
		string tmp_cpath = "turbobrain";
		time_t timev = time(nullptr);
		stringstream datestream;
		datestream << timev;
		tmp_cpath += datestream.str();
		tmp_cpath += ".c";
		
		ofstream cprog_file(tmp_cpath);
		if (!cprog_file.is_open())
			return -1;
		
		cprog_file << _cprog;
		cprog_file.close();
		
		stringstream ss;
		ss << comp << " " << tmp_cpath << " -o " << path << " " << oargs;
		cout << ss.str() << endl;
		
		if (!system(NULL)) {
			cerr << "clator: error: processor is unavailable" << endl;
			return -1;
		}
		int rv = system(ss.str().c_str());
		
		// do not forget to delete the old .c file
		remove(tmp_cpath.c_str());
		
		return rv;
	}
	
	const string &get_brainfuck() { return _program; }
	const string &get_c() { return _cprog; }
	bool ok() { return !_unbalanced; }
	
	private:
	int _translate_to_c() {
		
		string prefix(""
			#include "prefix/prefix"
		);
		
		stringstream ss;
		int level = 0;
		int lspace = 4; // # of spaces in a level
		
		ss << prefix << endl;
		
		// __turbobrain_main() is the start point of the program
		ss << "int __turbobrain_main() {" << endl;
		level++;
		
		for (auto it = _program.begin(); it != _program.end(); it++) {
			
			if (*it == ']')
				level--;
			
			ss << _setw(level * lspace) << _basic_assoc[*it] << endl;
			
			if (*it == '[')
				level++;
		}
		
		ss << _setw(level * lspace) << "return 0;" << endl;
		
		ss << "}" << endl; // closing bracket of __turbobrain_main()
		level--;
		
		_cprog = ss.str();
		
		// return 1 if the brackets are not balanced
		return level == 0 ? 0 : 1;
	}
	
	string _setw(int w) {
		return string(w, ' ');
	}
	
	string _program;
	string _cprog;
	bool _unbalanced;
	static unordered_map<char, string> _basic_assoc;
};

unordered_map<char, string> clator::_basic_assoc = {
	{'>', "++p;"},
	{'<', "--p;"},
	{'+', "++*p;"},
	{'-', "--*p;"},
	{'.', "printf(\"%c\", *p);"},
	{',', "scanf(\"%c\", p);"},
	{'[', "while(*p) {"},
	{']', "}"}
};

#endif
