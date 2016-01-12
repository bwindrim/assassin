/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *  optype.h - header module	*
 *				*
 ********************************/

extern void type1 (unsigned int code, unsigned int mode);
extern int type2 (int code, int nextw, char ch, int oplen);
extern void typeshift (int rcode, int lcode, int nextw, int oplen);
extern void typelea (int code);
extern void typemisc(int code);
extern void typeswi (int number);
extern void typecc (int code);
extern void typepspl (int code);
extern void typesc (int code);
extern void typext (int code);
extern void typebr (unsigned int nextw, unsigned int lso);
