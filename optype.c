/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *  optype.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "gen.h"
#include "recog.h"
#include "addr.h"
#include "optype.h"

static int getreg (void);

void	type1 (unsigned int code, unsigned int mode)
{
  static byte_ptr	optr;

  if (code > 255)
    genword (code);
  else
    genbyte (code);

  optr  = genptr - 1;
  error = noerror;

  switch (addmode ())
    {
      case IMM:if (BYTESTORE == mode || WORDSTORE == mode)
		 error = invmodestr;
	       else if (BYTELOAD == mode)
		 genbyte (val);
	       else
		 genword (val);
	       break;

      case EXT:*optr += 0x10;
      case IND:*optr += 0x10;
      case DIR:*optr += 0x10;
      case INV:break;

      default :error = invmodestr;
    }
}

int type2 (int code, int nextw, char ch, int oplen)
{
  static char regadj;
  static byte_ptr	optr;

  regadj = 0x40;

  if ((3 == oplen || 4 == oplen) &&
      (nextw & 0xFF) == ch)
    switch (nextw >> 8)
      {
        case 'B':regadj = 0x50;
	case 'A':genbyte (code | regadj);
		 error = noerror;
		 break;

	case NUL:optr = genptr;
		 genbyte (code);
		 switch (addmode ())
		   {
		     case EXT:*optr |= 0x10;
		     case IND:*optr |= 0x60;
		     case DIR:error = noerror;
		     case INV:break;

		     default :error = invmodestr;
		   }
		 break;
      }
  else
    return (0);

  return (1);
}

void	typeshift (int rcode, int lcode, int nextw, int oplen)
{
  static char code;
  static byte_ptr	optr;

  code = rcode;

  if (3 == oplen || 4 == oplen)
    switch (nextw)
      {
        case 'L':code = lcode;
	case 'R':optr = genptr;
		 genbyte (code);
		 switch (addmode ())
		   {
		     case EXT:*optr |= 0x10;
		     case IND:*optr |= 0x60;
		     case DIR:error = noerror;
		     case INV:break;

		     default :error = invmodestr;
		   }
		 break;

	case LB:lcode |= 0x10;
	case LA:lcode |= 0x40;
		genbyte (lcode);
		error = noerror;
		break;

	case RB:rcode |= 0x10;
	case RA:rcode |= 0x40;
		genbyte (rcode);
		error = noerror;
		break;
      }
}

void	typelea (int code)
{
  genbyte (code);
  error = noerror;

  if (addmode () != IND)
    error = invmodestr;
}

void	typemisc(int code)
{
  genbyte (code);
  error = noerror;
}

void	typeswi (int number)
{
  static char code;

  code = 0x10;

  switch (number)
    {
      case 3:++code;
      case 2:genbyte (code);
      case 1:genbyte (0x3F);
	     error = noerror;
    }
}

void	typecc (int code)
{
  error = noerror;

  if (IMM == addmode ())
    genbytebyte (code, val);
  else
    error = invmodestr;
}

void	typepspl (int code)
{
  static char dcode;

  genbyte (code);
  dcode = 0;
  error = noerror;

  while (TRUE)
    {
      switch (getreg())
        {
	  case  CC:dcode |= 0x01;
		   break;

	  case 'A':dcode |= 0x02;
		   break;

	  case 'D':dcode |= 0x02;
	  case 'B':dcode |= 0x04;
		   break;

	  case  DP:dcode |= 0x08;
		   break;

	  case 'X':dcode |= 0x10;
		   break;

	  case 'Y':dcode |= 0x20;
		   break;

	  case 'U':dcode |= 0x40;
		   if (code & 0x02)
		     error = sterrstr;
		   break;

	  case 'S':dcode |= 0x40;
		   if (!(code & 0x02))
		     error = sterrstr;
		   break;

	  case  PC:dcode |= 0x80;
		   break;

	  case NUL:if (dcode)
		     genbyte (dcode);
		   else
		     error = noregstr;
		   return;

	  default :error = regerrstr;
        }
    }
}

