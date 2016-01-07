/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 15/10/89	*
 *				*
 *   macro.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "symbol.h"
#include "recog.h"

#include "macro.h"


void	def_macro (linebuffer, inp, fname, lno)
	char	*linebuffer;
	FILE	*inp;
	char   *fname;
	int     lno;
{
  static char	*next_param, *ptr;
  static char	**param_ptr, **scan_ptr,
		*param [MAX_MACRO_PARAMS] = {"?", "sym", "tail"};
/*  char	buffer [TEXT_BUFF_SIZE];*/
  Line_ptr	curr_line, new_line;
  Obj_ptr	new_mac;


  error = noerror;

  if (NULL == curr_name)
    {
      error = laberrstr;
      outline (linebuffer, fname, lno);
      return;
    }

  for (param_ptr = param + 3;
       param_ptr < param + MAX_MACRO_PARAMS;
       ++param_ptr)
    {
	  if (NULL == (*param_ptr = next_field ()))
	break;

      if (!check_name (*param_ptr))
	{
	  error = paraerrstr;
	  break;
	}
    }

  if (param_ptr >= param + MAX_MACRO_PARAMS)
  {
    error = nparerrstr;
    return;
  }

  new_mac = add_macro (strupper (curr_name), param_ptr - param, *block_ptr);
  curr_line = NULL;


/*if (debug) fprintf (stderr, curr_name);*/


  while (!error)
    {
      outline (linebuffer, fname, lno);

      if (!fgets (linebuffer, TEXT_BUFF_SIZE, inp))
	break;

      ptr = linebuffer;

      if (*ptr != ';' && !isspace (*ptr))
	{
	  while (*ptr && !isspace (*ptr) && (*ptr != COLON))
	    ++ptr;
	}

      skipspace (&ptr);

      if (0 == strncmp (ptr, "endm", 4))
	{
	  if (*linebuffer != ';' && !isspace (*linebuffer))
	    error = endmerrstr;

	  break;
	}

      if (0 == strncmp (ptr, "macro", 5))
	{
	  error = defmerrstr;
	  break;
	}

      if (1 < pass)
	continue;	/* don't do line processing after first pass */

      /* process line for arguments */
      for (ptr = linebuffer;
	   *ptr;
	   ++ptr)
	{
	  if ('?' == *ptr)
	    {
		for (scan_ptr = param;
		     scan_ptr <= param_ptr;
		     ++scan_ptr)
		  if (0 == strncmp (*scan_ptr, ptr + 1, strlen (*scan_ptr)))
		    {
		      *++ptr = '1' + (scan_ptr - param) ;
		      strcpy (ptr + 1, ptr + strlen (*scan_ptr));
		      break;
		    }

	      if ('?' == *ptr)
		{
		  strcpy (ptr, ptr + 1);
		  error = paraerrstr;
		  --ptr;
		}
	    }
	}

      new_line = (Line_ptr) malloc (sizeof (struct macline)
				      + strlen (linebuffer)
				     );
      new_line->next_line = NULL;

      if (NULL == curr_line)
	new_mac->var.mac.lines = new_line;
      else
	curr_line->next_line = new_line;

      curr_line = new_line;

      strcpy (new_line->text, linebuffer);

      error = noerror;
    }

}


Obj_ptr	get_macro (name)
	char	*name;
{
  return find_name_sym (name, strend (name), MACRO);
}


void	exp_macro (macro, fname, lno)
	Obj_ptr	macro;
        char   *fname;
        int     lno;
{
  char	exp_buff [TEXT_BUFF_SIZE];
  char	param_buff [TEXT_BUFF_SIZE];
  char	**param_ptr, *param [MAX_MACRO_PARAMS];
  char	*src, *dst;
  Line_ptr line;
  int	param_count, param_num;
  int	save_list;


  /* set list = exp_flag during macro expansion */
  save_list = list;
  if (list)
      list  = exp_flag;

  dst = param_buff;
  param_ptr = param;


  for (param_ptr = param + 3;
       param_ptr < (param + macro->var.mac.no_params);
       ++param_ptr)
    {
      if (NULL == (src = next_field ()))
        break;

      strcpy (dst, src);
      *param_ptr = dst;
      dst = strend (dst) + 1;
    }

  param [0] = strcpy (dst, "?");
  dst = strend (dst) + 1;

  param_count = param_ptr - param;
  sprintf (dst, "%d", param_count - 3);
  param [1] = dst;
  dst = strend (dst) + 1;

  strcpy (dst, addr_str);
  param [2] = dst;

  for (line = macro->var.mac.lines;
       NULL != line;
       line = line->next_line)
    {
      src = line->text;
      dst = exp_buff;

      while (*src)
	{
	  if ('?' == *src)
	    {
	      param_num = *++src - '1';


	      if (0 <= param_num && param_count > param_num)
		{
		  strcpy (dst, param [param_num]);
		  dst = strend (dst);
		  ++src;
		}
	      else
		{
		  error = parsuperr;
		  break;
		}
	    }
	  else
	    {
	      *dst++ = *src++;
	    }
	}

      *dst = NUL;

      if (error)
	outline (line->text, fname, lno);
      else
	parse (exp_buff, NULL);
    }

  /* restore list_flag */
  list = save_list;
}
