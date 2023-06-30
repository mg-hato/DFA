#include "dfa.h"

// Reads DFA from a file. Returns a pointer to DFA if everything is ok, otherwise NULL pointer.
// Pass a non-zero integer for the second parameter to enable printing of errors (in case of any) 
struct DFA *read_dfa_from_file(const char *filename, int enabled_error_printing);