#include <regex.h>
#include "dfa.h"

struct DFA_Reader;

// Instantiates DFA reader 
struct DFA_Reader *make_DFA_reader();

// Deletes DFA reader and all memory it has occupied
void delete_DFA_reader(struct DFA_Reader*);

// Returns 1 if the reader is in error, otherwise returns 0.
int has_error(const struct DFA_Reader*);

// Returns error message. If no errors occurred the message points to NULL
const char *get_error(const struct DFA_Reader*);

// Returns 1 if the information read so far can produce a DFA
int can_make_DFA(const struct DFA_Reader*);

// Returns a DFA based on the lines read.
// If DFA cannot be constructed based on the lines provided (either because some information is missing or because error was detected)
// NULL is returned.
struct DFA *finish_and_get_DFA(const struct DFA_Reader*);

// DFA Reader reads a line that describes DFA
void read_DFA_line(struct DFA_Reader*, const char *line);
