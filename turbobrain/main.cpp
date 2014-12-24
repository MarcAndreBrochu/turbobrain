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

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <getopt.h>

#include "ptr.h"
#include "clator.h"

using namespace std;

int verbose_flag = 0;
void verbose(const string &msg) {
	if (verbose_flag)
		cerr << "verbose: " << msg << endl;
}

void print_help() {
	
	cerr << "turbobrain [-v] input [-c comp=gcc][-p opt=None][-o output]" << endl;
	cerr << "-v: enable verbose (debug) mode    [--verbose]" << endl;
	cerr << "-c: specify C compiler             [--compiler]" << endl;
	cerr << "-p: pass extra options to compiler [--options]" << endl;
	cerr << "-o: specify output file            [--output]" << endl;
}

int main(int argc, ptr<char> const *argv) {
	
	string inpath;
	string outpath;
	string compiler = "gcc";
	string extra_options;
	ptr_const<char> options = "v::c:o:p:";
	int c = 0;
	
	if (argc < 2) {
		cerr << "error: no input file" << endl;
		return EXIT_FAILURE;
	}
	
	while (true) {
		
		static struct option long_options[] = {
			
			// These options set a flag
			{"verbose", no_argument, &verbose_flag, 1},
			
			// These set no flag but take an argument
			{"output",  required_argument, 0, 'o'},
			{"compiler",  required_argument, 0, 'c'},
			{"options",  required_argument, 0, 'p'},
			{0, 0, 0, 0}
		};
		
		// getopt_long stores the option index here
		int option_index = 0;
		c = getopt_long_only(argc, argv, options, long_options, &option_index);
		
		// If the options have ended
		if (c == -1)
			break;
		
		switch (c) {
				
			case 0:
				// Do nothing really, for now anyways
				if (long_options[option_index].flag != 0)
					break;
				break;
				
			case 'v':
				verbose_flag = 1;
				break;
				
			case 'o':
				outpath = optarg;
				break;
				
			case 'c':
				compiler = optarg;
				break;
				
			case 'p':
				extra_options = optarg;
				break;
				
			case '?':
				// getopt_long already printed an error message
				break;
				
			default:
				print_help();
				return EXIT_FAILURE;
		}
	}
	
	// The rest of the passed command line are the file to compile and things
	// that can be ignored
	if (optind < argc) {
		
		if (argc - optind > 1) { // more than one file was passed
			verbose("too many files to compile");
			print_help();
			return EXIT_FAILURE;
		}
		
		verbose("file to compile detected");
		verbose("verifying its existence");
		
		// Verify that the file exists on the filesystem
		if (!fopen(argv[optind], "r")) { // C-style file because for this case it's more compact
			cerr << "error: no such file: '" << argv[optind] << "'" << endl;
			return EXIT_FAILURE;
		}
		else {
			verbose("file exists!");
			inpath = argv[optind];
		}
	}
	else {
		verbose("no file to compile");
		print_help();
		return EXIT_FAILURE;
	}
	
	// If no output path is set, the output will be named 'a.out'
	if (outpath.empty())
		outpath = "a.out";
	
	
	// Create a C version of the Brainfuck program and compile it using
	// gcc (by default) or any other specified compiled
	ifstream current_file(inpath);
	string contents;
	string line;
	
	verbose("getting file program");
	while (getline(current_file, line))
		contents += line;
	
	verbose("mapping brainfuck to C");
	clator make = clator(contents);
	if (!make.ok()) {
		cerr << "error: clator failed! Are your brackets matching?" << endl;
		return EXIT_FAILURE;
	}
	
	verbose("compiling C to executable");
	int rv = make.compile_to(outpath, extra_options, compiler);
	if (rv != 0) {
		cerr << "clator: error: " << compiler << " failed with internal " << rv << endl;
		return EXIT_FAILURE;
	}
	
	return 0;
}
