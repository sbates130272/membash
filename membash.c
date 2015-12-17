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
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/mman.h>

#include "src/argconfig.h"
#include "src/suffix.h"
#include "src/report.h"

typedef unsigned membash_t;

struct membash {
	unsigned      *mem;
	size_t        size;
	size_t        iters;
	unsigned      seed;
    unsigned      fence;
	unsigned      verbose;

    char          *mmap;
    int           mmapfd;

	size_t                  (* hash)(size_t);
    int                     (* run)(struct membash *);

	struct timeval          start_time;
	struct timeval          end_time;
};

static const struct membash defaults = {
	.mem        = NULL,
	.size       = 1024,
	.iters      = 1,
	.seed       = 0,
    .mmap       = NULL,
	.verbose    = 0,
	.hash       = NULL,
};

const char program_desc[] =
	"Simple memory tester program";

static const struct argconfig_commandline_options command_line_options[] = {
	{"s",             "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument, NULL},
	{"size",          "NUM", CFG_LONG_SUFFIX, &defaults.size, required_argument,
	 "block size to use"},
	{"i",             "NUM", CFG_LONG_SUFFIX, &defaults.iters, required_argument, NULL},
	{"iters",         "NUM", CFG_LONG_SUFFIX, &defaults.iters, required_argument,
	 "number of iterations to loop over"},
	{"seed",          "NUM", CFG_LONG_SUFFIX, &defaults.seed, required_argument,
	 "random seed to use for data (set to 0 for auto-gen seed)"},
    {"mmap",          "MMAP", CFG_STRING, &defaults.mmap, required_argument,
            "file to mmap"},
	{"fence",         "", CFG_NONE, &defaults.fence, no_argument,
	 "add a mfence between setup and run"},
	{"v",             "", CFG_NONE, &defaults.verbose, no_argument, NULL},
	{"verbose",       "", CFG_NONE, &defaults.verbose, no_argument,
	 "be verbose"},
	{0}
};

static int setup(struct membash *m)
{
	membash_t sum = 0;

    if ( m->mmap ){

        m->mmapfd = open(m->mmap, O_RDWR);
        m->mem = mmap(NULL, m->size, PROT_WRITE | PROT_READ,
                      MAP_SHARED, m->mmapfd, 0);
    }
    else
        m->mem = malloc(m->size*sizeof(membash_t));

	if (m->mem == NULL){
		fprintf(stderr,"could not allocate for mem!\n");
		exit(1);
	}

	gettimeofday(&m->start_time, NULL);
	for (size_t i=0; i<m->size-1; i++) {
		m->mem[i] = (char)rand();
		sum += m->mem[i];
	}
	m->mem[m->size-1] = UINT_MAX - sum + 1;

	gettimeofday(&m->end_time, NULL);
	fprintf(stdout, "Wrote         : ");
	report_transfer_rate(stdout, &m->start_time,
			     &m->end_time, m->size*sizeof(membash_t));
	fprintf(stdout, "\n");

	return 0;
}

static int run_memcpy(struct membash *m)
{
    membash_t *dest;

    dest = malloc(m->size*sizeof(membash_t));
	if (dest == NULL){
		fprintf(stderr,"could not allocate for dest!\n");
		exit(1);
	}

    gettimeofday(&m->start_time, NULL);
	for (size_t iters=0; iters < m->iters; iters++)
	{
        memcpy(dest,m->mem,m->size*sizeof(membash_t));
	}
	gettimeofday(&m->end_time, NULL);
	fprintf(stdout, "Read (memcpy) : ");
	report_transfer_rate(stdout, &m->start_time,
			     &m->end_time,
			     m->iters*m->size*sizeof(membash_t));
	fprintf(stdout, "\n");

    free(dest);
    return 0;
}

static int run_dumb(struct membash *m)
{
    membash_t sum = 0;

	gettimeofday(&m->start_time, NULL);
	for (size_t iters=0; iters < m->iters; iters++)
	{
		sum = 0;
		for (size_t i=0; i<m->size; i++){
			if ( m->hash )
				sum += m->mem[m->hash(i)];
			else
				sum += m->mem[i];
		}

		if ( sum ){
			fprintf(stderr,"sum did not add to zero (%u)!\n",
				sum);
			exit(1);
            }
	}
	gettimeofday(&m->end_time, NULL);
	fprintf(stdout, "Read (dumb)   : ");
	report_transfer_rate(stdout, &m->start_time,
			     &m->end_time,
			     m->iters*m->size*sizeof(membash_t));
	fprintf(stdout, "\n");

	return 0;
}

static void cleanup(struct membash *m)
{
    if ( m->mmap ){
        munmap(m->mem, m->size);
        close(m->mmapfd);
    }
    else
        free(m->mem);
}

static inline size_t hash_stupid(size_t i)
{
	return i;
}

static inline size_t hash_fast(size_t i)
{
    return i;
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
    if ( cfg.fence )
        asm volatile("mfence" ::: "memory");

	cfg.run  = run_dumb;
    cfg.run(&cfg);
	cfg.hash = hash_stupid;
    cfg.run(&cfg);
	cfg.hash = hash_fast;
	cfg.run(&cfg);

	cfg.run  = run_memcpy;
    cfg.run(&cfg);
	cfg.hash = hash_stupid;
    cfg.run(&cfg);
	cfg.hash = hash_fast;
	cfg.run(&cfg);

    cleanup(&cfg);
	return 0;
}
