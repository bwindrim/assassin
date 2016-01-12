/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 15/10/89	*
 *				*
 *   macro.h - header module	*
 *				*
 ********************************/

struct	macline
{
    struct macline *next_line;
    char	    text [1];
};

typedef struct  macline	*Line_ptr;

extern void def_macro (char *linebuffer, FILE *inp, const char *fname, int lno);
extern Obj_ptr get_macro (const char *name);
extern void exp_macro (Obj_ptr macro, const char *fname, int lno);
