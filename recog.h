/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   recog.h - header module	*
 *				*
 ********************************/

extern char
	*curr_name,
	*op_str,
	*addr_str;

extern void outline ();
extern void parse ();
extern void parse_stream ();
extern void parse_file ();
