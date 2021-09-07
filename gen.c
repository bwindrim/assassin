/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *    gen.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "gen.h"

byte genbuffer [GEN_BUFF_SIZE];
byte_ptr	genptr;

static FILE    *gen_file;


void	gen_init(unsigned int address)
{
  load_addr =
  exe_addr  =
  genaddr   = address;
  genptr    = genbuffer;
}

void	gen_org (unsigned int address)
{
  if (genaddr == load_addr)
    {
      load_addr =
      exe_addr  = address;
    }

  genaddr = address;
  genptr  = genbuffer;
}

void gen_exec (unsigned int address)
{
    error = noerror;

    if (load_addr == exe_addr)
	exe_addr = address;
    else
	error = exeadderr;
}

void	genbyte (int data)
{
  if (data <= 255 && data >= -128)
    *genptr++ = data;
  else
    error = byterrstr;
}

void	genword (int data)
{
  *genptr++ = data >> 8;
  *genptr++ = data;
}

void	genbytebyte (int data1, int data2) /* FixMe */
{
  genbyte (data1);
  genbyte (data2);
}

void	genbyteword (int data1, int data2)
{
  genbyte (data1);
  genword (data2);
}

void	genwordword (int data1, int data2)
{
  genword (data1);
  genword (data2);
}

void	genstring (const char *ptr)
{
  if (NULL == ptr)
    error = badstrerr;
  else
    while (*ptr)
      *genptr++ = *ptr++;
}

void	dumpcode (void)
{
  int length = genptr - genbuffer;

  if (length > 0)
    {
      if (!gen)
        code_length += length;
      else
	if (-1 == fwrite (genbuffer, 1, length, gen_file))
          error = outerrstr;

      genaddr += length;
      genptr = genbuffer;
    }
}

void	flushcode (void)
{
  if (1 < pass &&
      (symerrstr == error ||
       deferrstr == error))
    {
      if (!gen)
        code_length += genptr - genbuffer;

      genaddr += genptr - genbuffer;
    }

  genptr = genbuffer;
}

void	fputmw (FILE *fp, unsigned int word)
{
  putc (word >> 8, fp);
  putc (word & 0xff, fp);
}

void	gen_open (const char *filename)
{
  char	 buffer [20];
  static char	*ptr;

  ptr = buffer;

  while (*filename != '.' && *filename != NUL)
    *ptr++ = *filename++;

  *ptr = NUL;

  strcat (buffer, ".ex9");

  if (NULL == (gen_file = fopen (buffer, "wb")))
    {
      fprintf (stderr, filerrstr, buffer, "writing");
      exit (1);
    }

  fputmw (gen_file, load_addr);
  fputmw (gen_file, code_length);
}

void gen_close (void)
{
    fputmw (gen_file, exe_addr);
    fclose (gen_file);
}

void	dumpblock (void)
{

  if (!gen)
    {
      code_length += nextaddr - genaddr;
      genaddr = nextaddr;
    }
  else
    while (genaddr < nextaddr)
      {
	putc (0, gen_file);
	++genaddr;
      }
}
