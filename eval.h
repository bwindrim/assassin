/********************************
 *				*
 *   Assassin - 6809 assembler	*
 *				*
 *     Brian Windrim 6/11/88	*
 *				*
 *   eval.h - header module	*
 *				*
 ********************************/

extern int eval_str (const char *str);
extern int eval     (const char *start, const char *str);
extern int dec_conv (const char *start, const char *end);
extern int hex_conv (const char *start, const char *end);
extern int bin_conv (const char *start, const char *end);
extern int chr_conv (const char *start, const char *end);
