/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *  symbol.c - function module	*
 *				*
 ********************************/

#include <stdio.h>
#include <setjmp.h>
#include <malloc.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "search.h"
#include "symbol.h"

Object	root_block = {
		       BLOCK,
		       NULL,
		       "ROOT",
		       {
			   &root_block,
			   NULL
		       }
		     };

Obj_ptr	block_stack [BLOCK_STACK_SIZE] = { NULL, &root_block },
	*block_ptr		       = block_stack + 1;

static void dump_obj (
		      Obj_ptr obj,
		      int level
		      )
{
    Obj_ptr obj_ptr;
    static int i;

    for (i = level;
	 i;
	 --i)
	fprintf (stderr, "\t");

    if (BLOCK == obj->type && NULL != obj->var.blk.insert)
    {
	fprintf (stderr, "%s\n", obj->name ? obj->name : "NULL");

	for (obj_ptr = obj->var.blk.contents;
	     obj_ptr != obj->var.blk.insert->next;
	     obj_ptr = obj_ptr->next)
        {
	    dump_obj (obj_ptr, level + 1);
        }
    }
    else if (obj->type & VALUE)
    {
	fprintf (stderr, "%s = %x\n", obj->name, obj->var.sym.value);
    }
}

Obj_ptr	find_local_sym (
    const char	*start,
    const char        *end,
    int		stype,
    Obj_ptr     block
			)
{
  static Obj_ptr obj_ptr;
  static const char	*ch1, *ch2;

  if (start == end || NULL == block || NULL == block->var.blk.insert)
    return (NULL);

  for (obj_ptr =  block->var.blk.contents;
       obj_ptr != block->var.blk.insert->next;
       obj_ptr =  obj_ptr->next)
    if (NULL != obj_ptr->name && (obj_ptr->type & stype))
      {
	ch1 = start;
	ch2 = obj_ptr->name;

	while (ch1 < end && *ch2 && *ch1 == *ch2)
	  {
	    ++ch1;
	    ++ch2;
	  }

	if (ch1 == end && *ch2 == NUL)
	  {
	    if ((obj_ptr->type & (ABS | REL)) &&
		(obj_ptr->var.sym.value > genaddr)
		)
	      forward_sym = TRUE;

	    return (obj_ptr);
	  }
      }

  return (NULL);
}


void	create_block (const char *block_name)
{
  static Obj_ptr new_block;

/*if (debug) fprintf (stderr, "entering create_block\n");*/

  if (NULL != block_name
      && NULL != find_local_sym (block_name, strend (block_name),
				 BLOCK, *block_ptr
				)
     )
    {
      error = deferrstr;
/*if (debug) fprintf (stderr, "1 leaving create_block\n");*/
      return;
    }

  if (block_ptr == (block_stack + (BLOCK_STACK_SIZE - 2)))
    {
      error = bgnerrstr;
/*if (debug) fprintf (stderr, "2 leaving create_block\n");*/
      return;
    }

  new_block = cast (Obj_ptr) malloc (sizeof (Object));

  if (NULL == new_block)
    {
      error = memerrstr;
    }
  else
    {
      new_block->type = BLOCK;
      new_block->var.blk.insert = NULL;
      new_block->name = (char *)block_name; /* FixMe */

      insert_symbol (BLOCK, (char *)/*FixMe*/block_name, new_block, *block_ptr);

      new_block->var.blk.contents = (*block_ptr)->var.blk.contents;

      *++block_ptr = new_block;
    }
/*if (debug) fprintf (stderr, "3 leaving create_block\n");*/
}

void	open_block (const char *block_name)
{
  static Obj_ptr ptr;

  if (block_ptr == (block_stack + (BLOCK_STACK_SIZE - 2)))
    {
      error = bgnerrstr;
      return;
    }

  if (block_ptr [1] == NULL)
      ptr = (*block_ptr)->var.blk.contents;
  else
      ptr = block_ptr [1]->next;

  for (;
       ptr != NULL && !(ptr->type & BLOCK);
       ptr = ptr->next);

  if (NULL == ptr || safecmp (ptr->name, block_name))
    error = bgnerrstr;
  else
    {
      *++block_ptr  = ptr;
      block_ptr [1] = NULL;
    }
}

void	end_block (void)
{
  if (block_ptr > (block_stack + 1))
    --block_ptr;
  else
    error = enderrstr;
}

void	insert_symbol (
char	stype,
char	*sname,
Obj_ptr	new_symbol,
Obj_ptr	block
		       )
{

    if (NULL == block)
    {
	error = enterrstr;
	return;
    }

    new_symbol->type = stype;
    new_symbol->name = sname;

    if (block->var.blk.insert == NULL)
    {
	new_symbol->next	    = block->var.blk.contents;
	block->var.blk.contents = new_symbol;
    }
    else
    {
	new_symbol->next		= block->var.blk.insert->next;
	block->var.blk.insert->next = new_symbol;
    }

    block->var.blk.insert = new_symbol;
}


