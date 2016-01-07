/********************************
 *                              *
 *   Assassin - 6809 assembler  *
 *                              *
 *     Brian Windrim 6/11/88    *
 *                              *
 *   addr.c - function module   *
 *                              *
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "recog.h"
#include "gen.h"
#include "addr.h"
#include "eval.h"

int branchaddr ()
{
  return (eval_str (addr_str));
}


int addmode ()
{
  static char   prechar, reg;
  static byte   mode, indirect, regno;
  static bool   comma;
  static char_ptr cut;
  static char_ptr scan;
  static char_ptr after;
  

  if (addr_str == (cut = strend (addr_str)))
    {
          error = mismoderr;
      return (INV);
    }

  comma = FALSE;
  error = 0;

  if (*addr_str == '#' || *addr_str == '<' || *addr_str == '>')
    {
      prechar = *addr_str++;
    }
  else
    {
      prechar = NUL;
      if (*addr_str == '[')
        {
          indirect = 0x10;
          ++addr_str;

          while (cut > addr_str && *--cut != ']');

          if (cut == addr_str)
            {
              error = brakerrstr;
              return (IND);
            }
        }
      else
        indirect = 0;

      for (scan = addr_str;
           scan < cut && *scan != COMMA;
           ++scan);

      if (scan != cut)
        {
          comma = TRUE;
          after = cut;
          cut   = scan++;
          mode  = BASE;
          if (*scan == '-')
            if (*++scan == '-')
              {
                mode = WDEC;
                ++scan;
              }
            else
              mode = BDEC;

          regno = 0;
          switch (reg = (CASEMASK & *scan++))
            {
              case 'S':regno += 0x20;
              case 'U':regno += 0x20;
              case 'Y':regno += 0x20;
              case 'X':if (mode == BASE)
                         if (*scan == '+')
                           if (*++scan == '+')
                             {
                               mode = WINC;
                               ++scan;
                             }
                           else
                             {
                               mode = BINC;
                             }
                       break;

              case 'P':if ((CASEMASK & *scan) != 'C')
                         error = basregerr;
                       else if ((CASEMASK & *++scan) == 'R')
                         {
                           ++scan;
                           reg = 'R';
                         }
                       break;

              default :error = basregerr;
            }

          if (skipspace (&addr_str) != NUL)
            for (;
                 scan < after;
                 ++scan)
              if (!isspace (*scan))
                {
                  error = basregerr;
                }
        }
    }

  if ((comma)           &&
      (addr_str + 1 == cut)     &&
      (mode == BASE)    &&
      (reg != 'P')      &&
      (reg != 'R')
     )
    switch (CASEMASK & *addr_str)
      {
        case 'A':genbyte (0x86 | regno | indirect);
                 return (IND);
        case 'B':genbyte (0x85 | regno | indirect);
                 return (IND);
        case 'D':genbyte (0x8B | regno | indirect);
                 return (IND);
      }

  if (addr_str < cut)
    {
      if (0 == setjmp (termbuf))
          val = eval (addr_str, cut);
      else
          return (INV);
    }
  else if (comma)
      val = 0;
  else
    {
      error = mismoderr;
      return (INV);
    }
/*      return (EXT);*/

  switch (prechar)
    {
      case '#':return (IMM);

      case '<':genbyte (val);
               return (DIR);

      case '>':genword(val);
               return(EXT);

      case 0:if (comma)
                 if (reg == 'R' || reg == 'P')
                   {
                     if (reg == 'R')
                       {
                         val -= genaddr + 2;
                         if (val > 127 || val < -128)
                           genbyteword (0x8D | indirect, val - 1);
                         else
                           genbytebyte (0x8C | indirect, val);
                       }
                     else if (val > 127 || val < -128)
                       genbyteword (0x8D | indirect, val);
                     else
                       genbytebyte (0x8C | indirect, val);

                     return (IND);
                   }
                 else /* not pc or pcr */
                   switch (mode)
                     {
                       case BASE:if (forward_sym)
                                   genbyteword (0x89 | regno | indirect, val);
                                 else if (val == 0)
                                   genbyte (0x84 | regno | indirect);
                                 else if (val < 16 && val >= -16 && !indirect)
                                   genbyte ((val & 0x1f) | regno);
                                 else if (val <= 127 && val >= -128)
                                   genbytebyte (0x88 | regno | indirect, val);
                                 else
                                   genbyteword (0x89 | regno | indirect, val);

                                 return(IND);

                       case BINC:if (val)
                                   error = autoerrstr;
                                 else if (indirect)
                                   error = inderrstr;
                                 else
                                   genbyte (0x80 | regno);

                                 return(IND);

                       case WINC:if (val)
                                   error = autoerrstr;
                                 else
                                   genbyte (0x81 | regno | indirect);

                                 return(IND);

                       case BDEC:if (val)
                                   error = autoerrstr;
                                 else if (indirect)
                                   error = inderrstr;
                                 else
                                   genbyte (0x82 | regno);

                                 return(IND);

                       case WDEC:if (val)
                                   error = autoerrstr;
                                 else
                                   genbyte (0x83 | regno | indirect);

                                 return(IND);
                     }
               else /* not comma */
                 if (indirect)
                   {
                     genbyteword (0x9f, val);
                     return(IND);
                   }
                 else
                   {
                     genword (val);
                     return (EXT);
                   }
    }
}

