typedef char **char_pp;

struct opt_rec
{
	char *opt_name;
	int (*opt_func) ();
	char *opt_data;
};

extern int
    parse_opt (),
    noarg_opt (),
    starg_opt ();

