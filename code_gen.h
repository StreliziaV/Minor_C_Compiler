#ifndef _CODE_GEN_H_
#define _CODE_GEN_H_

#include <iostream>
#include <fstream>
#include<vector>
#include<map>
#include<set>
#include<string>
#include <iterator>
#include <algorithm>
#include <stdlib.h>

using namespace std;

struct production {
    int p_id;
    string lhs;
    vector<string> rhs;
};

struct node {
    string name; // token or nonterminal's name
    string value; // token's value, like token INT_NUM can have value: 1
    int node_type; // exp/var/int{1: in register, 2: in memory} array{3: in memory}, for if related node, save if_id
    int mem_pos; // used if node_type = 2
    int reg_id; // used if node_type = 1, 0-7(s0-s7) saved reg only for variable, 8-15(t0-t7) temp reg,
    int size; // number of ints, for while related node, save while_id
};

struct reg {
    int symbol_id;
    int if_busy; //busy: 2 not busy: 1
};

struct reg_table {
    int next_save; //next index of saved reg, if saved reg is all busy, the value will be -1
    int next_temp; //next index of temp reg
    // 0-7(s0-s7) saved reg only for variable, 8-15(t0-t7) temp reg, 16-18(t8 t9 at) syscall
    struct reg reg_list[19];
};

struct symbol_state {
    int size; // number of ints in an array, 1 for single var
    int reg_id; //saved id of register: 0-7 saved reg only
    int status; // single var {1: in reg, 2: in memory} array {3: in memory} unused: others
    int mem_pos; //saved position in $sp
};

int hello_world();

void cg_handle_reduce(vector<struct node> & symbol, struct node & new_node, int grammar_id);

#endif