#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dfa/read_dfa_from_file.h"

void clear_screen() {
    printf("\e[1;1H\e[2J");
}

char clear_input(char c) {
    while (c != '\n') {
        c = getchar();
    }
    return c;
}

void bad_option() {
    printf("\nBad option... try again\n\n");
}

void print_menu(int selected, int last_option) {
    clear_screen();
    switch (last_option)
    {
    case -1: // Bad option
        bad_option();
        break;
    
    default:
        break;
    }
    printf(
        "Options:\n"
        "1 -- switch DFA\n"
        "2 -- enter a word\n"
        "3 -- quit\n"
        "\n"
        "  Currently selected DFA: #%d\n"
        "> ",
        selected
    );
}

struct DFA *select_different_dfa(char *c_ptr, struct DFA *dfa, int *selected) {
    char c = *c_ptr;
    c = clear_input(c);
    clear_screen();
    printf("Select DFA (1, 2 or 3): ");
    c = getchar();
    if ('1' <= c && c <= '3') {
        char filename[60];
        snprintf(filename, 60, "example-%c.txt", c); 
        struct DFA *new_dfa = read_dfa_from_file(filename, 1);
        if (new_dfa != NULL) {
            *selected = c - '0';
            delete_DFA(dfa);
            dfa = new_dfa;
            printf("\nSuccessfully loaded... \n");
        } else {
            printf("\nLoading new DFA failed...\n");
        }
    } else {
        printf("\nInvalid selection...\n");
    }

    c = clear_input(c);

    printf("\n\nPress ENTER to continue... ");
    c = getchar();
    *c_ptr = clear_input(c);
    return dfa;
}

void run_word(char *c_ptr, struct DFA *dfa) {
    char c = *c_ptr;
    c = clear_input(c);
    int word_size = 50;
    int i = 0;
    char *word = malloc(word_size);

    clear_screen();
    printf("Enter your word: ");

    int done = 0;
    while (!done) {
        if (i >= word_size) {
            word_size *= 2;
            word = realloc(word, word_size);
        }
        c = getchar();
        word[i++] = c == '\n' ? 0 : c;
        done = c == '\n' ? 1 : 0;
    }
    printf(
        "\n\nWord \"%s\" is %s by the DFA...\n" , word,
        run_DFA(dfa, word, strlen(word)) == 1 ? "ACCEPTED" : "REJECTED"
    );

    free(word);
    printf("\nPress ENTER to continue...  ");
    c = getchar();
    *c_ptr = clear_input(c);
}


int main() {
    int selected = 1;
    struct DFA *dfa = read_dfa_from_file("./example-1.txt", 1);
    int done = 0;
    char c = '\n';
    int option = 0;
    do {
        c = clear_input(c);
        print_menu(selected, option);
        c = getchar();
        if ('1' <= c && c <= '3') option = c - '0';
        else option = -1;

        switch (c)
        {
        case '1':
            dfa = select_different_dfa(&c, dfa, &selected);
            break;
        case '2':
            run_word(&c, dfa);
            break;
        case '3':
            done = 1;
            break;
        }
    } while(done == 0);
    
    delete_DFA(dfa);
    return 0;
}