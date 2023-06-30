#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "errors.h"
#include "dfa_reader.h"


typedef enum DFA_Reader_State { STATE_NUMBER, FINAL_STATES, TRANSITIONS } DFA_Reader_State;

typedef struct Transition {
    unsigned origin, destination;
    char letter;

    unsigned line_number; // meta information for error messages
} Transition;

typedef struct DFA_Reader {
    char *error_message;
    enum DFA_Reader_State state;

    unsigned line_number;

    regex_t empty_re;

    regex_t number_of_states_re;
    
    regex_t final_states_none_re;
    regex_t final_states_re;
    
    regex_t transition_re;

    unsigned number_of_states;

    unsigned *final_states;
    unsigned final_states_count;

    Transition *transitions;
    unsigned transition_count;
} DFA_Reader;


// This structure is used to keep track of all the allocated memory that needs to be deallocated by the invoker of function.
// The invoker can deallocate this memory using the "free_deallocation_array" function.
typedef struct DeallocationArray {
    unsigned size;
    void **deallocation_array;

    unsigned regexes_size;
    regex_t **regexes;
} DeallocationArray;

DeallocationArray make_deallocation_array();
void free_deallocation_array(DeallocationArray d_arr);
DeallocationArray register_memory_for_deallocation(DeallocationArray d_arr, void *ptr);
DeallocationArray register_regex_for_deallocation(DeallocationArray d_arr, regex_t *regex_ptr);

const char *LEADING_ZEROES_REASON = "the number should not have leading zeroes";

// Helper functions declarations 

int is_line_empty(const DFA_Reader *reader, const char *line);
DeallocationArray parse_number_of_states(DFA_Reader *reader, const char *line);
DeallocationArray parse_final_states(DFA_Reader *reader, const char *line);
DeallocationArray parse_transition(DFA_Reader *reader, const char *line);

// Extracts substring: source[start..end)
char *extract_str(const char *source, int start, int end);

char *extract_match(const char *source, regmatch_t match);

// Returns 1 if the string has leading zeroes 
int is_valid_non_negative_number(const char* num);

// Definitions of functions from "dfa_reader.h"

DFA_Reader *make_DFA_reader() {
    DFA_Reader *reader = (DFA_Reader*) malloc(sizeof(DFA_Reader));
    reader->error_message = NULL;
    reader->state = STATE_NUMBER;
    reader->line_number = 0;

    reader->number_of_states = 0;

    reader->final_states = NULL;
    reader->final_states_count = 0;

    reader->transitions = NULL;
    reader->transition_count = 0;
    
    int regex_errors = 0;
    
    regex_errors = regex_errors | regcomp(&reader->empty_re, "^\\s*$", REG_EXTENDED);
    regex_errors = regex_errors | regcomp(&reader->number_of_states_re, "^\\s*([0-9]+)\\s*$", REG_EXTENDED);
    regex_errors = regex_errors | regcomp(&reader->final_states_re, "^(\\s*[0-9]+)+\\s*$", REG_EXTENDED);
    regex_errors = regex_errors | regcomp(&reader->final_states_none_re, "^\\s*NONE\\s*$", REG_EXTENDED);
    regex_errors = regex_errors | regcomp(&reader->transition_re, "^\\s*([0-9]+)\\s*->\\s*([0-9]+)\\s*:\\s*(\\S)\\s*$", REG_EXTENDED);
    
    if (regex_errors) {
        reader->error_message = dfa_reader_regex_error();
    }
    
    return reader;
}

void delete_DFA_reader(DFA_Reader *reader) {
    if (reader != NULL) {
        if (reader->error_message != NULL) {
            free(reader->error_message);
            reader->error_message = NULL;
        }

        if (reader->final_states != NULL) {
            free(reader->final_states);
            reader->final_states = NULL;
        }

        if (reader->transitions != NULL) {
            free(reader->transitions);
            reader->transitions = NULL;
        }

        regfree(&reader->empty_re);
        regfree(&reader->number_of_states_re);
        regfree(&reader->final_states_none_re);
        regfree(&reader->final_states_re);
        regfree(&reader->transition_re);
        free(reader);
    }
}

int has_error(const DFA_Reader *reader) {
    return reader->error_message != NULL ? 1 : 0;
}

const char *get_error(const DFA_Reader *reader) {
    return reader->error_message;
}

int can_make_DFA(const DFA_Reader *reader) {
    return has_error(reader) == 0 && reader->state == TRANSITIONS ? 1 : 0;
}

struct DFA *finish_and_get_DFA(const DFA_Reader* reader) {
    if (can_make_DFA(reader) != 1) {
        return NULL;
    }

    struct DFA *dfa = make_DFA(reader->number_of_states);
    
    // Enrich DFA with the final states
    for (int i = 0; i < reader->final_states_count; i++) {
        mark_state_as_final(dfa, reader->final_states[i]);
    }

    // Enrich DFA with transitions
    for (int i = 0; i < reader->transition_count; i++) {
        Transition t = reader->transitions[i];
        add_transition(dfa, t.origin, t.destination, t.letter);
    }

