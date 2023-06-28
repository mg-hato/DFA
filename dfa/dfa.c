#include <stdlib.h>
#include "dfa.h"

const int ALPHABET_SIZE = 256;

typedef struct State {
    unsigned id;
    int final;
    unsigned* transitions;
} State;

typedef struct DFA {
    struct State* states;
    unsigned number_of_states;
} DFA;

State make_state(unsigned id);
void delete_state(State state);

void set_all_transitions_to_garbage_state(DFA* dfa) {
    unsigned garbage_state_id = dfa->number_of_states - 1;
    for(unsigned state_id = 0; state_id < dfa->number_of_states; state_id++) {
        for(unsigned i = 0; i < ALPHABET_SIZE; i++) {
            dfa->states[state_id].transitions[i] = garbage_state_id;
        }
    }
}

DFA* make_DFA(unsigned number_of_states) {
    DFA* dfa = (DFA*) malloc(sizeof(DFA));
    dfa->number_of_states = number_of_states + 1;
    dfa->states = (struct State*) malloc(dfa->number_of_states * sizeof(struct State));

    for(unsigned i = 0; i < dfa->number_of_states; i++) {
        dfa->states[i] = make_state(i);
    }

    set_all_transitions_to_garbage_state(dfa);
    return dfa;
}


void delete_DFA(DFA* dfa) {
    if (dfa != NULL) {
        for(unsigned i = 0; i < dfa->number_of_states; i++) {
            delete_state(dfa->states[i]);
        }
        free(dfa);
    }
}

int run_DFA(DFA* dfa, const char* input, unsigned length) {
    unsigned current_state_id = 0;
    for(unsigned i = 0; i < length; i++) {
        current_state_id = dfa->states[current_state_id].transitions[input[i]];
    }
    return dfa->states[current_state_id].final == 0 ? 0 : 1;
}


void add_transition(DFA* dfa, unsigned origin, unsigned destination, char letter) {
    dfa->states[origin].transitions[letter] = destination;
}

void mark_state_as_final(DFA* dfa, unsigned state_id) {
    dfa->states[state_id].final = 1;
}

State make_state(unsigned id) {
    State new_state;

    new_state.id = id;
    new_state.final = 0;
    new_state.transitions = malloc(ALPHABET_SIZE * sizeof(unsigned));
    
    return new_state;
}

void delete_state(State state) {
    free(state.transitions);
}