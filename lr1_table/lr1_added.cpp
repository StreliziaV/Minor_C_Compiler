#include "lr1_added.h"
#include <iostream>
#include <fstream>
#include <cstdio>

bool show_gen_step = false;
bool show_stack_state = false;

int hello_world() {
    std::cout << "generating parse table, please wait few minutes." << std::endl;
    return 1;
}

void read_grammar(vector<struct production> & grammar, set<string> & nonterminals, set<string> & tokens) {
    ifstream my_grammar;
    my_grammar.open("grammar.txt", ios::in);
    if (!my_grammar) {
        cout << "fail to open grammar file" << endl;
        return;
    }
    string line;
    set<string> all_symbols;
    int pro_id = 0;
    while (getline(my_grammar, line)) {
        if (line.length() == 0) continue;
        int p = line.find(":");
        string lhs = line.substr(0, p);
        string rhs = line.substr(p + 1);

        nonterminals.insert(lhs);
        
        struct production new_pro;
        new_pro.lhs = lhs;
        new_pro.p_id = pro_id;
        pro_id++;
        string temp = "";
        for (int i = 0; i < rhs.length(); i++) {
            if (rhs[i] == ' ') {
                new_pro.rhs.push_back(temp);
                all_symbols.insert(temp);
                temp = "";
                continue;
            }
            temp = temp + rhs[i];
        }
        new_pro.rhs.push_back(temp);
        all_symbols.insert(temp);
        grammar.push_back(new_pro);

    }
    
    set_difference(all_symbols.begin(), all_symbols.end(), nonterminals.begin(), nonterminals.end(), insert_iterator<set<string>>(tokens, tokens.begin()));
}

void compute_symbol_first(map<string, set<string>> & first_set, vector<string> & alpha, set<string> & result) {
    if (alpha.size() == 0) {
        result.insert("$");
        return;
    }

    for (auto k : first_set[alpha[0]]) if (k != "$") result.insert(k);

    int i;
    for (i = 1; (i < alpha.size() && first_set[alpha[i - 1]].find("$") != first_set[alpha[i - 1]].end()); i++) {
        for (auto j : first_set[alpha[i]]) {
            if (j == "$") continue;
            result.insert(j);
        }
    }
    if (i == alpha.size() && first_set[alpha[i - 1]].find("$") != first_set[alpha[i - 1]].end()) {
        result.insert("$");
    }
}

void compute_first(map<string, set<string>> & first_set, set<string> & nonterminals, set<string> & tokens, vector<struct production> & grammar) {
    // step 0, initialize
    for (auto nt : nonterminals) {
        set<string> temp;
        first_set.insert(pair<string, set<string>>(nt, temp));
    }
    for (auto token : tokens) {
        set<string> temp;
        first_set.insert(pair<string, set<string>>(token, temp));
    }
    
    // step 1
    for(auto pro : grammar) {
        if (find(pro.rhs.begin(), pro.rhs.end(), "$") == pro.rhs.end()) continue;
        first_set[pro.lhs].insert("$");
    }

    // step 2
    first_set["$"].insert("$");
    for (auto token : tokens) {
        first_set[token].insert(token);
    }
    for (auto pro : grammar) {
        if (tokens.find(pro.rhs[0]) == tokens.end()) continue;
        first_set[pro.lhs].insert(pro.rhs[0]);
    }

    // step 3
    bool change = true;
    while (change) {
        change = false;
        
        for (auto pro : grammar) {
            set<string> temp;
            compute_symbol_first(first_set, pro.rhs, temp);
            int pre_size = first_set[pro.lhs].size();
            for (auto new_ele : temp) first_set[pro.lhs].insert(new_ele);

            if (pre_size != first_set[pro.lhs].size()) change = true;
        }
    }
    
}