void	typesc(int code)
{
  static char dcode;

  genbyte (code);
  dcode = 0;
  error = noerror;

  do
    switch (CASEMASK & *op_str++)
      {
	case 'E':dcode |= 0x80;
		 break;

	case 'F':dcode |= 0x40;
		 break;

	case 'H':dcode |= 0x20;
		 break;

	case 'I':dcode |= 0x10;
		 break;

	case 'N':dcode |= 0x08;
		 break;

	case 'Z':dcode |= 0x04;
		 break;

	case 'V':dcode |= 0x02;
		 break;

	case 'C':dcode |= 0x01;
		 break;

	default :error=flagerrstr;
      }
  while (!error && *op_str);

  genbyte ((0x1C == code) ? ~dcode : dcode);
}

void	typext (int code)
{
  static char count;
  static int  dcode;
  static int reg;

  count = 2;
  dcode = 0;
  error = noerror;

  genbyte (code);

  do
    {
      switch (reg = getreg ())
	{
	  case 'X':dcode |= 0x01;
	  case 'D':break;

	  case 'Y':dcode |= 0x02;
		   break;

	  case 'U':dcode |= 0x03;
		   break;

	  case 'S':dcode |= 0x04;
		   break;

	  case  PC:dcode |= 0x05;
		   break;

	  case 'A':dcode |= 0x08;
		   break;

	  case 'B':dcode |= 0x09;
		   break;

	  case  CC:dcode |= 0x0A;
		   break;

	  case  DP:dcode |= 0x0B;
		   break;

	  case NUL:error = noregstr;
		   break;

	  default :error = regerrstr;
	}

      dcode <<= 4;
      skipspace (&addr_str);
    }
  while (!error && --count && NUL != reg);

  dcode >>= 4;

  if (1 == count && error != regerrstr)
    error = dsterrstr;

  if (!error)
    if (((dcode & 0x88) == 0x80) || ((dcode & 0x88) == 0x08))
      error = sizerrstr;
    else
      genbyte (dcode);
}

void	typebr (unsigned int nextw, unsigned int lso)
{
  static int code;

  error = noerror;

  switch (nextw)
    {
      case SR:code = 0x178D;
	      break;

      case RA:code = 0x1620;
	      break;

      case RN:code = 0x1021;
	      break;

      case HI:code = 0x1022;
	      break;

      case LS:code = 0x1023;
	      break;

      case CC:
      case HS:code = 0x1024;
	      break;

      case CS:
      case LO:code = 0x1025;
	      break;

      case NE:code = 0x1026;
	      break;

      case EQ:code = 0x1027;
	      break;

      case VC:code = 0x1028;
	      break;

      case VS:code = 0x1029;
	      break;

      case PL:code = 0x102A;
	      break;

      case MI:code = 0x102B;
	      break;

      case GE:code = 0x102C;
	      break;

      case LT:code = 0x102D;
	      break;

      case GT:code = 0x102E;
	      break;

      case LE:code = 0x102F;
	      break;

      default:error = unrecopstr;
	      return;
    }

  forward_sym = FALSE;

  val = branchaddr () - (genaddr + 2);

  if (OPT == lso)
    if (val <= 127 && val >= -128 && !forward_sym && (val <= -2))
      lso = SHORT;
    else
      lso = LONG;

  if (SHORT == lso)
    genbytebyte (code, val);
  else if ((code & 0xFF00) == 0x1000)
    genwordword (code, val - 2);
  else
    genbyteword (code >> 8, val - 1);
}

static int getreg ()
{
  static char *ptr;


  if (NULL == (ptr = next_field ()))
    return (NUL);
  else if (2 < strlen (ptr))
    return (ERR);
  else
/*    return ((*(cast (int_ptr) ptr)) & CASEMASK);*/
    return (((ptr [1] << 8) | ptr [0]) & CASEMASK);
}
