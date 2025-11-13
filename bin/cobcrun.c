/*
   Copyright (C) 2004-2012, 2014-2024 Free Software Foundation, Inc.
   Written by Roger While, Simon Sobisch, Brian Tiffin

   This file is part of GnuCOBOL.

   The GnuCOBOL module loader is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GnuCOBOL is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GnuCOBOL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tarstamp.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef	HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "../libcob/common.h"
#include "../libcob/cobgetopt.h"

static int arg_shift = 1;
static int print_runtime_wanted = 0;
static signed int	verbose_output = 0;
#define MAX_EXEC_PARM_TEXT_SIZE 100
static int exec_parm_wanted = 0;
static struct {
	unsigned char length[2];
	unsigned char text[MAX_EXEC_PARM_TEXT_SIZE];
} __attribute__((packed)) exec_parm;
static const int exec_parm_argc = 1;
static char *exec_parm_argv[1] = { (char *)&exec_parm };

static const char short_options[] = "+hirc:VvqM:p:";

#define	CB_NO_ARG	no_argument
#define	CB_RQ_ARG	required_argument
#define	CB_OP_ARG	optional_argument

static const struct option long_options[] = {
	{"help",		CB_NO_ARG, NULL, 'h'},
	{"version",   		CB_NO_ARG, NULL, 'V'},
	{"verbose",		CB_NO_ARG, NULL, 'v'},
	{"brief",		CB_NO_ARG, NULL, 'q'},
	{"info",		CB_NO_ARG, NULL, 'i'},
	{"dumpversion",		CB_NO_ARG, NULL, '~'},	/* format: GCC dumpfullversion */
	{"runtime-config",		CB_NO_ARG, NULL, 'r'},
	{"config",		CB_RQ_ARG, NULL, 'C'},
	{"module",		CB_RQ_ARG, NULL, 'm'},
	{"exec-parm",		CB_RQ_ARG, NULL, 'p'},
	{NULL, 0, NULL, 0}
};

#ifdef ENABLE_NLS
#include "gettext.h"	/* from lib/ */
#define _(s)		gettext(s)
#define N_(s)		gettext_noop(s)
#else
#define _(s)		s
#define N_(s)		s
#endif

/**
 * Conversion table from ISO 8859-1 (CP819) to EBCDIC codepage 1047
 */
static const unsigned char cp819_to_cp1047[256] = {
  /* 0x00-0x0F */
  0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  /* 0x10-0x1F */
  0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
  /* 0x20-0x2F */
  0x40, 0x4F, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
  /* 0x30-0x3F */
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
  /* 0x40-0x4F */
  0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
  /* 0x50-0x5F */
  0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5A, 0x6D,
  /* 0x60-0x6F */
  0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
  /* 0x70-0x7F */
  0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4A, 0xBB, 0xD0, 0xA1,
  /* 0x80-0x8F */
  0x07, 0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A,
  /* 0x90-0x9F */
  0x1B, 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E,
  /* 0xA0-0xAF */
  0xFF, 0x41, 0xAA, 0xB0, 0xB1, 0x9F, 0xB2, 0xD0, 0xA0, 0xB5, 0x79, 0xB4, 0x9A, 0x8A, 0xBA, 0xCA,
  /* 0xB0-0xBF */
  0xAF, 0xA8, 0xB7, 0xB8, 0xB9, 0xAB, 0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68, 0x74, 0x71,
  /* 0xC0-0xCF */
  0x72, 0x73, 0x78, 0x75, 0x76, 0x77, 0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF, 0x80, 0xBC,
  /* 0xD0-0xDF */
  0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA7, 0x8C, 0x8B, 0xB6, 0xA9, 0xB3, 0xA6, 0xB8, 0xB0, 0xB1, 0xB2,
  /* 0xE0-0xEF */
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
  /* 0xF0-0xFF */
  0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5A, 0x6D
};

/**
 * Display cobcrun version info, optional with build and version date
 */
