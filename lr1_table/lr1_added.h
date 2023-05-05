#ifndef _LR1_ADDED_H_
#define _LR1_ADDED_H_

#include<vector>
#include<set>
#include<map>
#include<string>
#include <iterator>
#include <algorithm>

using namespace std;

struct production {
    int p_id;
    string lhs;
    vector<string> rhs;
};

//lhs -> rhs_1 * rhs_2, l with production p_id
struct config {
    int p_id;
    bool is_root;
    string lhs;
    vector<string> rhs_1;
    vector<string> rhs_2;
    string l;
};

struct config_st {
    vector<struct config> configs;
    map<string, int> to_set;
    int state_id;
};


int hello_world();

void read_grammar(vector<struct production> & grammar, set<string> & nonterminals, set<string> & tokens);

void compute_first(map<string, set<string>> & first_set, set<string> & nonterminals, set<string> & tokens, vector<struct production> & grammar);

void build_lr1(map<string, set<string>> & first_set, set<string> & nonterminals, set<string> & tokens, vector<struct production> & grammar, vector<struct config_st> & state_table);

#endif