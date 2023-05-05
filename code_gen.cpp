#include "code_gen.h"

static map<string, struct symbol_state> symbol_table;
static struct reg_table reg_tb{next_save: 0, next_temp: 8};
static int sp_reg = 0; // next memory position available
static int if_id = 0;
static int while_id = 0;
static int do_while_id = 0;

void copy_node(struct node & source_node, struct node & target_node) {
    target_node.mem_pos = source_node.mem_pos;
    target_node.node_type = source_node.node_type;
    target_node.reg_id = source_node.reg_id;
    target_node.size = source_node.size;
}

void full_copy_node(struct node & source_node, struct node & target_node) {
    target_node.name = source_node.name;
    target_node.value = source_node.value;
    target_node.mem_pos = source_node.mem_pos;
    target_node.node_type = source_node.node_type;
    target_node.reg_id = source_node.reg_id;
    target_node.size = source_node.size;
}

void free_temp_reg(struct node & target_node) {
    if (target_node.node_type == 1) if (target_node.reg_id >= 8) reg_tb.reg_list[target_node.reg_id].if_busy = 1;
}

void maintain_reg_table() {
    reg_tb.next_save = -1;
    reg_tb.next_temp = -1;
    for (int i = 0; i < 8; i++) {
        if (reg_tb.reg_list[i].if_busy != 2) {
            reg_tb.next_save = i;
            break;
        }
    }
    for (int i = 8; i < 16; i++) {
        if (reg_tb.reg_list[i].if_busy != 2) {
            reg_tb.next_temp = i;
            break;
        }
    }
}

