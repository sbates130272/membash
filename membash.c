////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 PMC-Sierra, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
//   Author: Stephen Bates
//
//   Date:   Dec 16 2015
//
//   Description:
//     Program for testing memory access 
//
////////////////////////////////////////////////////////////////////////

#include "src/argconfig.h"
#include "src/suffix.h"
#include "src/report.h"

struct membash {
	void          *mem;
	size_t         size;
	unsigned      verbose;
	
};

static const struct membash defaults = {
	.mem        = NULL,
	.size       = 1024,
	.verbose    = 0,
};


const char program_desc[] =
	"Simple memory tester program";

static const struct argconfig_commandline_options command_line_options[] = {
	{"s",             "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument, NULL},
	{"size",          "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument,
	 "block size to use"},
	{"v",             "", CFG_NONE, &defaults.verbose, no_argument, NULL},
	{"verbose",       "", CFG_NONE, &defaults.verbose, no_argument,
	 "be verbose"},
	{0}
};

int main(int argc, char **argv)
{
	struct membash cfg;
	
	int args = argconfig_parse(argc, argv, program_desc, command_line_options,
				   &defaults, &cfg, sizeof(cfg));
		
	return args;
}
