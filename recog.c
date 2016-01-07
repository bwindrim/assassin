/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   recog.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "symbol.h"
#include "macro.h"
#include "gen.h"
#include "search.h"
#include "optype.h"
#include "recog.h"
#include "addr.h"
#include "defop.h"

char	*curr_name,
	*op_str,
	*addr_str;

static bool	checkrest(str)
	char_ptr	str;
{
  do
    if (*str != *op_str++)
      return (FALSE);
  while (*str++);

  return (TRUE);
}


void	outline (linebuffer, fname, lno)
	char	*linebuffer;
	char    *fname;
	int      lno;
{
  static char	*pr_ptr;

  if (hack_error)
    error = hack_error;

  if (list)
    {
      if (error)
	{
	  if (out_diff)
	    {
              printf ("%04X:           %s", genaddr, linebuffer);
              printf (errfrmstr, errtext [error]);
	    }

	  flushcode ();
	}
      else
        {
	  printf ("%04X:", genaddr);

	  for (pr_ptr = genbuffer;
	       pr_ptr < genbuffer + 5;
	       ++pr_ptr)
	    {
	      if (pr_ptr < genptr)
		printf ("%02X", *pr_ptr & 0xff); /* clobber any sign extn. */
	      else
		printf ("  ");
	    }

          dumpcode ();
          printf (" %s", linebuffer);
        }
    }
  else
    {
      if (error)
        flushcode();
      else
        dumpcode();
    }

  if (error)
    {
      if (1 != pass)
	{
	  ++err_count;
	  fprintf (stderr, "%04X:           %s", genaddr, linebuffer);
	  fprintf (stderr, err2frmstr, fname, lno, errtext [error]);
	}

      error = noerror;
    }

  if (nextaddr)
    dumpblock ();
}


static void	end_line (linebuffer, fname, lno)
	char	*linebuffer;
        char    *fname;
        int      lno;
{
  if (curr_name)
    add_symbol (ABS, curr_name, genaddr, *block_ptr);

  outline (linebuffer, fname, lno);
}


static bool	split_line (buffer)
	char	*buffer;
{
  static char *ch, *name;
  static char loc_buf [TEXT_BUFF_SIZE];
  static bool quote_flag;

  curr_name = NULL;

  if (*buffer != ';' && !isspace (*buffer))
    {
      for (ch = buffer;
	   *ch && !isspace (*ch) && (*ch != COLON);
	   ++ch) ;

      if (NULL == (name = (char_ptr) malloc ((ch - buffer) + 1)))
	{
	  error = memerrstr;
	  return FALSE;   /* value not needed, keeps compiler happy */
	}

      for (ch = name;
           *buffer && !isspace (*buffer) && (*buffer != COLON);
	   *ch++ = *buffer++) ;

      *ch = NUL;

      if ((COLON == *buffer) && (0 < strlen (name)))
	add_symbol (ABS, name, genaddr, *block_ptr);
      else
	curr_name = name;

      ++buffer;
    }

  skipspace (&buffer);

  for (ch = loc_buf;
       *buffer && ';' != *buffer && !isspace(*buffer);
       *ch++ = toupper(*buffer++));

  *ch++ = NUL;

  op_str   = loc_buf;
  addr_str = ch;

  skipspace (&buffer);

  for (quote_flag = FALSE;
       *buffer && (quote_flag || *buffer != ';');
       *ch++ = *buffer++)
	{
	  if (2 != quote_flag && SINGLEQUOTE == *buffer)
	    quote_flag ^= 1;
	  else if (1 != quote_flag && DOUBLEQUOTE == *buffer)
	    quote_flag ^= 2;
	}

  while (isspace (ch [-1])) --ch;

  *ch = NUL;
/*
if (0 == pass)
  printf ("curr_name == >>%s<< op_str == >>%s<< addr_str == >>%s<<\n",
	  curr_name ? curr_name : "NULL",
	  op_str ? op_str : "NULL",
	  addr_str ? addr_str : "NULL");
*/
}


