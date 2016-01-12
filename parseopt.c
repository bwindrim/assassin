#include <stdlib.h>
#include <string.h>

#include "typedefs.h"
#include "const.h"
#include "parseopt.h"

int parse_opt (
     struct opt_rec table [],
     int size,
     int *argc,
     char *argv []
)
{
	static struct opt_rec *opt_ptr;
	static char **arg_ptr;
	static int count;
	static char **src, **dst;

	count = 0;

	for (arg_ptr = argv + 1;
	 arg_ptr < argv + *argc;
	 ++arg_ptr)
	{
/*printf ("scanning arg. no. %d\n", arg_ptr - argv);*/

	if (NULL == *arg_ptr)
	    continue;

	if (0 == strcmp ("--", *arg_ptr))
	{
	    *arg_ptr = NULL;
	    break;
	}

	for (opt_ptr = table;
	     opt_ptr < table + size;
	     ++opt_ptr)
	{
/*	printf ("comparing >>%s<< with >>%s<<\n",
		opt_ptr->opt_name, *arg_ptr
		);*/
	    if (0 == strcmp (opt_ptr->opt_name, *arg_ptr))
	    {
/*		printf ("matched\n");*/
		if (!(*(opt_ptr->opt_func)) (opt_ptr,
					 *argc - (arg_ptr - argv),
					 arg_ptr
					 )
		    )
		    return -1;
		else
		    ++count;

		break;
	    }
	}
    }

	/* clean up argv */
    for (src = dst = argv;
         src < argv + *argc;
	 )
    {
	if (NULL == *src)
	    ++src;
	else
	    *dst++ = *src++;
    }

	*argc = dst - argv;

    return count;
}

int noarg_opt (
    struct opt_rec *opt,
    int argc,
    char *argv []
)
{
    static int *ip;

	ip = (int_ptr) opt->opt_data;
	*ip = TRUE;
    *argv = NULL;
    return TRUE;
}

int starg_opt (
    struct opt_rec *opt,
    int argc,
    char *argv []
)
{
    static char **cpp;

    cpp = (char_pp) opt->opt_data;

    if (1 >= argc)
	return FALSE;

    *cpp = argv [1];
    argv [0] = NULL;
    argv [1] = NULL;

	return TRUE;
}

