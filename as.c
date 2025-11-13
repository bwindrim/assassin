/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *	as.c - main module	*
 *				*
 ********************************/

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>

#include "const.h"
#include "err.h"
#include "typedefs.h"
#include "globals.h"
#include "parseopt.h"
#include "symbol.h"
#include "gen.h"
#include "recog.h"

struct opt_rec table [] =
{
    { "-l", noarg_opt, (char_ptr) &list_flag },
    { "-x", noarg_opt, (char_ptr) &exp_flag  },
    { "-n", noarg_opt, (char_ptr) &gen_flag  },
    { "-d", noarg_opt, (char_ptr) &debug     },
    { "-v", noarg_opt, (char_ptr) &verbose   },
    { "-o", starg_opt, (char_ptr) &obj_name  }
};

#define TABLE_SIZE (sizeof(table)/sizeof(table[0]))

int	main (int argc, char *argv[])
{
  static char **argp;
  static int  argcount;
/*
  static outbuffer [16],
	 errbuffer [16];
*/

    /* parse command-line options */
    if (-1 == parse_opt (table, TABLE_SIZE, &argc, argv))
    {
	fprintf (stderr, "%s:bad options\n", argv [0]);
    }

  /* check if destination of stderr is same as stdout *
  fname (stdout, outbuffer);
  fname (stderr, errbuffer);

  out_diff = strcmp (outbuffer, errbuffer);
*/

  out_diff = TRUE;

  if (1 == argc)
    {
      gen_init (0);

      list = TRUE;
      parse_stream (stdin, "STDIN");
    }
  else
    {
      for (pass = 1;
	   pass <= 2;
	   ++pass)
	{
	  argp      = argv;
	  argcount  = argc;
	  err_count = 0;
	  lcount    = 0;

	  if (verbose)
	  {
		  printf("Pass %d\n", pass);
	  
		if (1 == pass)
			printf ("%s%s", TITLE_STR, list_flag ? "\n\n" : "\n");
	  }

	  if (2 == pass)
	    {
	      gen_open (obj_name ? obj_name : argp [1]);
	      list = list_flag;
	      gen  = !gen_flag;
	      block_ptr [1] = NULL;
	    }

	  gen_init (0);

	  while (--argcount)
	    {
	      parse_file (*++argp);
	    }

	  if (block_ptr != (block_stack + 1))
	    {
	      fprintf (stderr, errfrmstr, errtext [txtenderr]);
	      exit (2);
	    }

          if (2 == pass)
	    {
		gen_close ();

	      printf ("%sAssembly complete, %d lines",
		      list_flag ? "\n" : "",
		      lcount
		     );

	      if (err_count)
		  printf (", %d error%s.\n",
			  err_count, (err_count == 1) ? "" : "s"
			 );
	      else
		  printf (", no errors.\n");

	      if (err_count)
		exit (1);
	    }
	}
    }
}