void	parse (linebuffer, inp, fname, lno)
	char	*linebuffer;
	FILE	*inp;
        char    *fname;
	int      lno;
{
  static bool	  fourlong;
  static int	  oplen;
  static unsigned firstw, nextw;
  static Obj_ptr  macro;


  nextaddr = 0;
  forward_sym = FALSE;

  split_line (linebuffer);
  
/*  if (error)
    {
      outline (linebuffer, fname, lno);

      return;
    }
*/

  hack_error = error;

  if (0 == (oplen = strlen (op_str)))
    {
      outline (linebuffer, fname, lno);

      return;
    }
  
  error   = unrecopstr;

  if (oplen < 1)
    {
      outline (linebuffer, fname, lno);

      return;
    }
  
  fourlong = (oplen == 4);
  /* replace this bit with portable code
  firstw   = *(cast (int_ptr) op_str);
  op_str  += 2;
  nextw	   = *(cast (int_ptr) op_str);
  */

  firstw = (op_str [1] << 8) | op_str [0];
  op_str  += 2;
  nextw  = (op_str [1] << 8) | op_str [0];

  switch (firstw)
    {
      case AB:if (nextw == 'X')
		typemisc (0x3A);
	      break;
  
      case AD:if (fourlong)
	        switch (nextw)
		  {
		    case CA:type1 (0x89, BYTELOAD);
	  	  	    break;
	  	    case CB:type1 (0xC9, BYTELOAD);
	  	  	    break;
	  	    case DA:type1 (0x8B, BYTELOAD);
			    break;
		    case DB:type1 (0xCB, BYTELOAD);
	  	  	    break;
	  	    case DD:type1 (0xC3, WORDLOAD);
	  	  	    break;
	  	  }
	      break;
  
      case AN:if (fourlong)
	        switch (nextw)
	  	  {
		    case DA:type1 (0x84, BYTELOAD);
	  	  	    break;
	  	    case DB:type1 (0xC4, BYTELOAD);
	  	  	    break;
		  }
	      else if (oplen==5 && checkrest ("DCC"))
	        typecc (0x1C);
	      break;
  
      case AS:typeshift (0x07, 0x08, nextw, oplen);
	      break;

      case BI:if (fourlong)
	        switch (nextw)
	  	  {
		    case TA:type1 (0x85, BYTELOAD);
	  	  	    break;
	  	    case TB:type1 (0xC5, BYTELOAD);
			    break;
	  	  }
	      break;
  
      case CL:if (!type2 (0x0F, nextw, 'R', oplen) &&
	          (oplen > 2))
	  	typesc (0x1C);
	      break;
  
      case CM:if (fourlong)
	        switch (nextw)
	  	  {
		    case PA:type1 (0x81,   BYTELOAD);
	  	  	    break;
		    case PB:type1 (0xC1,   BYTELOAD);
			    break;
	  	    case PD:type1 (0x1083, WORDLOAD);
	  	  	    break;
	  	    case PS:type1 (0x118C, WORDLOAD);
	  	  	    break;
	  	    case PU:type1 (0x1183, WORDLOAD);
			    break;
	  	    case PX:type1 (0x8C,   WORDLOAD);
	  	  	    break;
	  	    case PY:type1 (0x108C, WORDLOAD);
	  	  	    break;
		  }
	      break;

      case CO:type2 (0x03, nextw, 'M', oplen);
	      break;
  
      case CW:if (fourlong && (nextw == AI))
	        typecc (0x3C);
	      break;

      case DA:if (nextw == 'A')
		typemisc (0x19);
	      break;
  
      case DE:switch (nextw)
	        {
		  case FB:if (fourlong)
	  	  	    defb ();
	  	  	  break;

	  	  case FW:if (fourlong)
	  	  	    defw ();
			  break;

	  	  case FS:if (fourlong)
	  	  	    defs ();
	  	  	  break;

	  	  default:type2 (0x0A, nextw, 'C', oplen);
	  	  	  break;
		}
	      break;
  
      case DB:if (2 == oplen)
	        defb ();
	      break;
  
      case DW:if (2 == oplen)
	        defw ();
	      break;
  
      case DS:if (2 == oplen)
	        defs ();
	      break;

      case EN:if ('D' == nextw)
	        {
	  	  error = noerror;
	  	  end_block ();
	        }
	      else if (checkrest ("TRY"))
		{
	  	  error = noerror;
	  	  add_symbol (ABS, curr_name, genaddr, block_ptr [-1]);
	  	  curr_name = NULL;
		}
	      break;
  
      case EO:if (fourlong)
	        switch (nextw)
	  	  {
		    case RA:type1 (0x88, BYTELOAD);
	  	    	    break;
	  	    case RB:type1 (0xC8, BYTELOAD);
	  	    	    break;
		  }
	      break;
  
      case EQ:if ('U' == nextw)
	        {
	  	  if (curr_name)
	  	    {
		      error = noerror;
	  	      val   = branchaddr ();

	  	      if (noerror == error)
	  	        add_symbol (EQU, curr_name, val, *block_ptr);

	  	      curr_name = NULL;
		    }
	  	  else
	  	    error = laberrstr;
	        }
	      break;
  
      case EX:if (nextw == 'G')
		typext (0x1E);
	      else if (checkrest ("EC"))
		  gen_exec (branchaddr ());
	      break;
  
      case IN:if (checkrest ("CLUDE"))
		{
		  error = noerror;
		  end_line (linebuffer, fname, lno);
		  parse_file (strip_quotes (addr_str));

		  return;
		}
	      else
		type2 (0x0C, nextw, 'C', oplen);
	      break;
  
      case JS:if (nextw == 'R')
	        type1 (0x8D, WORDSTORE);
	      break;
  
      case JM:if (nextw == 'P')
	        type2 (0x0E, nextw, 'P', oplen);
	      break;

      case LB:if (fourlong)
	        typebr (nextw, LONG);
	      break;

      case LD:switch (nextw)
	        {
		  case 'A':type1 (0x86,   BYTELOAD);
	  	  	   break;
	  	  case 'B':type1 (0xC6,   BYTELOAD);
	  	  	   break;
		  case 'D':type1 (0xCC,   WORDLOAD);
	  	  	   break;
	  	  case 'S':type1 (0x10CE, WORDLOAD);
	  	  	   break;
	  	  case 'U':type1 (0xCE,   WORDLOAD);
	  	  	   break;
	  	  case 'X':type1 (0x8E,   WORDLOAD);
			   break;
	  	  case 'Y':type1 (0x108E, WORDLOAD);
	  	  	   break;
	        }
	      break;
  
      case LE:if (fourlong)
		switch (nextw)
	  	  {
		    case AX:typelea (0x30);
	  	  	    break;
	  	    case AY:typelea (0x31);
	  	  	    break;
	  	    case AS:typelea (0x32);
			    break;
		    case AU:typelea (0x33);
	  	  	    break;
	  	  }
	      break;
  
      case LS:typeshift (0x04, 0x08, nextw, oplen);
	      break;

      case MA:if (checkrest ("CRO"))
		  def_macro (linebuffer, inp, fname, lno);
	      break;

      case MU:if (nextw == 'L')
		typemisc (0x3D);
	      break;
  
      case NE:type2 (0x00, nextw, 'G', oplen);
	      break;
  
      case NO:if (nextw == 'P')
		typemisc (0x12);
	      break;

      case OR:switch (nextw)
	        {
		  case 'A':type1 (0x8A, BYTELOAD);
	  	  	   break;

	  	  case 'B':type1 (0xCA, BYTELOAD);
	  	  	   break;

	  	  case  CC:if (fourlong)
	  	  	     typecc (0x1A);
			   break;

	  	  case 'G':error = noerror;
	  	  	   gen_org (branchaddr ());
	  	  	   break;
	        }
	      break;
  
      case PS:if (fourlong)
	        switch (nextw)
	  	  {
		    case HU:typepspl (0x36);
	  	  	    break;
	  	    case HS:typepspl (0x34);
	  	  	    break;
		  }
	      break;
  
      case PU:if (fourlong)
	        switch (nextw)
	  	  {
		    case LU:typepspl (0x37);
			    break;
	  	    case LS:typepspl (0x35);
	  	  	    break;
	  	  }
	      break;

      case RE:if (checkrest ("DEF"))
		redef ();
	      break;
  
      case RO:typeshift (0x06, 0x09, nextw, oplen);
	      break;
  
      case RT:switch (nextw)
		{
		  case 'S':typemisc (0x39);
	  	  	   break;
	  	  case 'I':typemisc (0x3B);
	  	  	   break;
	        }
	      break;

      case SU:if (fourlong)
	        switch (nextw)
	  	  {
		    case BA:type1 (0x80, BYTELOAD);
	  	  	    break;
	  	    case BB:type1 (0xC0, BYTELOAD);
			    break;
	  	    case BD:type1 (0x83, WORDLOAD);
	  	  	    break;
	  	  }
	      break;

      case SB:if (fourlong)
		switch (nextw)
	  	  {
		    case CA:type1 (0x82, BYTELOAD);
	  	  	    break;
	  	    case CB:type1 (0xC2, BYTELOAD);
	  	  	    break;
	  	    default:typebr (nextw, SHORT);
			    break;
	  	  }
	      break;
  
      case ST:switch (nextw)
	        {
		  case 'A':type1 (0x87,   BYTESTORE);
			   break;
	  	  case 'B':type1 (0xC7,   BYTESTORE);
	  	  	   break;
	  	  case 'D':type1 (0xCD,   WORDSTORE);
	  	  	   break;
	  	  case 'S':type1 (0x10CF, WORDSTORE);
			   break;
		  case 'U':type1 (0xCF,   WORDSTORE);
	  	  	   break;
	  	  case 'X':type1 (0x8F,   WORDSTORE);
	  	  	   break;
	  	  case 'Y':type1 (0x108F, WORDSTORE);
	  	  	   break;
	        }
	      break;
  
      case SE:if (nextw == 'X')
		typemisc (0x1D);
	      else
	        typesc (0x1A);
	      break;

      case SW:if (fourlong || (oplen == 3))
	        switch (nextw)
	  	  {
		    case 'I':typeswi (1);
	  	  	     break;
	  	    case  I2:typeswi (2);
			     break;
	  	    case  I3:typeswi (3);
	  	  	     break;
	  	  }
	      break;
  
      case SY:if (fourlong && (nextw == NC))
		typemisc (0x13);
	      break;
  
      case TS:type2 (0x0D, nextw, 'T', oplen);	      
	      break;
  
      case TF:if (nextw == 'R')
		typext (0x1F);
	      break;
  
      case BE:if (checkrest ("GIN"))
	        {
	  	  error = noerror;
  
		  if (1 >= pass)
		    create_block (curr_name);
	  	  else
	  	    open_block (curr_name);
  
		  curr_name = NULL;

		  break;
	        }
  
      default:if ((oplen == 3) && ((firstw & 0xFF) == 'B'))
	        typebr (firstw >> 8 | nextw << 8, OPT);
    }

  if (unrecopstr == error && NULL != (macro = get_macro (op_str - 2)))
    {
      error = noerror;
      end_line (linebuffer, fname, lno);
      exp_macro (macro, fname, lno); 
      return;
    }

  end_line (linebuffer, fname, lno);
}


