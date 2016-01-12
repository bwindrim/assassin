/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *  symbol.h - header module	*
 *				*
 ********************************/

struct	object
      {
	char	type;
	struct 	object	*next;
	char	*name;

	union
		{
		  struct
			{
			  struct 	object	*contents;
			  struct	object	*insert;
			} blk;

		  struct
			{
			  int	value;
			} sym;

		  struct
			{
			  int	  no_params;
			  struct  macline *lines;
			} mac;
		} var;
      };

typedef	struct	object	Object, *Obj_ptr;

extern Object	root_block;

extern Obj_ptr
	block_stack [BLOCK_STACK_SIZE],
	*block_ptr;

extern void insert_symbol (char	stype,
char	*sname,
Obj_ptr	new_symbol,
Obj_ptr	block);
extern Obj_ptr find_local_sym (const char	*start,
    const char        *end,
    int		stype,
    Obj_ptr     block);
extern void create_block (const char *block_name);
extern void open_block (const char *block_name);
extern void end_block (void);
extern void insert_symbol (char	stype,
char	*sname,
Obj_ptr	new_symbol,
Obj_ptr	block);
extern void add_symbol (char	stype,
     char	*sname,
     int	svalue,
     Obj_ptr	block);
extern Obj_ptr add_macro (char	*sname,
		   int	params,
		   Obj_ptr	block);
extern Obj_ptr find_global_sym (const char	*start,
    const char        *end,
    char	stype,
    Obj_ptr     block);
extern Obj_ptr find_name_sym (const char	*start,
		       const char     *end,
		       char	stype);
extern int symbol_val (const char *start,
    const char *end);
extern void redef_sym (char	stype,
		    const char   *sname,
		    int	svalue);
