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

extern void outline (char	*linebuffer,
		 const char    *fname,
		 int      lno);
extern void parse (char	*linebuffer,
	       FILE	*inp,
	       const char    *fname,
	       int      lno);
extern void parse_stream (FILE	*fp,
		      const char    *fname);
extern void parse_file (const char *filename);