void compute_closure1(map<string, set<string>> & first_set, set<string> & nonterminals, vector<struct production> & grammar, struct config_st & s) {
    vector<struct config> cfgs;
    for (auto cg : s.configs) cfgs.push_back(cg);

    // compute configs in closure
    while (!cfgs.empty()) {
        struct config cg;
        cg.lhs = cfgs[0].lhs;
        cg.p_id = cfgs[0].p_id;
        cg.l = cfgs[0].l;
        for (auto i : cfgs[0].rhs_1) cg.rhs_1.push_back(i);
        for (auto i : cfgs[0].rhs_2) cg.rhs_2.push_back(i);
        cfgs.erase(cfgs.begin());

        if (cg.rhs_2.size() == 0) continue;
        if (find(nonterminals.begin(), nonterminals.end(), cg.rhs_2[0]) == nonterminals.end()) continue;
        
        for (auto gm : grammar) {
            if (gm.lhs != cg.rhs_2[0]) continue;
            set<string> first_pl;
            vector<string> pl(cg.rhs_2.begin() + 1, cg.rhs_2.end());
            if (pl.size() == 0) pl.push_back("$");
            pl.push_back(cg.l);
            compute_symbol_first(first_set, pl, first_pl);

            for (auto u : first_pl) {
                // bool add = false;
                // if (u == cg.l && cg.lhs == gm.lhs && cg.rhs_2.size() == gm.rhs.size()) {
                //     for (int i = 0; i < gm.rhs.size(); i++) {
                //         if (cg.rhs_2[i] != gm.rhs[i]) {
                //             add = true;
                //             break;
                //         }
                //     }
                // }
                // else add = true;
                // if (!add) continue;

                struct config temp;
                temp.p_id = gm.p_id;
                temp.lhs = gm.lhs;
                temp.l = u;
                for (auto i : gm.rhs) temp.rhs_2.push_back(i);

                bool if_exist;
                for (auto i : s.configs) {
                    if_exist = false;
                    if (i.p_id == temp.p_id && i.l == temp.l && i.lhs == temp.lhs && i.rhs_1.size() == temp.rhs_1.size() && i.rhs_2.size() == temp.rhs_2.size()) {
                        if_exist = true;
                        for (int j = 0; j < temp.rhs_2.size(); j++) {
                            if (temp.rhs_2[j] != i.rhs_2[j]) {
                                if_exist = false;
                                break;
                            }
                        }
                    }
                    if (if_exist == true) break;
                }
                if (if_exist == true) continue;

                temp.is_root = false;
                cfgs.push_back(temp);
                s.configs.push_back(temp);
            }

            
        }
    }
    
    // compute to_set
    for (auto cg : s.configs) {
        if (cg.rhs_2.size() == 0) continue;
        s.to_set[cg.rhs_2[0]] = -1;
    }
}

bool check_exist(vector<struct config_st> & state_table, struct config_st & temp_st, int & in_id) {
    bool if_exist = false;
    in_id = -1;
    for (auto temp_c_st : state_table) {
        bool the_same = true;
        for (auto config_i : temp_st.configs) {
            bool this_the_same = false;
            for (auto m_i : temp_c_st.configs) {
                if (!m_i.is_root) continue;
                if (m_i.l != config_i.l) continue;
                if (m_i.lhs != config_i.lhs) continue;
                if (m_i.p_id != config_i.p_id) continue;
                if (m_i.rhs_1.size() != config_i.rhs_1.size()) continue;
                if (m_i.rhs_2.size() != config_i.rhs_2.size()) continue;
                this_the_same = true;
                if (this_the_same) break;
            }
            the_same = the_same && this_the_same;
            if (!the_same) break;
        }
        if (the_same) {
            if_exist = true;
            in_id = temp_c_st.state_id;
            break;
        }
    }
    return if_exist;
}

