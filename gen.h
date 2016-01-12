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

extern void gen_init (unsigned int address);
extern void gen_org (unsigned int address);
extern void gen_exec (unsigned int address);
extern void genbyte (int data);
extern void genword (int data);
extern void genbytebyte (byte data1, byte data2);
extern void genbyteword (byte data1, int data2);
extern void genwordword (int data1, int data2);
extern void genstring (const char *ptr);
extern void dumpcode (void);
extern void flushcode (void);
extern void fputmw (FILE *fp, unsigned int word);
extern void gen_open (const char *filename);
extern void gen_close (void);
extern void dumpblock (void);