    return dfa;
}

void read_DFA_line(DFA_Reader *reader, const char *line) {
    if (has_error(reader)) {
        return;
    }

    ++reader->line_number;

    // Skip empty line
    if (is_line_empty(reader, line)) {
        return;
    }

    switch (reader->state)
    {
    case STATE_NUMBER:
        free_deallocation_array(parse_number_of_states(reader, line));
        break;
    case FINAL_STATES:
        free_deallocation_array(parse_final_states(reader, line));
        break;
    case TRANSITIONS:
        free_deallocation_array(parse_transition(reader, line));
        break;
    }
}


// Helper functions definitions

int is_line_empty(const DFA_Reader *reader, const char *line) {
    return regexec(&reader->empty_re, line, 0, NULL, 0) == 0 ? 1 : 0;
}


DeallocationArray parse_number_of_states(DFA_Reader *reader, const char *line) {
    DeallocationArray d_arr = make_deallocation_array();
    reader->state = FINAL_STATES;

    // If number of states regular expression did not match
    if (regexec(&reader->number_of_states_re, line, 0, NULL, 0)) {
        reader->error_message = dfa_reader_bad_line_error(reader->line_number, line, "number of state");
        return d_arr;
    }

    regmatch_t groups[2];
    regexec(&reader->number_of_states_re, line, 2, groups, 0);
    char *extracted_number = extract_str(line, groups[1].rm_so, groups[1].rm_eo);
    d_arr = register_memory_for_deallocation(d_arr, extracted_number);

    // If the number is invalid, register error and return
    if (is_valid_non_negative_number(extracted_number) == 0) {
        reader->error_message = dfa_reader_bad_number_error(reader->line_number, extracted_number, "number of states", LEADING_ZEROES_REASON);
        return d_arr;
    }

    // Check that the number is valid (i.e. greater than zero) and return
    reader->number_of_states = strtoul(extracted_number, NULL, 10);
    if (reader->number_of_states == 0) {
        reader->error_message = dfa_reader_bad_number_error(reader->line_number, extracted_number, "number of states", "must be greater than 0");
    }
    return d_arr;
}

DeallocationArray parse_final_states(DFA_Reader *reader, const char *line) {
    DeallocationArray d_arr = make_deallocation_array();

    int outcome_for_none = regexec(&reader->final_states_none_re, line, 0, NULL, 0);
    int outcome_for_fs = regexec(&reader->final_states_re, line, 0, NULL, 0);

    if (outcome_for_fs != 0 && outcome_for_none != 0) {
        reader->error_message = dfa_reader_bad_line_error(reader->line_number, line, "final states descriptor");
        return d_arr;
    }

    reader->state = TRANSITIONS;
    if (outcome_for_none == 0) {
        return d_arr;
    }

    reader->final_states = malloc(0);

    regex_t *number_re = malloc(sizeof(regex_t));
    regcomp(number_re, "[0-9]+", REG_EXTENDED);
    d_arr = register_regex_for_deallocation(d_arr, number_re);


    int start_offset = 0;
    regmatch_t groups[1];
    
    while (!regexec(number_re, &line[start_offset], 1, groups, 0)) {
        char *extracted_num = extract_match(&line[start_offset], groups[0]);
        d_arr = register_memory_for_deallocation(d_arr, extracted_num);
        
        // If final state number is not valid
        if (!is_valid_non_negative_number(extracted_num)) {
            reader->error_message = dfa_reader_bad_number_error(reader->line_number, extracted_num, "final state", LEADING_ZEROES_REASON);
            return d_arr;
        } 
        
        unsigned state_id = strtoul(extracted_num, NULL, 10);
        // If final state is out of bounds
        if (reader->number_of_states <= state_id) {
            reader->error_message = dfa_reader_state_out_of_bounds_error(reader->line_number, state_id, reader->number_of_states);
            return d_arr;
        }

        // Otherwise, add final state
        reader->final_states = realloc(reader->final_states, ++reader->final_states_count * sizeof(unsigned));
        reader->final_states[reader->final_states_count-1] = state_id;
        start_offset += groups[0].rm_eo;
    }

    // Before completing, ensure that there are no repeating states in the final states sequence
    for (int i = 0; i < reader->final_states_count; i++) {
        for (int k = i + 1; k < reader->final_states_count; k++) {
            if (reader->final_states[i] == reader->final_states[k]) {
                reader->error_message = dfa_reader_repeating_final_state_error(reader->line_number, reader->final_states[i]);
                return d_arr;
            }
        }
    }

    return d_arr;
}


