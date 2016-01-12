/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   defop.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "recog.h"
#include "gen.h"
#include "eval.h"
#include "symbol.h"

#include "defop.h"

void	defb (void)
{
  static char *ptr;

  error = noerror;

  while (NULL != (ptr = next_field ()))
    {
      if (DOUBLEQUOTE == *ptr)
	genstring (strip_quotes (ptr));
      else
        genbyte (eval_str (ptr));
    }
}

void	defw (void)
{
  static char *ptr;

  error = noerror;

  while (NULL != (ptr = next_field ()))
    {
      genword (eval_str (ptr));
    }
}

void	defs (void)
{
  error	      = noerror;
  forward_sym = FALSE;

  val = eval_str (addr_str);

  if (forward_sym)
    error = dfserrstr;

  if (!error)
    nextaddr = genaddr + val;
}

void	redef (void)
{
  error	      = noerror;
  forward_sym = FALSE;

  if (NULL == curr_name)
    error = laberrstr;
  else
    {
      val = eval_str (addr_str);

      if (forward_sym)
        error = dfserrstr;
      else
	{
	  redef_sym (ABS, curr_name, val);
	  curr_name = NULL;
	}
    }
}