// T is load to t8, P is load to t9
void load_8_9(struct node & T, struct node & P, ofstream & cpfile) {
    if (T.node_type == 1) {
        // if T is in temp reg
        if (T.reg_id >= 8) cpfile << "add $t8 $zero $t" << (T.reg_id - 8) << endl;
        // if T is in saved reg
        else cpfile << "add $t8 $zero $s" << (T.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << T.mem_pos << "($sp)" << endl;

    if (P.node_type == 1) {
        if (P.reg_id >= 8) cpfile << "add $t9 $zero $t" << (P.reg_id - 8) << endl;
        else cpfile << "add $t9 $zero $s" << (P.reg_id) << endl;
    }
    else cpfile << "lw $t9 " << P.mem_pos << "($sp)" << endl;
}

// T is load to t8
void load_8(struct node & T, ofstream & cpfile) {
    if (T.node_type == 1) {
        // if T is in temp reg
        if (T.reg_id >= 8) cpfile << "add $t8 $zero $t" << (T.reg_id - 8) << endl;
        // if T is in saved reg
        else cpfile << "add $t8 $zero $s" << (T.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << T.mem_pos << "($sp)" << endl;
}

// new T(new_node, result in t9) will store in T, if T is in temp/mem, otherwise, use new temp/memory space
void easy_save(struct node & T, struct node & new_node, ofstream & cpfile) {
    copy_node(T, new_node);
    if (T.node_type == 1) {
        if (T.reg_id >= 8) {
            cpfile << "add $t" << (T.reg_id - 8) << " $zero $t9" << endl;
        }
        else {
            if (reg_tb.next_temp != -1) {
                cpfile << "add $t" << (reg_tb.next_temp - 8) << " $zero $t9" << endl;
                new_node.node_type = 1;
                new_node.reg_id = reg_tb.next_temp;
                new_node.size = 1;
                reg_tb.reg_list[reg_tb.next_temp].if_busy = 2;
                maintain_reg_table();
            }
            else {
                cpfile << "sw $t9 " << sp_reg << "($sp)" << endl;
                new_node.node_type = 2;
                new_node.mem_pos = sp_reg;
                new_node.size = 1;
                new_node.reg_id = -1;
                sp_reg = sp_reg - 4;
            }
        }
    }
    else cpfile << "sw $t9 " << T.mem_pos << "($sp)" << endl;
}

int hello_world() {

    cout << "hello world" << endl;

    return 1;
}

// 0 program:var_declarations statements
// 1 var_declarations:var_declaration var_declarations
// 2 var_declarations:$
// 3 var_declaration:INT declaration_list SEMI
// 4 declaration_list:declaration_list COMMA declaration
// 5 declaration_list:declaration

// 6 declaration:ID ASSIGN exp
void reduce_6(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    struct node exp;
    full_copy_node(symbol[symbol.size() - 1], exp);
    full_copy_node(symbol[symbol.size() - 3], id);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp to $t8
    if (exp.node_type == 1) {
        // if exp is in temp reg
        if (exp.reg_id >= 8) cpfile << "add $t8 $zero $t" << (exp.reg_id - 8) << endl;
        // if exp is in saved reg
        else cpfile << "add $t8 $zero $s" << (exp.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << exp.mem_pos << "($sp)" << endl;

    struct symbol_state id_state;
    id_state.size = 1;
    // if there are availble save reg
    if (reg_tb.next_save != -1) {
        // variable is initialized as exp
        cpfile << "add $s" << reg_tb.next_save << " $zero $t8" << endl;
        // update symbol state
        id_state.status = 1;
        id_state.reg_id = reg_tb.next_save;
        symbol_table[id.value] = id_state;
        // update reg state
        reg_tb.reg_list[reg_tb.next_save].if_busy = 2;
        maintain_reg_table();
    }// if there are no availble save reg
    else {
        // variable is initialized as exp
        cpfile << "sw $t8 " << sp_reg << "($sp)" << endl;
        //update symbol state
        id_state.status = 2;
        id_state.mem_pos = sp_reg;
        symbol_table[id.value] = id_state;
        //update memory position
        sp_reg = sp_reg - 4;
    }

    free_temp_reg(exp);
    maintain_reg_table();

    cpfile.close();
}

// 7 declaration:ID LSQUARE INT_NUM RSQUARE
void reduce_7(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    struct node int_num;
    full_copy_node(symbol[symbol.size() - 2], int_num);
    full_copy_node(symbol[symbol.size() - 4], id);
    int a_size = atoi(int_num.value.c_str());
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    struct symbol_state id_state;
    id_state.size = a_size;
    // array is always saved in memory
    cpfile << "sw $t8 " << sp_reg << "($sp)" << endl;
    //update symbol state
    id_state.status = 3;
    id_state.mem_pos = sp_reg;
    symbol_table[id.value] = id_state;
    //update memory position
    sp_reg = sp_reg - 4 * a_size;

    cpfile.close();
}

// 8 declaration:ID
void reduce_8(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    full_copy_node(symbol[symbol.size() - 1], id);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    struct symbol_state id_state;
    id_state.size = 1;
    // if there are availble save reg
    if (reg_tb.next_save != -1) {
        // variable is initialized as 0
        cpfile << "addi $s" << reg_tb.next_save << " $zero 0" << endl;
        // update symbol state
        id_state.status = 1;
        id_state.reg_id = reg_tb.next_save;
        symbol_table[id.value] = id_state;
        // update reg state
        reg_tb.reg_list[reg_tb.next_save].if_busy = 2;
        maintain_reg_table();
    }// if there are no availble save reg
    else {
        // variable is initialized as 0
        cpfile << "sw $zero " << sp_reg << "($sp)" << endl;
        //update symbol state
        id_state.status = 2;
        id_state.mem_pos = sp_reg;
        symbol_table[id.value] = id_state;
        //update memory position
        sp_reg = sp_reg - 4;
    }

    cpfile.close();
}

// 9 code_block:statement
// 10 code_block:LBRACE statements RBRACE
// 11 statements:statements statement
// 12 statements:statement

// 13 statement:assign_statement SEMI
// 14 statement:control_statement
// 15 statement:read_write_statement SEMI
// 16 statement:SEMI
void stm() {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);
    cpfile << endl;
    cpfile.close();
}

// 17 control_statement:if_statement
// 18 control_statement:while_statement
// 19 control_statement:do_while_statement SEMI
// 20 control_statement:return_statement SEMI
// 21 read_write_statement:read_statement
// 22 read_write_statement:write_statement

// 23 assign_statement:ID LSQUARE exp RSQUARE ASSIGN exp
void reduce_23(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    struct node exp;
    struct node a_exp;
    full_copy_node(symbol[symbol.size() - 6], id);
    full_copy_node(symbol[symbol.size() - 4], exp);
    full_copy_node(symbol[symbol.size() - 1], a_exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp to $t8
    if (exp.node_type == 1) {
        // if exp is in temp reg
        if (exp.reg_id >= 8) cpfile << "add $t8 $zero $t" << (exp.reg_id - 8) << endl;
        // if exp is in saved reg
        else cpfile << "add $t8 $zero $s" << (exp.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << exp.mem_pos << "($sp)" << endl;
    
    // check if the variable is declared
    if (symbol_table.find(id.value) != symbol_table.end()) {
        // calculate the correct memory offeset, store it in $t9
        cpfile << "addi $t9 $zero -4" << endl;
        cpfile << "mul $t9 $t9 $t8" << endl;
        cpfile << "addi $t9 $t9 " << (symbol_table[id.value].mem_pos) << endl;
        // calculate correct memory position, store it in $t8
        cpfile << "add $t8 $t9 $sp" << endl;

        // complete assignment
        if (a_exp.node_type == 1) {
            // if a_exp is in temp reg
            if (a_exp.reg_id >= 8) cpfile << "sw $t" << (a_exp.reg_id - 8) << " 0($t8)" << endl;
            // if a_exp is in saved reg
            else cpfile << "sw $s" << (a_exp.reg_id) << " 0($t8)" << endl;
        }
        else {
            cpfile << "lw $t9 " << a_exp.mem_pos << "($sp)" << endl;
            cpfile << "sw $t9 0($t8)" << endl;
        }
    }
    else {
        cout << "variable " << id.value << " is not declared, terminates" << endl;
        exit(1);
    }

    cpfile.close();
}

// 24 assign_statement:ID ASSIGN exp
void reduce_24(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    struct node exp;
    full_copy_node(symbol[symbol.size() - 3], id);
    full_copy_node(symbol[symbol.size() - 1], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp to $t8
    if (exp.node_type == 1) {
        // if exp is in temp reg
        if (exp.reg_id >= 8) cpfile << "add $t8 $zero $t" << (exp.reg_id - 8) << endl;
        // if exp is in saved reg
        else cpfile << "add $t8 $zero $s" << (exp.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << exp.mem_pos << "($sp)" << endl;

    // check if the variable is declared
    if (symbol_table.find(id.value) == symbol_table.end()) {
        cout << "variable " << id.value << " is not declared, terminates" << endl;
        exit(1);
    }

    // then store the value of exp to the space of ID
    if (symbol_table[id.value].status == 1) {
        cpfile << "add $s" << symbol_table[id.value].reg_id << " $t8 $zero" << endl;
    }
    else cpfile << "sw $t8 " << symbol_table[id.value].mem_pos << "($sp)" << endl;

    cpfile.close();
}

// 25 if_statement:if_struct if_tail
void reduce_25(vector<struct node> & symbol, struct node & new_node) {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // pass if_id
    new_node.node_type = symbol[symbol.size() - 2].node_type;

    // create new if_out branch
    cpfile << "IF_OUT_" << new_node.node_type << ":" << endl;

    cpfile.close();
}
// 26 if_struct:if_head code_block
void reduce_26(vector<struct node> & symbol, struct node & new_node) {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // pass if_id
    new_node.node_type = symbol[symbol.size() - 2].node_type;

    // after code block, jump to exit point
    cpfile << "j IF_OUT_" << new_node.node_type << endl;
    // create new else branch
    cpfile << "ELSE_" << new_node.node_type << ":" << endl;

    cpfile.close();
}
// 27 if_head:IF LPAR exp RPAR
void reduce_27(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    new_node.node_type = if_id;
    if_id++;

    // first load exp value to t8
    load_8(exp, cpfile);

    // if exp is 0, jump to else
    cpfile << "beq $t8 $zero ELSE_" << new_node.node_type << endl;

    free_temp_reg(exp);
    maintain_reg_table();

    cpfile.close();
}
// 28 if_tail:ELSE code_block
// 29 if_tail:$

// 30 while_statement:while_head code_block
void reduce_30(vector<struct node> & symbol, struct node & new_node) {
    struct node exp; // exp info and while id is saved in while_head 
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // save exp info and while_id
    copy_node(exp, new_node);

    // jump back to while enter
    cpfile << "j WHILE_" << new_node.size << endl;

    // create while out
    cpfile << "WHILE_OUT_" << new_node.size << ":" << endl;

    cpfile.close();
}

// 31 while_head:while_head_L exp RPAR
void reduce_31(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // while id in while_head_l
    copy_node(exp, new_node);
    new_node.size = symbol[symbol.size() - 3].size;

    // first load exp value to t8
    load_8(exp, cpfile);

    // if exp is 0, jump to while out
    cpfile << "beq $t8 $zero WHILE_OUT_" << new_node.size << endl;

    cpfile.close();
}

// 69 while_head_L:WHILE LPAR
void reduce_69(vector<struct node> & symbol, struct node & new_node) {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // save while_id in while_head_L
    new_node.size = while_id;
    while_id++;

    // create while enter
    cpfile << "WHILE_" << new_node.size << ":" << endl;

    cpfile.close();
}

// 32 do_while_statement:do_token code_block WHILE LPAR exp RPAR
void reduce_32(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    load_8(exp, cpfile);

    // if exp not 0, continue do while
    cpfile << "bne $t8 $zero DO_WHILE_" << symbol[symbol.size() - 6].size << endl;

    cpfile.close();
}

// 33 do_token:DO
void reduce_33(vector<struct node> & symbol, struct node & new_node) {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    new_node.size = do_while_id;
    do_while_id++;

    // create do while enter
    cpfile << "DO_WHILE_" << new_node.size << ":" << endl;

    cpfile.close();
}

// 34 return_statement:RETURN
void reduce_34(vector<struct node> & symbol, struct node & new_node) {
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);
    cpfile << "j END" << endl;
    cpfile.close();
}

// 35 read_statement:READ LPAR ID RPAR
void reduce_35(vector<struct node> & symbol, struct node & new_node) {
    struct node id; // ID
    full_copy_node(symbol[symbol.size() - 2], id);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // check ID
    if (symbol_table.find(id.value) == symbol_table.end()) {
        cout << "variable " << id.value << " is not declared, terminates" << endl;
        exit(1);
    }

    // set $v0 to 5
    cpfile << "addi $v0 $zero 5" << endl;
    // read
    cpfile << "syscall" << endl;
    // save
    if (symbol_table[id.value].status == 1) {
        cpfile << "add $s" << symbol_table[id.value].reg_id << " $v0 $zero" << endl;
    }
    else {
        cpfile << "sw $v0 " << symbol_table[id.value].mem_pos << "($sp)" << endl;
    }

    cpfile.close();
}

// 36 write_statement:WRITE LPAR exp RPAR
void reduce_36(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load exp to $a0
    if (exp.node_type == 1) {
        // if exp is in temp reg
        if (exp.reg_id >= 8) cpfile << "add $a0 $zero $t" << (exp.reg_id - 8) << endl;
        // if exp is in saved reg
        else cpfile << "add $a0 $zero $s" << (exp.reg_id) << endl;
    }
    else cpfile << "lw $a0 " << exp.mem_pos << "($sp)" << endl;

    // set $v0 to 1
    cpfile << "addi $v0 $zero 1" << endl;
    // print
    cpfile << "syscall" << endl;

    cpfile.close();
}

// 37 exp:exp OROR exp1
void reduce_37(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    struct node exp1;
    full_copy_node(symbol[symbol.size() - 3], exp);
    full_copy_node(symbol[symbol.size() - 1], exp1);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load exp to t8 and exp1 to t9
    load_8_9(exp, exp1, cpfile);

    // || operation, only when exp = 0 and exp1 = 0, the result will be 0
    // set $t8 to 1 if it is not 0, same for $t9, use v0 temporarily
    cpfile << "addi $v0 $zero 1" << endl;
    cpfile << "movn $t8 $v0 $t8" << endl;
    cpfile << "movn $t9 $v0 $t9" << endl;
    cpfile << "or $v0 $t8 $t9" << endl;
    // save the result in memory
    cpfile << "sw $v0 " << sp_reg << "($sp)" << endl;
    // update result node
    new_node.mem_pos = sp_reg;
    new_node.node_type = 2;
    new_node.size = 1;

    // maintain regs
    free_temp_reg(exp);
    free_temp_reg(exp1);
    maintain_reg_table();

    sp_reg = sp_reg - 4;
    cpfile.close();
}

// 38 exp:exp1
void reduce_38(vector<struct node> & symbol, struct node & new_node) {
    struct node exp1;
    full_copy_node(symbol[symbol.size() - 1], exp1);
    copy_node(exp1, new_node);
}

// 39 exp1:exp1 ANDAND exp2
void reduce_39(vector<struct node> & symbol, struct node & new_node) {
    struct node exp; //exp1
    struct node exp1; //exp2
    full_copy_node(symbol[symbol.size() - 3], exp);
    full_copy_node(symbol[symbol.size() - 1], exp1);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load exp1 to t8 and exp2 to t9
    load_8_9(exp, exp1, cpfile);

    // && operation, only when exp not 0 and exp1 not 0, the result will be 1
    // set $v0 as 1 first, if any of $t8 or $t9 is 0, set $v0 to 0
    cpfile << "addi $v0 $zero 1" << endl;
    cpfile << "movz $v0 $zero $t8" << endl;
    cpfile << "movz $v0 $zero $t9" << endl;
    // save the result in memory
    cpfile << "sw $v0 " << sp_reg << "($sp)" << endl;
    // update result node
    new_node.mem_pos = sp_reg;
    new_node.node_type = 2;
    new_node.size = 1;

    // maintain regs
    free_temp_reg(exp);
    free_temp_reg(exp1);
    maintain_reg_table();

    sp_reg = sp_reg - 4;
    cpfile.close();
}

// 40 exp1:exp2
void reduce_40(vector<struct node> & symbol, struct node & new_node) {
    struct node exp2;
    full_copy_node(symbol[symbol.size() - 1], exp2);
    copy_node(exp2, new_node);
}

// 41 exp2:exp2 OR_OP exp3
void reduce_41(vector<struct node> & symbol, struct node & new_node) {
    struct node exp3;
    struct node exp2;
    full_copy_node(symbol[symbol.size() - 1], exp3);
    full_copy_node(symbol[symbol.size() - 3], exp2);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp2 to $t8, exp3 to $t9
    load_8_9(exp2, exp3, cpfile);

    // bitwise or, result in $t9
    cpfile << "or $t9 $t8 $t9" << endl;

    // new exp2 will store in exp2, if exp2 is in temp/mem, otherwise, use new temp/memory space
    easy_save(exp2, new_node, cpfile);

    // free P's temp reg
    free_temp_reg(exp3);
    maintain_reg_table();

    cpfile.close();
}

// 42 exp2:exp3
void reduce_42(vector<struct node> & symbol, struct node & new_node) {
    struct node exp3;
    full_copy_node(symbol[symbol.size() - 1], exp3);
    copy_node(exp3, new_node);
}

// 43 exp3:exp3 AND_OP exp4
void reduce_43(vector<struct node> & symbol, struct node & new_node) {
    struct node exp3; //exp4
    struct node exp2; //exp3
    full_copy_node(symbol[symbol.size() - 1], exp3);
    full_copy_node(symbol[symbol.size() - 3], exp2);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp3 to $t8, exp4 to $t9
    load_8_9(exp2, exp3, cpfile);

    // bitwise and, result in $t9
    cpfile << "and $t9 $t8 $t9" << endl;

    // new exp3 will store in exp3, if exp3 is in temp/mem, otherwise, use new temp/memory space
    easy_save(exp2, new_node, cpfile);

    // free P's temp reg
    free_temp_reg(exp3);
    maintain_reg_table();

    cpfile.close();
}

// 44 exp3:exp4
void reduce_44(vector<struct node> & symbol, struct node & new_node) {
    struct node exp4;
    full_copy_node(symbol[symbol.size() - 1], exp4);
    copy_node(exp4, new_node);
}

// 45 exp4:exp4 EQ exp5
void reduce_45(vector<struct node> & symbol, struct node & new_node) {
    struct node exp4;
    struct node exp5;
    full_copy_node(symbol[symbol.size() - 1], exp5);
    full_copy_node(symbol[symbol.size() - 3], exp4);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp4 to $t8, exp5 to $t9
    load_8_9(exp4, exp5, cpfile);

    // new exp4 = 1, if exp4 = exp5, result save in t9
    cpfile << "xor $t9 $t8 $t9" << endl;
    cpfile << "sltiu $t9 $t9 1" << endl;

    // save new exp4, result in t9
    easy_save(exp4, new_node, cpfile);

    free_temp_reg(exp5);
    maintain_reg_table();

    cpfile.close();
}

// 46 exp4:exp4 NOTEQ exp5
void reduce_46(vector<struct node> & symbol, struct node & new_node) {
    struct node exp4;
    struct node exp5;
    full_copy_node(symbol[symbol.size() - 1], exp5);
    full_copy_node(symbol[symbol.size() - 3], exp4);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp4 to $t8, exp5 to $t9
    load_8_9(exp4, exp5, cpfile);

    // new exp4 = 0, if exp4 = exp5, otherwise, 1, result save in t9
    cpfile << "xor $t9 $t8 $t9" << endl;
    cpfile << "addi $t8 $zero 1" << endl;
    cpfile << "movn $t9 $t8 $t9" << endl;

    // save new exp4, result in t9
    easy_save(exp4, new_node, cpfile);

    free_temp_reg(exp5);
    maintain_reg_table();

    cpfile.close();
}

// 47 exp4:exp5
void reduce_47(vector<struct node> & symbol, struct node & new_node) {
    struct node exp5;
    full_copy_node(symbol[symbol.size() - 1], exp5);
    copy_node(exp5, new_node);
}

// 48 exp5:exp5 LTEQ exp6
void reduce_48(vector<struct node> & symbol, struct node & new_node) {
    struct node exp5;
    struct node exp6;
    full_copy_node(symbol[symbol.size() - 1], exp6);
    full_copy_node(symbol[symbol.size() - 3], exp5);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp5 to $t8, exp6 to $t9
    load_8_9(exp5, exp6, cpfile);

    // new exp5 = 0, if exp6 < exp5, result save in t9
    cpfile << "slt $t9 $t9 $t8" << endl;
    cpfile << "not $t9 $t9" << endl;

    // save new exp5, result in t9
    easy_save(exp5, new_node, cpfile);

    free_temp_reg(exp6);
    maintain_reg_table();

    cpfile.close();
}

// 49 exp5:exp5 GTEQ exp6
void reduce_49(vector<struct node> & symbol, struct node & new_node) {
    struct node exp5;
    struct node exp6;
    full_copy_node(symbol[symbol.size() - 1], exp6);
    full_copy_node(symbol[symbol.size() - 3], exp5);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp5 to $t8, exp6 to $t9
    load_8_9(exp5, exp6, cpfile);

    // new exp5 = 0, if exp5 < exp6, result save in t9
    cpfile << "slt $t9 $t8 $t9" << endl;
    cpfile << "not $t9 $t9" << endl;

    // save new exp5, result in t9
    easy_save(exp5, new_node, cpfile);

    free_temp_reg(exp6);
    maintain_reg_table();

    cpfile.close();
}

// 50 exp5:exp5 LT exp6
void reduce_50(vector<struct node> & symbol, struct node & new_node) {
    struct node exp5;
    struct node exp6;
    full_copy_node(symbol[symbol.size() - 1], exp6);
    full_copy_node(symbol[symbol.size() - 3], exp5);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp5 to $t8, exp6 to $t9
    load_8_9(exp5, exp6, cpfile);

    // new exp5 = 1, if exp5 < exp6, result save in t9
    cpfile << "slt $t9 $t8 $t9" << endl;

    // save new exp5, result in t9
    easy_save(exp5, new_node, cpfile);

    free_temp_reg(exp6);
    maintain_reg_table();

    cpfile.close();
}

// 51 exp5:exp5 GT exp6
void reduce_51(vector<struct node> & symbol, struct node & new_node) {
    struct node exp5;
    struct node exp6;
    full_copy_node(symbol[symbol.size() - 1], exp6);
    full_copy_node(symbol[symbol.size() - 3], exp5);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp5 to $t8, exp6 to $t9
    load_8_9(exp5, exp6, cpfile);

    // new exp5 = 1, if exp6 < exp5, result save in t9
    cpfile << "slt $t9 $t9 $t8" << endl;

    // save new exp5, result in t9
    easy_save(exp5, new_node, cpfile);

    free_temp_reg(exp6);
    maintain_reg_table();

    cpfile.close();
}

// 52 exp5:exp6
void reduce_52(vector<struct node> & symbol, struct node & new_node) {
    struct node exp6;
    full_copy_node(symbol[symbol.size() - 1], exp6);
    copy_node(exp6, new_node);
}

// 53 exp6:exp6 SHL_OP exp7
void reduce_53(vector<struct node> & symbol, struct node & new_node) {
    struct node exp6;
    struct node exp7;
    full_copy_node(symbol[symbol.size() - 1], exp7);
    full_copy_node(symbol[symbol.size() - 3], exp6);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp6 to $t8, exp7 to $t9
    load_8_9(exp6, exp7, cpfile);

    // shift left, result in $t9, t9 = exp6 << exp7 = t8 << t9
    cpfile << "sllv $t9 $t8 $t9" << endl;

    // save new exp6, result in t9
    easy_save(exp6, new_node, cpfile);

    free_temp_reg(exp7);
    maintain_reg_table();

    cpfile.close();
}

// 54 exp6:exp6 SHR_OP exp7
void reduce_54(vector<struct node> & symbol, struct node & new_node) {
    struct node exp6;
    struct node exp7;
    full_copy_node(symbol[symbol.size() - 1], exp7);
    full_copy_node(symbol[symbol.size() - 3], exp6);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp6 to $t8, exp7 to $t9
    load_8_9(exp6, exp7, cpfile);

    // shift right, result in $t9, t9 = exp6 >> exp7 = t8 >> t9
    cpfile << "srav $t9 $t8 $t9" << endl;

    // save new exp6, result in t9
    easy_save(exp6, new_node, cpfile);

    free_temp_reg(exp7);
    maintain_reg_table();

    cpfile.close();
}

// 55 exp6:exp7
void reduce_55(vector<struct node> & symbol, struct node & new_node) {
    struct node exp7;
    full_copy_node(symbol[symbol.size() - 1], exp7);
    copy_node(exp7, new_node);
}

// 56 exp7:exp7 PLUS T
void reduce_56(vector<struct node> & symbol, struct node & new_node) {
    struct node exp7;
    struct node T;
    full_copy_node(symbol[symbol.size() - 1], T);
    full_copy_node(symbol[symbol.size() - 3], exp7);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp7 to $t8, T to $t9
    load_8_9(exp7, T, cpfile);

    // addition, result in $t9
    cpfile << "add $t9 $t8 $t9" << endl;

    // save new exp7
    easy_save(exp7, new_node, cpfile);

    free_temp_reg(T);
    maintain_reg_table();

    cpfile.close();
}

// 57 exp7:exp7 MINUS T
void reduce_57(vector<struct node> & symbol, struct node & new_node) {
    struct node exp7;
    struct node T;
    full_copy_node(symbol[symbol.size() - 1], T);
    full_copy_node(symbol[symbol.size() - 3], exp7);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp7 to $t8, T to $t9
    load_8_9(exp7, T, cpfile);

    // sub, result in $t9 = exp7 - T = t8 - t9
    cpfile << "sub $t9 $t8 $t9" << endl;

    // save new exp7
    easy_save(exp7, new_node, cpfile);

    free_temp_reg(T);
    maintain_reg_table();

    cpfile.close();
}

// 58 exp7:T
void reduce_58(vector<struct node> & symbol, struct node & new_node) {
    struct node T;
    full_copy_node(symbol[symbol.size() - 1], T);
    copy_node(T, new_node);
}

// 59 T:T MUL_OP P
void reduce_59(vector<struct node> & symbol, struct node & new_node) {
    struct node P;
    struct node T;
    full_copy_node(symbol[symbol.size() - 1], P);
    full_copy_node(symbol[symbol.size() - 3], T);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of T to $t8, P to $t9
    load_8_9(T, P, cpfile);

    // multiply, result in $t9
    cpfile << "mul $t9 $t8 $t9" << endl;

    // new T will store in T, if T is in temp/mem, otherwise, use new temp/memory space
    easy_save(T, new_node, cpfile);

    // free P's temp reg
    free_temp_reg(P);
    maintain_reg_table();

    cpfile.close();
}

// 60 T:T DIV_OP P
void reduce_60(vector<struct node> & symbol, struct node & new_node) {
    struct node P;
    struct node T;
    full_copy_node(symbol[symbol.size() - 1], P);
    full_copy_node(symbol[symbol.size() - 3], T);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of T to $t8, P to $t9
    load_8_9(T, P, cpfile);

    // division, result in $t9
    cpfile << "div $t8 $t9" << endl;
    cpfile << "mflo $t9" << endl;

    // new T will store in T, if T is in temp/mem, otherwise, use new temp/memory space
    easy_save(T, new_node, cpfile);

    // free P's temp reg
    free_temp_reg(P);
    maintain_reg_table();

    cpfile.close();
}

// 61 T:P
void reduce_61(vector<struct node> & symbol, struct node & new_node) {
    struct node P;
    full_copy_node(symbol[symbol.size() - 1], P);
    copy_node(P, new_node);
}

// 62 P:NOT_OP F
void reduce_62(vector<struct node> & symbol, struct node & new_node) {
    struct node F;
    full_copy_node(symbol[symbol.size() - 1], F);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of F to $t8
    if (F.node_type == 1) {
        if (F.reg_id >= 8) cpfile << "add $t8 $zero $t" << (F.reg_id - 8) << endl;
        else cpfile << "add $t8 $zero $s" << (F.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << F.mem_pos << "($sp)" << endl;

    // if F is not 0, set result(in $t9) to 0, otherwise, 1
    cpfile << "addi $t9 $zero 1" << endl;
    cpfile << "movn $t9 $zero $t8" << endl;
    
    // P will store in F, if F is in temp/mem, otherwise, use new memory space
    copy_node(F, new_node);
    if (F.node_type == 1) {
        if (F.reg_id >= 8) {
            cpfile << "add $t" << (F.reg_id - 8) << " $zero $t9" << endl;
        }
        else {
            cpfile << "sw $t9 " << sp_reg << "($sp)" << endl;
            new_node.node_type = 2;
            new_node.mem_pos = sp_reg;
            new_node.size = 1;
            new_node.reg_id = -1;
            sp_reg = sp_reg - 4;
        }
    }
    else cpfile << "sw $t9 " << F.mem_pos << "($sp)" << endl;

    // P will store in F, no need for temp reg release

    cpfile.close();
}

// 63 P:MINUS F
void reduce_63(vector<struct node> & symbol, struct node & new_node) {
    struct node F;
    full_copy_node(symbol[symbol.size() - 1], F);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of F to $t8
    if (F.node_type == 1) {
        if (F.reg_id >= 8) cpfile << "add $t8 $zero $t" << (F.reg_id - 8) << endl;
        else cpfile << "add $t8 $zero $s" << (F.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << F.mem_pos << "($sp)" << endl;

    // store - F in $t9
    cpfile << "sub $t9 $zero $t8" << endl;
    
    // P will store in F, if F is in temp/mem, otherwise, use new memory space
    copy_node(F, new_node);
    if (F.node_type == 1) {
        if (F.reg_id >= 8) {
            cpfile << "add $t" << (F.reg_id - 8) << " $zero $t9" << endl;
        }
        else {
            cpfile << "sw $t9 " << sp_reg << "($sp)" << endl;
            new_node.node_type = 2;
            new_node.mem_pos = sp_reg;
            new_node.size = 1;
            new_node.reg_id = -1;
            sp_reg = sp_reg - 4;
        }
    }
    else cpfile << "sw $t9 " << F.mem_pos << "($sp)" << endl;

    // P will store in F, no need for temp reg release

    cpfile.close();
}

// 64 P:F
void reduce_64(vector<struct node> & symbol, struct node & new_node) {
    struct node F;
    full_copy_node(symbol[symbol.size() - 1], F);
    copy_node(F, new_node);
}

// 65 F:LPAR exp RPAR
void reduce_65(vector<struct node> & symbol, struct node & new_node) {
    struct node exp;
    full_copy_node(symbol[symbol.size() - 2], exp);
    copy_node(exp, new_node);
}

// 66 F:INT_NUM
void reduce_66(vector<struct node> & symbol, struct node & new_node) {
    // cout << symbol[symbol.size() - 1].name << " " << symbol[symbol.size() - 1].value << ";" << endl;
    struct node int_num;
    full_copy_node(symbol[symbol.size() - 1], int_num);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // if there are temp register available
    if (reg_tb.next_temp != -1) {
        cpfile << "addi $t" << (reg_tb.next_temp - 8) << " $zero " << int_num.value << endl;
        // set status of new node
        new_node.node_type = 1;
        new_node.reg_id = reg_tb.next_temp;
        new_node.size = 1;
        // set status of register
        reg_tb.reg_list[reg_tb.next_temp].if_busy = 2;
        maintain_reg_table();
        
    }// if there are no temp register available
    else {
        cpfile << "addi $t8 $zero " << int_num.value << endl;
        cpfile << "sw $t8 " << sp_reg << "($sp)" << endl;
        // set status of new node
        new_node.mem_pos = sp_reg;
        new_node.node_type = 2;
        new_node.size = 1;
        // update next memory position available
        sp_reg = sp_reg - 4;
    }

    cpfile.close();
}

// 67 F:ID LSQUARE exp RSQUARE
void reduce_67(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    struct node exp;
    full_copy_node(symbol[symbol.size() - 4], id);
    full_copy_node(symbol[symbol.size() - 2], exp);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // first load the value of exp to $t8
    if (exp.node_type == 1) {
        // if exp is in temp reg
        if (exp.reg_id >= 8) cpfile << "add $t8 $zero $t" << (exp.reg_id - 8) << endl;
        // if exp is in saved reg
        else cpfile << "add $t8 $zero $s" << (exp.reg_id) << endl;
    }
    else cpfile << "lw $t8 " << exp.mem_pos << "($sp)" << endl;
    
    // check if the variable is declared
    if (symbol_table.find(id.value) != symbol_table.end()) {
        // calculate the correct offeset, store it in $t9
        cpfile << "addi $t9 $zero -4" << endl;
        cpfile << "mul $t9 $t9 $t8" << endl;
        cpfile << "addi $t9 $t9 " << (symbol_table[id.value].mem_pos) << endl;
        // calculate correct memory position, store it in $t8
        cpfile << "add $t8 $t9 $sp" << endl;
        // load target value in $t9
        cpfile << "lw $t9 0($t8)" << endl;

        // store value of F at the space of exp if exp use temp/mem, otherwise(exp is an id), use new mem
        copy_node(exp, new_node);
        if (exp.node_type == 1) {
            // if exp is in temp reg
            if (exp.reg_id >= 8) cpfile << "add $t" << (exp.reg_id - 8) << " $t9 $zero" << endl;
            // if exp is in saved reg
            else {
                cpfile << "sw $t9 " << sp_reg << "($sp)" << endl;
                new_node.mem_pos = sp_reg;
                new_node.node_type = 2;
                new_node.size = 1;
                sp_reg = sp_reg - 4;
            }
        }
        else cpfile << "sw $t9 " << exp.mem_pos << "($sp)" << endl;
    }
    else {
        cout << "variable " << id.value << " is not declared, terminates" << endl;
        exit(1);
    }

    //F will use the stored space of exp here, so no need to free temp reg

    cpfile.close();
}

// 68 F:ID
void reduce_68(vector<struct node> & symbol, struct node & new_node) {
    struct node id;
    full_copy_node(symbol[symbol.size() - 1], id);
    ofstream cpfile;
    cpfile.open("compiler_output.txt", ios::app);

    // check if the variable is declared
    if (symbol_table.find(id.value) != symbol_table.end()) {
        id.mem_pos = symbol_table[id.value].mem_pos;
        id.node_type = symbol_table[id.value].status;
        id.reg_id = symbol_table[id.value].reg_id;
        load_8(id, cpfile);
        cpfile << "sw $t8 " << sp_reg << "($sp)" << endl;
        new_node.mem_pos = sp_reg;
        new_node.node_type = 2;
        new_node.size = 1;
        sp_reg = sp_reg - 4;
    }
    else {
        cout << "variable " << id.value << " is not declared, terminates" << endl;
        exit(1);
    }

    cpfile.close();
}

void cg_handle_reduce(vector<struct node> & symbol, struct node & new_node, int grammar_id) {
    if (grammar_id == 6) reduce_6(symbol, new_node);
    if (grammar_id == 7) reduce_7(symbol, new_node);
    if (grammar_id == 8) reduce_8(symbol, new_node);

    if (grammar_id == 13) stm();
    if (grammar_id == 14) stm();
    if (grammar_id == 15) stm();
    if (grammar_id == 16) stm();

    if (grammar_id == 23) reduce_23(symbol, new_node);
    if (grammar_id == 24) reduce_24(symbol, new_node);
    if (grammar_id == 25) reduce_25(symbol, new_node);
    if (grammar_id == 26) reduce_26(symbol, new_node);
    if (grammar_id == 27) reduce_27(symbol, new_node);

    if (grammar_id == 30) reduce_30(symbol, new_node);
    if (grammar_id == 31) reduce_31(symbol, new_node);
    if (grammar_id == 69) reduce_69(symbol, new_node);
    if (grammar_id == 32) reduce_32(symbol, new_node);
    if (grammar_id == 33) reduce_33(symbol, new_node);

    if (grammar_id == 34) reduce_34(symbol, new_node);
    if (grammar_id == 35) reduce_35(symbol, new_node);
    if (grammar_id == 36) reduce_36(symbol, new_node);
    if (grammar_id == 37) reduce_37(symbol, new_node);
    if (grammar_id == 38) reduce_38(symbol, new_node);
    if (grammar_id == 39) reduce_39(symbol, new_node);
    if (grammar_id == 40) reduce_40(symbol, new_node);
    if (grammar_id == 41) reduce_41(symbol, new_node);

    if (grammar_id == 42) reduce_42(symbol, new_node);
    if (grammar_id == 43) reduce_43(symbol, new_node);
    if (grammar_id == 44) reduce_44(symbol, new_node);
    if (grammar_id == 45) reduce_45(symbol, new_node);
    if (grammar_id == 46) reduce_46(symbol, new_node);
    if (grammar_id == 47) reduce_47(symbol, new_node);
    if (grammar_id == 48) reduce_48(symbol, new_node);
    if (grammar_id == 49) reduce_49(symbol, new_node);
    if (grammar_id == 50) reduce_50(symbol, new_node);
    if (grammar_id == 51) reduce_51(symbol, new_node);
    if (grammar_id == 52) reduce_52(symbol, new_node);
    if (grammar_id == 53) reduce_53(symbol, new_node);
    if (grammar_id == 54) reduce_54(symbol, new_node);
    if (grammar_id == 55) reduce_55(symbol, new_node);
    if (grammar_id == 56) reduce_56(symbol, new_node);
    if (grammar_id == 57) reduce_57(symbol, new_node);
    if (grammar_id == 58) reduce_58(symbol, new_node);
    if (grammar_id == 59) reduce_59(symbol, new_node);
    if (grammar_id == 60) reduce_60(symbol, new_node);
    if (grammar_id == 61) reduce_61(symbol, new_node);
    if (grammar_id == 62) reduce_62(symbol, new_node);
    if (grammar_id == 63) reduce_63(symbol, new_node);
    if (grammar_id == 64) reduce_64(symbol, new_node);
    if (grammar_id == 65) reduce_65(symbol, new_node);
    if (grammar_id == 66) reduce_66(symbol, new_node);
    if (grammar_id == 67) reduce_67(symbol, new_node);
    if (grammar_id == 68) reduce_68(symbol, new_node);
}