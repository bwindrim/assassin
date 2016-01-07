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

extern void insert_symbol ();
extern Obj_ptr find_local_sym ();
extern void create_block ();
extern void open_block ();
extern void end_block ();
extern void insert_symbol ();
extern void add_symbol ();
extern Obj_ptr add_macro ();
extern Obj_ptr find_global_sym ();
extern Obj_ptr find_name_sym ();
extern int symbol_val ();
extern void redef_sym ();
