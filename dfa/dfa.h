struct DFA;
enum Outcome;

struct DFA* make_DFA(unsigned number_of_states);
void delete_DFA(struct DFA*);

// Returns 1 if the input is ACCEPTED, 0 if it is REJECTED
int run_DFA(struct DFA*, const char* input, unsigned length);

void add_transition(struct DFA*, unsigned origin, unsigned destination, char letter);
void mark_state_as_final(struct DFA*, unsigned state_id);