static void
cobcrun_print_version (void)
{
	printf ("cobcrun (%s) %s.%d\n",
		PACKAGE_NAME, PACKAGE_VERSION, PATCH_LEVEL);
	puts ("Copyright (C) 2024 Free Software Foundation, Inc.");
	printf (_("License GPLv3+: GNU GPL version 3 or later <%s>"),
		"https://gnu.org/licenses/gpl.html");
	putchar ('\n');
	puts (_("This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
	putchar ('\n');
	printf (_("Written by %s"), "Roger While, Simon Sobisch, Brian Tiffin");
	putchar ('\n');
	if (verbose_output) {
		char	cob_build_stamp[COB_MINI_BUFF];
		char	month[64];
		int status, day, year;

		/* Set up build time stamp */
		memset (cob_build_stamp, 0, (size_t)COB_MINI_BUFF);
		memset (month, 0, sizeof(month));
		day = 0;
		year = 0;
		status = sscanf (__DATE__, "%63s %d %d", month, &day, &year);
		/* LCOV_EXCL_START */
		if (status != 3) {
			snprintf (cob_build_stamp, (size_t)COB_MINI_MAX,
				"%s %s", __DATE__, __TIME__);
		/* LCOV_EXCL_STOP */
		} else {
			snprintf (cob_build_stamp, (size_t)COB_MINI_MAX,
				"%s %2.2d %4.4d %s", month, day, year, __TIME__);
		}
		printf (_("Built     %s"), cob_build_stamp);
		putchar ('\n');
		printf (_("Packaged  %s"), COB_TAR_DATE);
		putchar ('\n');
	}
}

/**
 * Display cobcrun help
 */
static void
cobcrun_print_usage (char * prog)
{
	puts (_("GnuCOBOL module loader"));
	putchar ('\n');
	printf (_("Usage: %s [options] PROGRAM [parameter ...]"), prog);
	putchar ('\n');
	printf (_("  or:  %s options"), prog);
	putchar ('\n');
	putchar ('\n');
	puts (_("Options:"));
	puts (_("  -h, --help                      display this help and exit"));
	puts (_("  -V, --version                   display version information for cobcrun + runtime and exit"));
	puts (_("  -dumpversion                    display runtime version and exit"));
	puts (_("  -i, --info                      display runtime information (build/environment)"));
	puts (_("  -v, --verbose                   display extended output with --info"));
#if 0 /* Simon: currently only removing the path from cobcrun in output --> don't show */
	puts (_("  -q, --brief                     reduced displays"));
#endif
	puts (_("  -c <file>, --config=<file>      set runtime configuration from <file>"));
	puts (_("  -r, --runtime-config            display current runtime configuration\n"
	        "                                  (value and origin for all settings)"));
	puts (_("  -M <module>, --module=<module>  set entry point module name and/or load path\n"
			"                                  where -M module prepends any directory to the\n"
			"                                  dynamic link loader library search path\n"
			"                                  and any basename to the module preload list\n"
			"                                  (COB_LIBRARY_PATH and/or COB_PRELOAD)"));
	puts (_("  -p \"<parameter>\", --exec-parm=\"<parameter>\"  set parameter string to be\n"
			"                                  passed to the program in EBCDIC preceded by a 2-byte\n"
			"                                  big-endian length field. The length does not include\n"
			"                                  the length of the length field itself."));
	putchar ('\n');
#ifndef PACKAGE_BUGREPORT_URL
	printf (_("Report bugs to: %s\n"
	          "or (preferably) use the issue tracker via the home page."),
			PACKAGE_BUGREPORT);
	putchar ('\n');
#else
	puts (_("For bug reporting instructions, please see:"));
	printf ("%s.\n", PACKAGE_BUGREPORT_URL);
#endif
	printf (_("GnuCOBOL home page: <%s>"),
		"https://www.gnu.org/software/gnucobol/");
	putchar ('\n');
	printf (_("General help using GNU software: <%s>"),
		"https://www.gnu.org/gethelp/");
	putchar ('\n');
}

/**
 * split into path and file, or just path, or just file
 * returns allocated strings (possible emtpy) for both
 *  Note: cob_free must be called with *pathname and *filename
 *        for releasing memory after use
 */
static void
cobcrun_split_path_file (char** pathname, char** filename, char *pf)
{
	char *pos = pf;
	char *next_pos;

	char sav;

	/* set pos to last slash (if any) */
	while ((next_pos = strpbrk (pos + 1, "\\/")) != NULL) {
		pos = next_pos;
	}
	/* copy string up to last slash as pathname (possible emtpy) */
	sav = *pos;
	*pos = 0;
	*pathname = cob_strdup (pf);
	*pos = sav;

	/* set pos to first character after last slash (if any) */
	if (pf != pos) {
		pos++;
	}

	/* copy string after last slash as filename (possible emtpy) */
	*filename = cob_strdup (pos);
}

/**
 * Prepend a new directory path to the library search COB_LIBRARY_PATH
 * and setup a module COB_PRE_LOAD, for each component included.
 */
static const char *
cobcrun_initial_module (char *module_argument)
{
	char	*pathname, *filename;
	char	env_space[COB_MEDIUM_BUFF], *envptr;

	/* FIXME: split in two functions (one setting module, one setting path)
	          after allowing module with path in COB_PRE_LOAD */

	/* note: getopt ensures that we have an argument, but it may be empty */
	if (module_argument[0] == 0) {
		return "";	/* used as "no further information" */
	}

#if 0	/* CHECKME: Do we want that validation here or handle it? */
	if (strchr (module_argument, PATHSEP_CHAR)) {
		static char [COB_MINI_BUFF] buff;
		snprintf (buff, COB_MINI_MAX, _("should not contain '%c'"), PATHSEP_CHAR);
		return buff;
	}
#endif

	/* See if we have a /dir/path/module, or a /dir/path/ or a module (no slash) */
	cobcrun_split_path_file (&pathname, &filename, module_argument);
	if (*pathname) {
		/* TODO: check content, see libcob/common.c/h to raise error message */
		envptr = cob_getenv_direct ("COB_LIBRARY_PATH");
		if (envptr
		 && strlen (envptr) + strlen (pathname) + 1 < COB_MEDIUM_MAX) {
			memset (env_space, 0, COB_MEDIUM_BUFF);
			snprintf (env_space, COB_MEDIUM_MAX, "%s%c%s",
				pathname, PATHSEP_CHAR, envptr);
			env_space[COB_MEDIUM_MAX] = 0; /* fixing code analyser warning */
			(void) cob_setenv ("COB_LIBRARY_PATH", env_space, 1);
		} else {
			(void) cob_setenv ("COB_LIBRARY_PATH", pathname, 1);
		}
	}
	cob_free((void *)pathname);

	if (*filename) {
		/* TODO: check content, see libcob/common.c/h to raise error message */
		envptr = cob_getenv_direct ("COB_PRE_LOAD");
		if (envptr
		 && strlen (envptr) + strlen (filename) + 1 < COB_MEDIUM_MAX) {
			memset (env_space, 0, COB_MEDIUM_BUFF);
			snprintf (env_space, COB_MEDIUM_MAX, "%s%c%s", filename,
				PATHSEP_CHAR, envptr);
			env_space[COB_MEDIUM_MAX] = 0; /* fixing code analyser warning */
			(void) cob_setenv ("COB_PRE_LOAD", env_space, 1);
		} else {
			(void) cob_setenv ("COB_PRE_LOAD", filename, 1);
		}
	}
	cob_free ((void *)filename);
	return NULL;
}

/**
 * process the cobcrun command options
 */
static void
process_command_line (int argc, char *argv[])
{
	int			c, idx;
	const char		*err_msg;
	
#if defined (_WIN32) || defined (__DJGPP__)
	if (!cob_getenv_direct ("POSIXLY_CORRECT")) {
		/* Translate command line arguments from DOS/WIN to UNIX style */
		int argnum = 0;
		while (++argnum < argc) {
			if (strrchr(argv[argnum], '/') == argv[argnum]) {
				if (argv[argnum][1] == '?' && !argv[argnum][2]) {
					argv[argnum] = (char *)"--help";
					continue;
				}
				argv[argnum][0] = '-';
			}
		}
	}
#endif

	/* c = -1 if idx > argc or argv[idx] has non-option */
	while ((c = cob_getopt_long_long (argc, argv, short_options,
					  long_options, &idx, 1)) >= 0) {
		switch (c) {
		case '?':
			/* Unknown option or ambiguous */
			exit (EXIT_FAILURE);

		case 'c':
		case 'C':
			/* -c <file>, --config=<file> */
			/* LCOV_EXCL_START */
			if (cob_optarg[0] == 0
			 || strlen (cob_optarg) > COB_SMALL_MAX) {
				fputs (_("invalid configuration file name"), stderr);
				putc ('\n', stderr);
				fflush (stderr);
				exit (EXIT_FAILURE);
			}
			/* LCOV_EXCL_STOP */
			arg_shift++;
			(void) cob_setenv ("COB_RUNTIME_CONFIG", cob_optarg, 1);
			/* shift argument again if two part argument was used */
			if (c == 'c') {
				arg_shift++;
			}
			break;

		case 'h':
			/* --help */
			cobcrun_print_usage (argv[0]);
			exit (EXIT_SUCCESS);

		case 'i':
			/* --info */
			print_info_detailed (verbose_output);
			exit (EXIT_SUCCESS);

		case 'q':
			/* --brief : reduced reporting */
			/* resets -verbose and removes the path to cobcrun in argv[0] */
			verbose_output = 0;
			strcpy (argv[0], "cobcrun");	/* set for simple compare in test suite
										   and other static output */
			arg_shift++;
			break;

		case 'v':
			/* --verbose : Verbose reporting */
			verbose_output++;
			arg_shift++;
			break;

		case 'r':
			/* --runtime-conf */
			print_runtime_wanted = 1;
			arg_shift++;
			break;

		case 'V':
			/* --version */
			cobcrun_print_version ();
			putchar ('\n');
			print_version ();
			if (verbose_output) {
				putchar ('\n');
				print_version_summary ();
			}
			exit (EXIT_SUCCESS);

		case '~':
			/* -dumpversion */
			puts (libcob_version());
			exit (EXIT_SUCCESS);

		case 'M':
		case 'm':
			/* -M <module>, --module=<module> */
			arg_shift++;
			err_msg = cobcrun_initial_module (cob_optarg);
			if (err_msg != NULL) {
				fprintf (stderr, _("invalid module argument '%s'"), cob_optarg);
				if (err_msg[0]) {
					fprintf (stderr, "; %s\n", err_msg);
				} else {
					fputc ('\n', stderr);
				}
				fflush (stderr);
				exit (EXIT_FAILURE);
			}
			/* shift argument again if two part argument was used */
			if (c == 'M') {
				arg_shift++;
			}
			break;

		case 'p':
			/* -p <parameter>, --exec-parm=<parameter> */
			exec_parm_wanted = 1;
			arg_shift++;
			if (strlen (cob_optarg) > sizeof(exec_parm.text)) {
				fprintf (stderr, _("exec-parm parameter is too long"));
				putc ('\n', stderr);
				fflush (stderr);
				exit (EXIT_FAILURE);
			}
			for (int i = 0; i < strlen (cob_optarg); i++) {
				exec_parm.text[i] = cp819_to_cp1047[(unsigned char)cob_optarg[i]];
			}
			exec_parm.length[0] = (unsigned char)(strlen (cob_optarg) >> 8);
			exec_parm.length[1] = (unsigned char)(strlen (cob_optarg) & 0xFF);
			arg_shift++;
			break;

		/* LCOV_EXCL_START */
		default:
			/* not translated as it is an unlikely internal error: */
			fprintf (stderr, "missing evaluation of command line option '%c'", c);
			putc ('\n', stderr);
			fputs (_("Please report this!"), stderr);
			fflush (stderr);
			exit (EXIT_FAILURE);
		/* LCOV_EXCL_STOP */

		}
	}
}

/**
 * cobcrun, for invoking entry points from dynamic shared object libraries
 */
int
main (int argc, char **argv)
{
	cob_call_union	unifunc;

#ifdef	HAVE_SETLOCALE
	setlocale (LC_ALL, "");
#endif

	/* minimal initialization of the environment like binding textdomain,
	   allowing test to be run under WIN32 (implied in cob_init(),
	   no need to call outside of GnuCOBOL) */
	cob_common_init (NULL);

	process_command_line (argc, argv);

	/* At least one option or module name needed */
	if (argc <= arg_shift) {
		if (print_runtime_wanted) {
			cob_init_nomain (0, &argv[0]);
			print_runtime_conf ();
			cob_stop_run (EXIT_SUCCESS);
		}
		fprintf (stderr, _("%s: missing PROGRAM name"), argv[0]);
		putc ('\n', stderr);
		fprintf (stderr, _("Try '%s --help' for more information."), argv[0]);
		putc ('\n', stderr);
		fflush (stderr);
		return 1;
	}

	/* No other program arguments are allowed if exec-parm is specified */
	if (exec_parm_wanted && argc > arg_shift + 1) {
		fprintf (stderr, _("Additional arguments are not allowed when exec-parm is specified"));
		putc ('\n', stderr);
		fflush (stderr);
		return 1;
	}

	/* Initialize the COBOL system, ... */
	/* Note: we use cob_init_nomain here as there are no functions
	         linked here we want to provide for the COBOL environment */
	if (exec_parm_wanted) {
		cob_init_nomain (exec_parm_argc, exec_parm_argv);
	} else {
		cob_init_nomain (argc - arg_shift, &argv[arg_shift]);
	}
	if (print_runtime_wanted) {
		print_runtime_conf ();
		putc ('\n', stdout);
	}
	/* ... verify and resolve the PROGRAM name, ... */
	/* Note: cob_resolve_cobol takes care for call errors,
	   because of the last parameter; no need to check here afterwards;
	   another program may use "0" and check for function pointer != NULL */
	unifunc.funcvoid = cob_resolve_cobol (argv[arg_shift], 0, 1);
	
	/* ... then invoke, wrapped in a STOP RUN */
	/* Note:  we requested a program exit if resolving had issues,
	          so are only still running if we have a a valid, _likely_ COBOL
	          function to execute */
	if (exec_parm_wanted) {
		cob_stop_run (unifunc.funcint(&exec_parm));
	} else {
		cob_stop_run (unifunc.funcint());
	}
}
