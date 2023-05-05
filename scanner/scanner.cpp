#include "added.h"
#include <iostream>
#include <fstream>
#include <cstdio>

static int DFA[80][128];

static struct record_struct record{size: 0};

int main(int argc, char const *argv[]) {
    std::remove("token_list.txt");
    initialize_DFA(DFA);

    if (argc < 2) {
        std::cout << "please enter the target file name" << std::endl;
        return 0;
    }

    std::ifstream infile;
    infile.open(argv[1]);
    char current_char;
    char next_char = infile.get();
    int current_state = 0;
    int next_state;
    while (next_char != EOF) {
        current_char = next_char;
        next_char = infile.get();
        if ((current_state == 0 && current_char == '\n') || (current_state == 0 && current_char == ' ')) {
            continue;
        }

        int temp_char = current_char;
        // the scanner will refect any character that is not within 0-127
        if (temp_char > 127) {
            std::cout << "syntax error: " << current_char << std::endl;
            break;
        }
        current_state = DFA[current_state][temp_char];

        if (current_state == 0) {
            std::cout << "syntax error: " << current_state << ' ' << current_char << std::endl;
            break;
        }
        record.rec[record.size] = current_char;
        record.size = record.size + 1;

        temp_char = next_char;
        if (next_char == EOF) temp_char = 32;
        next_state = DFA[current_state][temp_char];

        if (next_state < 0) {
            get_token(next_state, record);
            current_state = 0;
            record.size = 0;
        }
    }

    std::cout << "Scanner finished, token list saved in file token_list.txt" << std::endl;
    infile.close();
}