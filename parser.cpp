#include "code_gen.h"

vector<map<string, int>> lr1_table;
vector<string> token_list;
set<string> nonterminals;
set<string> tokens;
vector<struct production> grammar;

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

void read_table(vector<map<string, int>> & lr1_table) {
    ifstream infile;
    infile.open("lr1_table.txt");
    string line;
    int r_mode = 0; // 1: read shift, 2: read reduce
    int current_state = 0;

    map<string, int> n_st;
    lr1_table.push_back(n_st);

    bool conflict_detect = false;

    while (getline(infile, line)) {
        if (line.length() == 0) {
            map<string, int> temp_st;
            lr1_table.push_back(temp_st);  
            current_state++;
            continue;
        }
        if (line == "S") {
            r_mode = 1;
            continue;
        }
        if (line == "R") {
            r_mode = 2;
            continue;
        }
        
        int p = line.find(":");
        string lhs = line.substr(0, p);
        string rhs = line.substr(p + 1);
        int s_id = atoi(rhs.c_str());
        // use positive integer to store shift action
        if (r_mode == 1) {
            if (lr1_table[current_state][lhs] < 0) {
                cout << "---------------" << endl;
                conflict_detect = true;
            }
            lr1_table[current_state][lhs] = s_id;
        }
        // use negative integer to store reduce action
        if (r_mode == 2) {
            if (lr1_table[current_state][lhs] > 0) {
                cout << "!!!!!!!!!!!!! " << lhs << lr1_table[current_state][lhs] << ": " << s_id << endl;
                conflict_detect = true;
            }
            if (s_id == 0) lr1_table[current_state][lhs] = -100000;
            else lr1_table[current_state][lhs] = -s_id;
        }
    }
    
    if (!conflict_detect) {
        cout << "LR(1) parser table has been loaded without detecting any reduce/shift conflict, start parsing..." << endl;
    }
    else {
        cout << "The parsing table has reduce/shift conflict, please check your grammar" << endl;
        exit(1);
    }

}

int main(int argc, char const *argv[]) {
    remove("compiler_output.txt");
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);
    cpfile << "main:" << endl;
    cpfile.close();

    read_grammar(grammar, nonterminals, tokens);

    read_table(lr1_table);

    // read token list from scanner
    ifstream infile;
    infile.open("token_list.txt");
    string line;
    while (getline(infile, line)) {
        token_list.push_back(line);
    }
    
    remove("parser_output.txt");
    ofstream outfile;
    outfile.open("parser_output.txt", ios::app);
    int current_state = 0;
    int current_token = 0;
    vector<int> step;
    vector<struct node> symbol;
    bool read_new = true;
    struct node token;
    // start parsing
    while (true) {

        if (read_new) {
            if (current_token < token_list.size()) { 
                int p = token_list[current_token].find(":");
                token.name = token_list[current_token].substr(0, p);
                token.value = token_list[current_token].substr(p + 1);
            }
            else token.name = "$";
        }

        outfile << endl;
        outfile << "step: ";
        for (int i : step) outfile << i << " ";
        outfile << endl;
        outfile << "symbol stack: ";
        for (auto i : symbol) outfile << i.name << " ";
        outfile << endl;
        outfile << "current symbol: " << token.name << ", " << "current state: " << current_state << endl;
        
        if ((lr1_table[current_state].find(token.name) != lr1_table[current_state].end())) {
            int action = lr1_table[current_state][token.name];
            if (action == -100000) {
                cout << "accept !" << endl;
                outfile << "accept !" << endl;
                break;
            }
            // positive action means shift and go to the state
            if (action >= 0) {
                if (read_new) step.push_back(current_state);
                symbol.push_back(token);
                current_state = action;
                if (read_new) current_token++;
                read_new = true;
                outfile << "action: " << "shift and go to state " << action;
            }
            // negative action means reduce
            else {
                cg_handle_reduce(symbol, token, -action);
                action = - action;
                step.push_back(current_state);
                int reduce_num = grammar[action].rhs.size();
                for (int i = 0; i < reduce_num; i++) {
                    step.pop_back();
                    symbol.pop_back();
                }
                current_state = step[step.size() - 1];
                token.name = grammar[action].lhs;
                token.value = "";
                read_new = false;
                outfile << "action: reduce " << action << ": ";
                outfile << grammar[action].lhs << " -> ";
                for (auto i : grammar[action].rhs) outfile << i << " ";
            }
        }
        else if (lr1_table[current_state].find("$") != lr1_table[current_state].end()) {
            outfile << "no match for the token, use $" << endl;
            int action = lr1_table[current_state]["$"];
            if (action == -100000) {
                cout << "accept !" << endl;
                outfile << "accept !" << endl;
                break;
            }
            // positive action means shift and go to the state
            if (action >= 0) {
                if (read_new) step.push_back(current_state);
                struct node new_node;
                new_node.name = "$";
                symbol.push_back(new_node);
                current_state = action;
                read_new = true;
                outfile << "action: " << "shift and go to state " << action;
            }
            // negative action means reduce
            else {
                cg_handle_reduce(symbol, token, -action);
                action = - action;
                step.push_back(current_state);
                int reduce_num = grammar[action].rhs.size();
                for (int i = 0; i < reduce_num; i++) {
                    step.pop_back();
                    symbol.pop_back();
                }
                current_state = step[step.size() - 1];
                token.name = grammar[action].lhs;
                token.value = "";
                read_new = false;
                outfile << "action: reduce " << action << ": ";
                outfile << grammar[action].lhs << " -> ";
                for (auto i : grammar[action].rhs) outfile << i << " ";
            }
        }
        else {
            cout << "syntax error: " << current_token << endl;
            exit(1);
        }

        outfile << endl;
    }

    cpfile.open("compiler_output.txt", ios::app);
    cpfile << "END:" << endl;
    cpfile.close();

    return 0;
}