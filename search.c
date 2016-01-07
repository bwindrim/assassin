#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>

#include "const.h"
#include "typedefs.h"
#include "err.h"
#include "globals.h"
#include "recog.h"

#include "search.h"

char skipspace (ptr)
	char **ptr;
{
  while (isspace (**ptr))
    ++*ptr;

  return(**ptr);
}

char	*next_field ()
{
  static char	*ptr, *ret_ptr;
  static bool	quote_flag;

  skipspace (&addr_str);
  ret_ptr = addr_str;

  if (!*addr_str)
    return (NULL);

  for (quote_flag = FALSE;
       *addr_str && (quote_flag || *addr_str != COMMA);
       ++addr_str)
	 {
	  if (2 != quote_flag && SINGLEQUOTE == *addr_str)
	    quote_flag ^= 1;
	  else if (1 != quote_flag && DOUBLEQUOTE == *addr_str)
	    quote_flag ^= 2;
	}

  for (ptr = addr_str;
       isspace (ptr [-1]);
       --ptr) ;

  if (*addr_str) ++addr_str;

  *ptr = NUL;

  return (ret_ptr);
}

char	*strend (ptr)
	char	*ptr;
{
	while (*ptr) ++ptr;

	return (ptr);
}

char	*strip_quotes (ptr)
	char	*ptr;
{
  static char	*end;

  if (DOUBLEQUOTE == *ptr)
  {
	end = strend (++ptr) - 1;

	if (DOUBLEQUOTE == *end)
		*end = NUL;
	else
		ptr = NULL;
  }

  return (ptr);
}

bool	check_name (ptr)
	char	*ptr;
{
  if (!*ptr || isdigit (*ptr))
    return (FALSE);

  do
    {
      if (!isalnum (*ptr)
	  && '_' != *ptr
	  && '?' != *ptr
	  && '.' != *ptr
	  && ':' != *ptr)
	return (FALSE);
    } while (*++ptr);

  return (TRUE);
}

char	*bound_str (start, end)
	char	*start,
		*end;
{
  static char buffer [TEXT_BUFF_SIZE];
  static char *ptr;

  for (ptr = buffer;
       start < end;
       *ptr++ = *start++) ;

  *ptr = NUL;

  return (buffer);
}

char	*strupper (string)
	char	*string;
{
  static char	*ptr;

  for (ptr = string;
       *ptr;
       ++ptr)
      if (islower (*ptr))
       *ptr = toupper (*ptr);

  return (string);
}

/*
char	*strchange (string, func)
	char	*string;
	char	*(*func) ()
{
  static char	*ptr;

  for (ptr = string;
       *ptr;
       *ptr = (*func) (*ptr)) ;

  return (string);
}

char	*strdup (string)
	char	*string;
{
  return malloc (strlen (string) + 1);
}
*/
/*
 * Safe strcmp function, doesn't mind NULL ptrs
 */
int safecmp (s1, s2)
char *s1, *s2;
{
    if (NULL == s1)
	if (NULL == s2)
	    return 0;
	else
	    return 1;
    else
	if (NULL == s2)
	    return -1;
	else
	    return strcmp (s1, s2);
}

