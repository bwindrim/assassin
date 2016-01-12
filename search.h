extern char skipspace (char **ptr);
extern char *next_field (void);
extern char *strend (const char *ptr);
extern char *strip_quotes (char *ptr);
extern bool check_name (const char *ptr);
extern char *bound_str (const char *start, const char *end);
extern char *strupper (char *string);
extern int safecmp (const char *s1, const char *s2);
