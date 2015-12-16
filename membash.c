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

#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include <sys/time.h>

#include "src/argconfig.h"
#include "src/suffix.h"
#include "src/report.h"

struct membash {
	unsigned      *mem;
	size_t        size;
	unsigned      seed;
	unsigned      verbose;

	struct timeval          start_time;
	struct timeval          end_time;	
};

static const struct membash defaults = {
	.mem        = NULL,
	.size       = 1024,
	.seed       = 0,
	.verbose    = 0,
};

const char program_desc[] =
	"Simple memory tester program";

static const struct argconfig_commandline_options command_line_options[] = {
	{"s",             "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument, NULL},
	{"size",          "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument,
	 "block size to use"},
	{"seed",          "NUM", CFG_LONG_SUFFIX, &defaults.seed, required_argument,
	 "random seed to use for data (set to 0 for auto-gen seed)"},
	{"v",             "", CFG_NONE, &defaults.verbose, no_argument, NULL},
	{"verbose",       "", CFG_NONE, &defaults.verbose, no_argument,
	 "be verbose"},
	{0}
};

static int setup(struct membash *m)
{
	unsigned sum = 0;

	m->mem = malloc(m->size*sizeof(unsigned));
	if (m->mem == NULL){
		fprintf(stderr,"could not allocate for mem!\n");
		exit(1);
	}
		
	
	gettimeofday(&m->start_time, NULL);
	for (unsigned i=0; i<m->size-1; i++) {
		m->mem[i] = (char)rand();
		sum += m->mem[i];
	}
	m->mem[m->size-1] = UINT_MAX - sum + 1;

	gettimeofday(&m->end_time, NULL);
	fprintf(stdout, "Wrote: ");
	report_transfer_rate(stdout, &m->start_time,
			     &m->end_time, m->size*sizeof(unsigned));
	fprintf(stdout, "\n");

	return 0;
}

static int run(struct membash *m)
{
	unsigned sum = 0;

	gettimeofday(&m->start_time, NULL);
	for (unsigned i=0; i<m->size; i++)
		sum += m->mem[i];

	if ( sum ){
		fprintf(stderr,"sum did not add to zero (%u)!\n",
			sum);
		exit(1);
	}
		
	gettimeofday(&m->end_time, NULL);
	fprintf(stdout, "Read : ");
	report_transfer_rate(stdout, &m->start_time,
			     &m->end_time, m->size*sizeof(unsigned));
	fprintf(stdout, "\n");

	return 0;
}

int main(int argc, char **argv)
{
	struct membash cfg;
	
	int args = argconfig_parse(argc, argv, program_desc, command_line_options,
				   &defaults, &cfg, sizeof(cfg));
	if (args != 0) {
		argconfig_print_help(argv[0], program_desc, command_line_options);
		return 1;
	}

	if (cfg.seed==0)
		cfg.seed = time(NULL);
	srand(cfg.seed);

	setup(&cfg);
	run(&cfg);
	
	return 0;
}
