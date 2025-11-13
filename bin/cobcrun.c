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
static const unsigned char cp819_to_cp1047[256] =
  "\x00\x01\x02\x03\x37\x2D\x2E\x2F\x16\x05\x25\x0B\x0C\x0D\x0E\x0F" /*00-0F*/
  "\x10\x11\x12\x13\x3C\x3D\x32\x26\x18\x19\x3F\x27\x1C\x1D\x1E\x1F" /*10-1F*/
  "\x40\x4F\x7F\x7B\x5B\x6C\x50\x7D\x4D\x5D\x5C\x4E\x6B\x60\x4B\x61" /*20-2F*/
  "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\x7A\x5E\x4C\x7E\x6E\x6F" /*30-3F*/
  "\x7C\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xD1\xD2\xD3\xD4\xD5\xD6" /*40-4F*/
  "\xD7\xD8\xD9\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xAD\xE0\xBD\x5A\x6D" /*50-5F*/
  "\x79\x81\x82\x83\x84\x85\x86\x87\x88\x89\x91\x92\x93\x94\x95\x96" /*60-6F*/
  "\x97\x98\x99\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xC0\x4A\xBB\xD0\xA1" /*70-7F*/
  "\x07\x20\x21\x22\x23\x24\x15\x06\x17\x28\x29\x2A\x2B\x2C\x09\x0A" /*80-8F*/
  "\x1B\x30\x31\x1A\x33\x34\x35\x36\x08\x38\x39\x3A\x3B\x04\x14\x3E" /*90-9F*/
  "\xFF\x41\xAA\xB0\xB1\x9F\xB2\xD0\xA0\xB5\x79\xB4\x9A\x8A\xBA\xCA" /*A0-AF*/
  "\xAF\xA8\xB7\xB8\xB9\xAB\x64\x65\x62\x66\x63\x67\x9E\x68\x74\x71" /*B0-BF*/
  "\x72\x73\x78\x75\x76\x77\xAC\x69\xED\xEE\xEB\xEF\xEC\xBF\x80\xBC" /*C0-CF*/
  "\x90\x8F\xEA\xFA\xBE\xA7\x8C\x8B\xB6\xA9\xB3\xA6\xB8\xB0\xB1\xB2" /*D0-DF*/
  "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xD1\xD2\xD3\xD4\xD5\xD6" /*E0-EF*/
  "\xD7\xD8\xD9\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xAD\xE0\xBD\x5A\x6D" /*F0-FF*/
  ;

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