DeallocationArray parse_transition(DFA_Reader *reader, const char *line) {
    DeallocationArray d_arr = make_deallocation_array();
    if (regexec(&reader->transition_re, line, 0, NULL, 0)) {
        reader->error_message = dfa_reader_bad_line_error(reader->line_number, line, "a valid transition descriptor");
        return d_arr;
    }
    
    regmatch_t groups[4];
    regexec(&reader->transition_re, line, 4, groups, 0);

    char *origin = extract_match(line, groups[1]);
    char *destination = extract_match(line, groups[2]);
    char *letters = extract_match(line, groups[3]);
    d_arr = register_memory_for_deallocation(d_arr, origin);
    d_arr = register_memory_for_deallocation(d_arr, destination);
    d_arr = register_memory_for_deallocation(d_arr, letters);

    // Check that the "origin" state in the transition has no leading zeroes
    if (!is_valid_non_negative_number(origin)) {
        reader->error_message = dfa_reader_bad_number_error(reader->line_number, origin, "origin state of transition", LEADING_ZEROES_REASON);
        return d_arr;
    }
    
    unsigned origin_state_id = strtoul(origin, NULL, 10);

    // Check that the "origin" state in the transition is within state ID bounds
    if (reader->number_of_states <= origin_state_id) {
        reader->error_message = dfa_reader_state_out_of_bounds_error(reader->line_number, origin_state_id, reader->number_of_states);
        return d_arr;
    }

    // Check that the "destination" state in the transition has no leading zeroes
    if (!is_valid_non_negative_number(destination)) {
        reader->error_message = dfa_reader_bad_number_error(reader->line_number, destination, "destination state of transition", LEADING_ZEROES_REASON);
        return d_arr;
    }
    
    unsigned destination_state_id = strtoul(destination, NULL, 10);

    // Check that the "destination" state in the transition is within state ID bounds
    if (reader->number_of_states <= destination_state_id) {
        reader->error_message = dfa_reader_state_out_of_bounds_error(reader->line_number, destination_state_id, reader->number_of_states);
        return d_arr;
    }

    Transition t;
    t.origin = origin_state_id;
    t.destination = destination_state_id;
    t.letter = letters[0];
    t.line_number = reader->line_number;

    // Check that this transition does not have a conflict with any other previously added transition
    // Conflict is when the two transitions have same origin state and transition letter, but different destination state.
    // Note that this conflict only applies to DFA, but not NFA.
    for (int i = 0; i < reader->transition_count; i++) {
        
        // Same origin & letter -- this is either a conflict or a duplication
        // - If it is a conflict we register the error
        // - If it is a duplication, we ignore this transition as it is already defined
        // Either way we can stop the check and return.
        if (t.origin == reader->transitions[i].origin && t.letter == reader->transitions[i].letter) {

            // If it is a conflict, raise an error
            if (t.destination != reader->transitions[i].destination) {
                Transition prev = reader->transitions[i];
                reader->error_message = dfa_reader_conflicting_transitions_error(
                    prev.line_number, t.line_number, prev.destination, t.destination, t.origin, t.letter
                );
            }
            return d_arr;
        }
    }

    // All ok, add transition
    reader->transitions = realloc(reader->transitions, ++reader->transition_count * sizeof(Transition));
    reader->transitions[reader->transition_count - 1] = t;

    return d_arr;
}

// Helper functions definitions

char *extract_match(const char *source, regmatch_t match) {
    return extract_str(source, match.rm_so, match.rm_eo);
}

char *extract_str(const char *source, int start, int end) {
    unsigned length = end - start;
    char *extracted_string = (char*) calloc(length + 10, sizeof(char));
    strncpy(extracted_string, source + start, length);
    return extracted_string;
}

int is_valid_non_negative_number(const char* num) {
    regex_t regex;
    regcomp(&regex, "^(0|[1-9][0-9]*)$", REG_EXTENDED);
    int is_valid = regexec(&regex, num, 0, NULL, 0) == 0 ? 1 : 0;
    regfree(&regex);
    return is_valid;
}

// Deallocation array functions

DeallocationArray make_deallocation_array() {
    DeallocationArray d_arr;

    d_arr.size = 0;
    d_arr.deallocation_array = NULL;

    d_arr.regexes_size = 0;
    d_arr.regexes = NULL;
    
    return d_arr;
}

void free_deallocation_array(DeallocationArray d_arr) {
    
    for (unsigned i = 0; i < d_arr.size; i++) {
        free(d_arr.deallocation_array[i]);
    }

    for (unsigned i = 0; i < d_arr.regexes_size; i++) {
        regfree(d_arr.regexes[i]);
        free(d_arr.regexes[i]);
    }
    if (d_arr.deallocation_array != NULL) {
        free(d_arr.deallocation_array);
    }
    if (d_arr.regexes != NULL) {
        free(d_arr.regexes);
    }
}

DeallocationArray register_memory_for_deallocation(DeallocationArray d_arr, void *ptr) {
    d_arr.deallocation_array = (void**) realloc(d_arr.deallocation_array, (++d_arr.size) * sizeof(void*));
    d_arr.deallocation_array[d_arr.size-1] = ptr;
    return d_arr;
}

DeallocationArray register_regex_for_deallocation(DeallocationArray d_arr, regex_t *regex_ptr) {
    d_arr.regexes = (regex_t**) realloc(d_arr.regexes, (++d_arr.regexes_size) * sizeof(regex_t*));
    d_arr.regexes[d_arr.regexes_size-1] = regex_ptr;
    return d_arr;
}