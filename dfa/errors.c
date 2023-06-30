#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum DFA_Creation_Errors {
    FILE_DOES_NOT_EXIST,
} DFA_Creation_Errors;

const char *error_prefix = "error while reading DFA";

char *could_not_open_file_error(const char *filename) {
    char *error = malloc(200);
    snprintf(error, 200, "%s: could not open the file %s", error_prefix, filename);
    return error;
}

char *dfa_reader_regex_error() {
    char *error = malloc(100);
    snprintf(error, 100, "DFA reader instantation error happened: regex compilation failed");
    return error;
}

char *dfa_reader_bad_line_error(unsigned line_number, const char *line, const char *expected) {
    int size = 100 + strlen(error_prefix) + strlen(line) + strlen(expected);
    char *error = malloc(size);
    snprintf(
        error,
        size,
        "%s on line #%u: could not interpret line \"%s\"; expected %s",
        error_prefix,
        line_number,
        line,
        expected
    );
    return error;
}

char *dfa_reader_bad_number_error(unsigned line_number, const char *num, const char *purpose, const char *reason) {
    int size = 100 + strlen(error_prefix) + strlen(num) + strlen(reason);
    char *error = malloc(size);
    snprintf(error, size, "%s on line #%u: bad number \"%s\" for %s: %s", error_prefix, line_number, num, purpose, reason);
    return error;
}

char *dfa_reader_state_out_of_bounds_error(unsigned line_number, unsigned state_id, unsigned number_of_states) {
    char *valid_range_explanation = malloc(100);
    if (number_of_states == 1) {
        snprintf(valid_range_explanation, 100, "Only valid state ID is 0");
    } else {
        snprintf(valid_range_explanation, 100, "Valid state IDs are numbers from 0 to %d (inclusive)", number_of_states-1);
    }
    int size = 100 + strlen(valid_range_explanation) + strlen(error_prefix);
    char *error = malloc(size);
    snprintf(error, size, "%s on line #%u: state %d is out of bounds. %s", error_prefix, line_number, state_id, valid_range_explanation);
    free(valid_range_explanation);
    return error;
}

char *dfa_reader_repeating_final_state_error(unsigned line_number, unsigned repeating_final_state_id) {
    int size = 240 + strlen(error_prefix);
    char *error = malloc(size);
    snprintf(
        error, size,
        "%s on line %u: while reading final states it was detected that state with ID %d repeats. %s",
        error_prefix, line_number, repeating_final_state_id,
        "Please ensure that there are no repeating final state IDs."
    );
    return error;
}

char *dfa_reader_conflicting_transitions_error(
    unsigned t1_line_number,
    unsigned t2_line_number,
    unsigned t1_destination_state,
    unsigned t2_destination_state,
    unsigned origin_state,
    char transition_letter
) {
    int size = 400 + strlen(error_prefix);
    char *error = malloc(size);
    snprintf(
        error, size,
        "%s: while reading transitions a conflict was detected. "
        "Transitions defined on lines %d and %d are conflicting. "
        "They have the same origin state %d and transition letter '%c', "
        "but differing destination states: namely %d and %d.",
        error_prefix,
        t1_line_number, t2_line_number,
        origin_state, transition_letter,
        t1_destination_state, t2_destination_state
    );
    return error;
}