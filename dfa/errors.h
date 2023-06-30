
char *could_not_open_file_error(const char *filename);
char *dfa_reader_regex_error();
char *dfa_reader_bad_line_error(unsigned line_number, const char *line, const char *expected);
char *dfa_reader_bad_number_error(unsigned line_number, const char *num, const char *purpose, const char *reason);
char *dfa_reader_state_out_of_bounds_error(unsigned line_number, unsigned state_id, unsigned number_of_states);
char *dfa_reader_repeating_final_state_error(unsigned line_number, unsigned repeating_final_state_id);
char *dfa_reader_conflicting_transitions_error(unsigned line_num1, unsigned line_num2, unsigned dest1, unsigned dest2, unsigned origin, char letter);