void	add_symbol (
     char	stype,
     char	*sname,
     int	svalue,
     Obj_ptr	block
     )
{
  static Obj_ptr new_symbol;

  if (NULL == block)
    {
      error = enterrstr;
      return;
    }

  if (NULL != (new_symbol = find_local_sym (sname,
					    strend (sname),
					    VALUE,
					    block)))
    {
      if (1 >= pass ||
	  (new_symbol->type & ~REDEF & 0xff) != (stype & ~REDEF & 0xff))
	{
	  error = deferrstr;
	  return;
	}
      else if (new_symbol->var.sym.value == svalue)
	{
	  return;
	}
      else if (new_symbol->type & REDEF)
	{
	  new_symbol->var.sym.value = svalue;
	  new_symbol->type &= ~REDEF;
	}
      else
	{
	  error = deferrstr;
	}
    }
  else if (1 < pass)
  {
      error = pasdeferr;
  }
  else if (NULL == (new_symbol = cast (Obj_ptr) malloc (sizeof (Object))))
  {
      error = memerrstr;
  }
  else
  {
      new_symbol->var.sym.value = svalue;
      insert_symbol (stype, sname, new_symbol, block);
  }
}


Obj_ptr	add_macro (
		   char	*sname,
		   int	params,
		   Obj_ptr	block
		   )
{
  static Obj_ptr new_symbol;

  if (NULL == block)
    {
      error = enterrstr;
      return NULL;
    }

  if (NULL != (new_symbol = find_local_sym (sname, 
					    strend (sname), 
					    VALUE,
					    block)))
    {
      if (1 >= pass)
	{
	  error = deferrstr;
	  return NULL;
	}
      else if (new_symbol->var.mac.no_params == params)
	{
	  return NULL;
	}
    }
  else if (1 < pass)
  {
      error = pasdeferr;
  }
  else if (NULL == (new_symbol = cast (Obj_ptr) malloc (sizeof (Object))))
  {
      error = memerrstr;
  }
  else
  {
      new_symbol->var.mac.no_params = params;
      new_symbol->var.mac.lines     = NULL;
      insert_symbol (MACRO, sname, new_symbol, block);
  }

    return new_symbol;
}


Obj_ptr	find_global_sym (
    const char	*start,
    const char        *end,
    char	stype,
    Obj_ptr     block
			 )
{
  static Obj_ptr obj_ptr;
  static const char	*ch1, *ch2;

  if (start == end || NULL == block)
    return (NULL);

/*
  if (1 >= pass)
    return find_local_sym (start, end, stype, block);
*/
/*
  if (NULL != (obj_ptr = find_local_sym (start, end, stype, block)))
  {
    return obj_ptr;
  }
  else
  {
    forward_sym = TRUE; * search outside block *

    if (1 >= pass)	* if first pass stop here *
 	return NULL;
  }
*/
  for (obj_ptr = block->var.blk.contents;
       obj_ptr != NULL;
       obj_ptr = obj_ptr->next)
    if (NULL != obj_ptr->name && (obj_ptr->type & stype))
      {
	ch1 = start;
	ch2 = obj_ptr->name;

	while (ch1 < end && *ch2 && *ch1 == *ch2)
	  {
	    ++ch1;
	    ++ch2;
	  }

	if (ch1 == end && *ch2 == NUL)
	  {
	    if ((obj_ptr->type & (ABS | REL)) &&
		(obj_ptr->var.sym.value > genaddr)
		)
	      forward_sym = TRUE;

	    return (obj_ptr);
	  }
      }

  return (NULL);
}


Obj_ptr	find_name_sym (
		       const char	*start,
		       const char     *end,
		       char	stype
		       )
{
    Obj_ptr	 block;
    const char	*ptr;

/*if (debug) fprintf (stderr, "entering find_name_sym\n");*/

    for (ptr = end;
	 ptr > start && '.' != ptr [-1];
         --ptr);

    if (ptr == end)
    {			/* path ended with dot */
	error = nameptherr;
/*if (debug) fprintf (stderr, "1 leaving find_name_sym\n");*/
	return (NULL);
    }
    else if (ptr == start)
    {			/* no dot found */
/*if (debug) fprintf (stderr, "2 leaving find_name_sym\n");*/

	if (':' == *ptr)	/* path starts with colon */
	{
/*if (debug) fprintf (stderr, "global name\n");*/

	    block = &root_block;
	    ++ptr;
	}
	else
	    return (find_global_sym (start, end, stype, *block_ptr));
    }
    else if (ptr == start + 1)
    {			/* path starts with dot */
	block = *block_ptr;
    }
    else
    {
	block = find_name_sym (start, ptr - 1, BLOCK);
/*if (debug) fprintf (stderr, "3 leaving find_name_sym\n");*/
    }

/*if (debug) fprintf (stderr, "4 leaving find_name_sym\n");*/
    return (find_local_sym (ptr, end, stype, block));
}


int	symbol_val (
    const char *start,
    const char *end
		    )
{
  static Obj_ptr obj_ptr;


  if (NULL != (obj_ptr = find_name_sym (start, end, VALUE))
      && !(BLOCK & obj_ptr->type)
     )
    return (obj_ptr->var.sym.value);
  else if (1 >= pass)
    {
      forward_sym = TRUE;
      error = 0;
      return (0);
    }
  else
    {
      error = symerrstr;
      longjmp (termbuf, 10);

      return 0;   /* keeps compiler happy */
    }
}


void	redef_sym  (
		    char	stype,
		    const char   *sname,
		    int	svalue
		    )
{
  static Obj_ptr obj_ptr;


  if (NULL != (obj_ptr = find_name_sym (sname, strend (sname), stype)))
    {
      /*
       * Should check  obj_ptr->type = stype here
       */

      obj_ptr->var.sym.value = svalue;
      obj_ptr->type |= REDEF;
    }
  else
    {
      error = symerrstr;
    }
}

