#include <stdio.h>
#include <stdlib.h>
#include "read_dfa_from_file.h"
#include "dfa_reader.h"

// Returns 1 if EOF encountered. It dynamically expands the length of the line
int get_line_from_file(FILE *file, char **line_ptr, int *len);

struct DFA *read_dfa_from_file(const char *filename, int enabled_error_printing) {

    // Handle openning the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        if (enabled_error_printing) {
            printf(
                "error while opening the file \"%s\": "
                "make sure the file exists.\n", filename
            );
        }
        return NULL;
    }

    // Instantiate DFA Reader
    struct DFA_Reader *dfa_reader = make_DFA_reader();

    // Instantiate memory for line-by-line fetching
    int line_length = 30;
    char *line = (char*) calloc(line_length, sizeof(char));
    int eof_reached = 0;

    // Keep fetching the line & processing it by the DFA Reader
    // until EOF is reached or DFA reader error has been encountered 
    while (!eof_reached && !has_error(dfa_reader)) {
        eof_reached = get_line_from_file(file, &line, &line_length);
        read_DFA_line(dfa_reader, line);
    }
    
    // Close the file & free memory used for line-by-line fetching
    free(line);
    fclose(file);
    
    struct DFA *dfa = finish_and_get_DFA(dfa_reader);

    if (has_error(dfa_reader) && enabled_error_printing) {
        printf("%s\n", get_error(dfa_reader));
    }

    delete_DFA_reader(dfa_reader);
    return dfa;
}

int get_line_from_file(FILE *file, char **line_ptr, int *line_length) {
    char *line = *line_ptr;
    int done = 0;
    int i = 0;
    int is_eof_reached = 0;

    while (!done) {
        // dynamically increase the line length when needed
        if (i >= *line_length) {
            *line_length = *line_length * 2;
            line = realloc(line, *line_length);
            *line_ptr = line;
        }

        int c = fgetc(file);
        is_eof_reached = c == EOF ? 1 : 0;

        // If EOF or \n reached, finish reading the current line
        if (c == EOF || c == '\n') {
            line[i++] = 0;
            done = 1;
        } else {
            line[i++] = c;
        }
    }

    return is_eof_reached;
}