typedef char **char_pp;

struct opt_rec
{
	char *opt_name;
	int (*opt_func) ();
	char *opt_data;
};

extern int parse_opt (struct opt_rec table [],
	       int size,
	       int *argc,
	       char *argv []
		      );
extern int noarg_opt (struct opt_rec *opt,
		      int argc,
		      char *argv []
		      );
extern int starg_opt (struct opt_rec *opt,
		      int argc,
		      char *argv []
		      );

