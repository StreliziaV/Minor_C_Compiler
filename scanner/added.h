#ifndef _ADDED_H_
#define _ADDED_H_

struct record_struct {
    int size;
    char rec[100];
};

int hello_world();

void initialize_DFA(int (&DFA)[80][128]);

void get_token(int token_no, struct record_struct & record);

#endif