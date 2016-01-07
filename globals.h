/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   globals.h - header module	*
 *				*
 ********************************/

extern bool
	forward_sym,
	gen,
	list,
	out_diff;

extern int
	gen_flag,
	list_flag,
	exp_flag,
	debug;

extern char
	*obj_name;

extern char
	errfrmstr  [],
	err2frmstr [],
	filerrstr  [];

extern int
		val,
		err_count,
		pass,
		lcount;

extern unsigned
	genaddr, nextaddr,
	load_addr,
	exe_addr,
	code_length;

extern int		error, hack_error;

extern jmp_buf		termbuf;


