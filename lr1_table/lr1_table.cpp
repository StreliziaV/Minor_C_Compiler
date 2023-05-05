#include "lr1_added.h"
#include <iostream>
#include <fstream>
#include <cstdio>

set<string> nonterminals;
set<string> tokens;
vector<struct production> grammar;
map<string, set<string>> first_set;
vector<struct config_st> state_table;

int main(int argc, char const *argv[]) {
    hello_world();

    read_grammar(grammar, nonterminals, tokens);
    // cout << grammar.size() << endl;
    // for (auto i : grammar) {
    //     cout << i.lhs << " :";
    //     for (auto j : i.rhs) {
    //         cout << " " << j;
    //     }
    //     cout << endl;
    // }
    // for (auto i : nonterminals) cout << i << endl;
    // for (auto i : tokens) cout << i << endl;

    compute_first(first_set, nonterminals, tokens, grammar);
    // for (auto i : first_set) {
    //     cout << i.first << " : ";
    //     for (auto j : i.second) cout << j << " ";
    //     cout << endl;
    // }
    // for (auto i : first_set["$"]) cout << i << endl;

    build_lr1(first_set, nonterminals, tokens, grammar, state_table);

    // write lr1 table to file
    remove("lr1_table.txt");
    ofstream outfile;
    outfile.open("lr1_table.txt", ios::app);
    for (auto & cg_st : state_table) {
        outfile << "S" << endl;
        for (auto & ts : cg_st.to_set) {
            outfile << ts.first << ":" << ts.second << endl;
        }
        outfile << "R" << endl;
        for (auto & cg : cg_st.configs) {
            if (cg.rhs_2.size() == 0) outfile << cg.l << ":" << cg.p_id << endl;
        }
        outfile << endl;
    }

    cout << "the lr1 table has been written in lr1_table.txt with total state number: " << state_table.size() << endl;

    return 0;
}