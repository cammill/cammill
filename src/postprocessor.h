

void postcam_var_push_double (char *name, double value);
void postcam_var_push_int (char *name, int value);
void postcam_var_push_string (char *name, char *value);
void postcam_comment (char *value);
void postcam_call_function (char *name);
void postcam_init_lua (const char* path, char *plugin);
void postcam_exit_lua (void);