void build_lr1(map<string, set<string>> & first_set, set<string> & nonterminals, set<string> & tokens, vector<struct production> & grammar, vector<struct config_st> & state_table) {
    // initialize
    struct config g0;
    g0.lhs = grammar[0].lhs;
    for (string i : grammar[0].rhs) g0.rhs_2.push_back(i);
    g0.p_id = 0;
    g0.l = "$";
    g0.is_root = true;

    struct config_st s0;
    s0.configs.push_back(g0);
    s0.state_id = 0;
    compute_closure1(first_set, nonterminals, grammar, s0);

    vector<struct config_st> S;
    S.push_back(s0);
    state_table.push_back(s0);

    // for (auto i : s0.to_set) cout << "|" << i.first << endl;

    int i = 0;
    while (!S.empty()) {
        struct config_st temp_c_st;
        for (auto i : S[0].to_set) temp_c_st.to_set.insert(i);
        for (auto i : S[0].configs) temp_c_st.configs.push_back(i);
        temp_c_st.state_id = S[0].state_id;

        if (show_gen_step) {
            cout << endl;
            cout << "generate from: " << endl;
            for (auto t : temp_c_st.to_set) cout << t.first << " ";
            cout << endl;
            for (auto p : temp_c_st.configs) {
                cout << p.lhs << " : ";
                for (auto i : p.rhs_1) cout << i << " ";
                cout << "| ";
                for (auto i : p.rhs_2) cout << i << " ";
                cout << ", {" << p.l << "} is_root: " << p.is_root << endl;
            }
            cout << "generate result: " << endl;
        }

        for (auto & to : temp_c_st.to_set) {
            if (to.first == "") return;
            // move the dot forward one token/nonterminal
            struct config_st temp_st;
            for (auto cg : temp_c_st.configs) {
                if (cg.rhs_2.empty()) continue;
                if (cg.rhs_2[0] != to.first) continue;
                struct config temp_cg;
                temp_cg.is_root = true;
                temp_cg.l = cg.l;
                temp_cg.p_id = cg.p_id;
                temp_cg.lhs = cg.lhs;
                for (auto i : cg.rhs_1) temp_cg.rhs_1.push_back(i);
                temp_cg.rhs_1.push_back(cg.rhs_2[0]);
                for (int j = 1; j < cg.rhs_2.size(); j++) temp_cg.rhs_2.push_back(cg.rhs_2[j]);

                temp_st.configs.push_back(temp_cg);
            }
            
            if (temp_st.configs.empty()) continue;

            //check if the resulted configuation set has the same root configurations as its mother
            bool the_same = true;
            for (auto config_i : temp_st.configs) {
                bool this_the_same = false;
                for (auto m_i : temp_c_st.configs) {
                    if (!m_i.is_root) continue;
                    if (m_i.l != config_i.l) continue;
                    if (m_i.lhs != config_i.lhs) continue;
                    if (m_i.p_id != config_i.p_id) continue;
                    if (m_i.rhs_1.size() != config_i.rhs_1.size()) continue;
                    if (m_i.rhs_2.size() != config_i.rhs_2.size()) continue;
                    this_the_same = true;
                    if (this_the_same) break;
                }
                the_same = the_same && this_the_same;
                if (!the_same) break;
            }

            if (the_same) {
                if (show_gen_step) cout << "------------------------self generation: " << to.first << "--------------------------------" <<endl;
                state_table[temp_c_st.state_id].to_set[to.first] = temp_c_st.state_id;
                continue;
            }

            //check if the resulted configuation set has existed
            int in_id;
            bool if_exist = check_exist(state_table, temp_st, in_id);
            if (if_exist) {
                if (show_gen_step) cout << "------------------------exist: " << to.first << "--------------------------------" <<endl;
                state_table[temp_c_st.state_id].to_set[to.first] = in_id;
                continue;
            }

            compute_closure1(first_set, nonterminals, grammar, temp_st);
            state_table[temp_c_st.state_id].to_set[to.first] = state_table.size();
            temp_st.state_id = state_table.size();
            state_table.push_back(temp_st);
            if (!temp_st.to_set.empty()) S.push_back(temp_st);

            if (show_gen_step) {
                cout << to.first << " result:" << endl;
                for (auto t : temp_st.to_set) cout << t.first << " ";
                cout << endl;
                for (auto p : temp_st.configs) {
                    cout << p.lhs << " : ";
                    for (auto i : p.rhs_1) cout << i << " ";
                    cout << "| ";
                    for (auto i : p.rhs_2) cout << i << " ";
                    cout << ", {" << p.l << "} is_root: " << p.is_root << endl;
                }
            }
        }

        S.erase(S.begin());
        if (show_stack_state) {
            cout << "iteration time: " << i << endl;
            cout << "state table size: " << state_table.size() << endl;
            cout << "stack remaining: " << S.size() << endl;
        }
        
        if (i == 500) break;
        i++;
        
    }
    

    // cout << endl;
    // for (auto p : s0.configs) {
    //     cout << p.lhs << " : ";
    //     for (auto i : p.rhs_1) cout << i << " ";
    //     cout << "| ";
    //     for (auto i : p.rhs_2) cout << i << " ";
    //     cout << ", {" << p.l << "}" << endl;
    // }
}
