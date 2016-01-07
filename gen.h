/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *    gen.h - header module	*
 *				*
 ********************************/

extern byte		genbuffer [GEN_BUFF_SIZE];
/*
extern FILE		*gen_file;
*/

extern byte_ptr	genptr;

extern void gen_init ();
extern void gen_org ();
extern void gen_exec ();
extern void genbyte ();
extern void genword ();
extern void genbytebyte ();
extern void genbyteword ();
extern void genwordword ();
extern void genstring ();
extern void dumpcode ();
extern void flushcode ();
extern void fputmw ();
extern void gen_open ();
extern void gen_close ();
extern void dumpblock ();
