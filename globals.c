/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   globals.c - decl'n module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>

#include "const.h"
#include "typedefs.h"
#include "globals.h"

bool	forward_sym,
	gen = FALSE,
	list = FALSE,
	out_diff;

int	gen_flag = FALSE,
	list_flag = FALSE,
	exp_flag = FALSE,
	debug = FALSE;

char	*obj_name = NULL;

char	errfrmstr  [] = "** Error -  %s.\n",
        err2frmstr [] = "\"%s\", line %d,  error -  %s.\n",
	filerrstr  [] = "** Error -  Can't open %s for %s.\n";

int		val,
		err_count = 0,
		pass	  = 0,
		lcount    = 0;

unsigned	genaddr, nextaddr,
		load_addr   = 0,
		exe_addr    = 0,
		code_length = 0;

int		error, hack_error;

jmp_buf		termbuf;


