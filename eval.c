/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   eval.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "symbol.h"

#include "eval.h"


int eval_str (str)
	char	*str;
{
static	char	*cut;


  if (str == (cut = strend (str)))
    {
      error = miserrstr;
      return (0);
    }

  if (0 == setjmp (termbuf))
    {
      return (eval (str, cut));
    }
  else
    {
      return (0);
    }
}


int	eval (start, end)
char	*start, *end;
{
  char	*ptr, *cut;
  int	paren, level;

  /* skip leading and trailing spaces, and strip matching brackets */
  while (TRUE)
    {
      /* check for empty expression */
      if (start >= end)
        {
          error = experrstr;
          longjmp (termbuf, 1);
        }

      while (isspace (*start))
	++start;
      while (isspace (end [-1]))
	--end;

      if ((OPENPAREN == *start) && (CLOSEPAREN == end [-1]))
	{
	  ++start;
	  --end;
	}
      else
	break;
    }

  paren = 0;
  level = 1;
  cut   = start;

  /* scan for operators, applying precedence rules */
  for (ptr = start;
       ptr < end;
       ++ptr)
    switch (*ptr)
      {
	/* level 5 operators */
	case '^':
	case '|':if ((level < 5) && (0 == paren))
		   {
		     cut   = ptr;
		     level = 5;
		   }
		 break;

	/* level 4 operators */
	case '&':if ((level < 4) && (0 == paren))
		   {
		     cut   = ptr;
		     level = 4;
		   }
		 break;

	/* level 3 operators */
	case '+':
	case '-':if ((ptr != start) && (level < 3) && (0 == paren))
		   {
		     cut   = ptr;
		     level = 3;
		   }
		 break;

	/* level 2 operators */
	case '*':
	case '/':
	case '%':if ((level < 2) && (0 == paren))
		   {
		     cut   = ptr;
		     level = 2;
		   }
		 break;

	/* more level 2 operators */
	case '<':
	case '>':if ((ptr [0] == ptr [1]) && (level < 2) && (0 == paren))
		   {
		     cut   = ptr++;
		     level = 2;
		   }
		 break;

	/* parentheses */
	case '(':++paren;
		 break;

	case ')':--paren;
		 break;
      }


  switch (*cut)
    {
      case  '!':return (~eval (start + 1, end));

      case  '|':return (eval (start, cut) | eval (cut + 1, end));

      case  '^':return (eval (start, cut) ^ eval (cut + 1, end));

      case  '&':return (eval (start, cut) & eval (cut + 1, end));

      case  '+':if (cut != start)	/* not unary */
		  return (eval (start, cut) + eval (cut + 1, end));
		else
		  return (eval (start + 1, end));

      case  '-':if (cut != start)	/* not unary */
		  return (eval (start, cut) - eval (cut + 1, end));
		else
		  return (-eval (start + 1, end));

      case  '*':return (eval (start, cut) * eval (cut + 1, end));

      case  '/':return (eval (start, cut) / eval (cut + 1, end));

      case  '%':return (eval (start, cut) % eval (cut + 1, end));

      case  '<':return (eval (start, cut) << (eval (cut + 2, end) & 15));

      case  '>':return (eval (start, cut) >> (eval (cut + 2, end) & 15));

      case  '0':
      case  '1':
      case  '2':
      case  '3':
      case  '4':
      case  '5':
      case  '6':
      case  '7':
      case  '8':
      case  '9':return (dec_conv (start, end));

      case  '$':return (hex_conv (start + 1, end));

      case  '@':return (bin_conv (start + 1, end));

      case '\'':return (chr_conv (start + 1, end));

      case  '.':if ((end - start) == 1)
		  return (genaddr);

      default  :if (check_name (bound_str (start, end)))
		  return (symbol_val (start, end));
    }

  error = experrstr;
  longjmp (termbuf, 2);

  return 0;  /* keeps compiler happy */
}

int	dec_conv (start, end)
char	*start, *end;
{
  static int retval;

  /* check for empty expression */
  if (start >= end)
    {
      error = experrstr;
      longjmp (termbuf, 1);
    }

  retval = 0;

  for (;
       start < end;
       ++start)
    if ((*start >= '0') &&
	(*start <= '9'))
      {
	retval = retval * 10 + *start - '0';
      }
    else
      {
	error = decerrstr;
	longjmp (termbuf, 3);
      }

  return (retval);
}

int	hex_conv (start, end)
char	*start, *end;
{
  static int retval;

  /* check for empty expression */
  if (start >= end)
    {
      error = experrstr;
      longjmp (termbuf, 1);
    }

  retval = 0;

  for (;
       start < end;
       ++start)
    if ((*start >= '0') &&
	(*start <= '9'))
      {
	retval = (retval << 4) + *start - '0';
      }
    else if (*start >= 'A' && *start <= 'F')
      {
	/* handle cases separately as cant depend on toupper () */
	retval = (retval << 4) + *start - 'A' + 10;
      }
    else if (*start >= 'a' && *start <= 'f')
      {
	retval = (retval << 4) + *start - 'a' + 10;
      }
    else
      {
	error = hexerrstr;
	longjmp (termbuf, 4);
      }

  return (retval);
}

int	bin_conv (start, end)
char	*start, *end;
{
  static int retval;

  /* check for empty expression */
  if (start >= end)
    {
      error = experrstr;
      longjmp (termbuf, 1);
    }

  retval = 0;

  for (;
       start < end;
       ++start)
    if ((*start == '0') || (*start == '1'))
      {
	retval = (retval << 1) + *start - '0';
      }
    else
      {
	error = binerrstr;
	longjmp (termbuf, 5);
      }

  return (retval);
}

int	chr_conv (start, end)
char	*start, *end;
{
  static int retval;
  static int count;
  static char ch;

  retval = 0;

  for (count = 0;
       count < 3 && start < end;
       ++count)
    {
      switch (ch = *start++)
	{
	  case '\\':switch (ch = *start++)
		      {
		  	case 'n':
		  	case 'N':ch = 10;
			         break;
			case 't':
		        case 'T':ch = 9;
			         break;
			case 'b':
		        case 'B':ch = 8;
			         break;
		        case 'r':
		        case 'R':ch = 13;
			         break;
			case 'f':
		        case 'F':ch = 12;
			         break;
		      }
		    break;

	  case '\'':if (0 == count)
		      {
		        error = chrerrstr;
			longjmp (termbuf, 6);
		      }
		    else
		      return (retval);
        }

      retval = (retval << 8) + ch;
    }
  error = chrerrstr;
  longjmp (termbuf, 6);

  return 0;  /* keeps compiler happy */
}