static bool	ctrl_c_hit ()
{
/* need none of this
  if (keyhit () && (3 == rawin ()))
    {
      error = intrerrstr;

      return (TRUE);
    }
*/

  return (FALSE);
}


void	parse_stream (fp, fname)
FILE	*fp;
char    *fname;
{
  static char textbuffer [TEXT_BUFF_SIZE];
  int local_count;

  local_count = 0;

  while (fgets (textbuffer, TEXT_BUFF_SIZE, fp))
    {
/*      if (debug) dump_obj (&root_block, 0);*/

      ++local_count;
      parse (textbuffer, fp, fname, local_count);
      ++lcount;

      if (ctrl_c_hit ())
	{
	  fprintf (stderr, errfrmstr, errtext [intrerrstr]);
	  exit (3);
	}
    }
}


void	parse_file (filename)
	char	*filename;
{
  static FILE *inp;
  static int level = -1;
  FILE *inp_save;
  int	pos;
  char  name [15];

  if (0 == strlen (filename))
    {
      error = inclerrstr;

      return;
    }

  /* can do without this stuff
    if (0 < level++)
    {
	* stream already in use *

	pos = ftell (inp);
	fname (inp, name);
	fclose (inp);
    }
    else
    {
	inp_save = inp;
    }
*/

  if (NULL == (inp = fopen (filename, "r")))
    {
      fprintf (stderr, filerrstr, filename, "reading");
      exit (1);
    }
  else
    {
      parse_stream (inp, filename);
      fclose (inp);
    }

  /* and this stuff
    if (0 < --level)
    {
	if (NULL == (inp = fopen (name, "r")))
	{
	     fprintf (stderr, filerrstr, name, "reading");
	     exit (1);
	}

	fseek (inp, 0, pos, 0);
    }
    else
    {
	inp = inp_save;
    }
    */
}
