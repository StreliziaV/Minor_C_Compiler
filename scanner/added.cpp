#include "added.h"
#include <iostream>
#include <fstream>
//                      1    2     3        4       5       6       7       8       9         10       11     12              13         14        15         16         17       18       19      20      21      22      23          24       25        26        27       28     29         30       31       32    33    34        35     36      37        38     39
char tokens[][10] = {"INT", "IF", "DO", "RETURN", "MAIN", "ELSE", "VOID", "READ", "BREAK", "WHILE", "WRITE", "ID", "INT_NUM", "LBRACE", "RBRACE", "LSQUARE", "RSQUARE", "LPAR", "RPAR", "SEMI", "PLUS", "MINUS", "MUL_OP", "DIV_OP", "AND_OP", "ANDAND", "OR_OP", "OROR", "NOT_OP", "NOTEQ", "ASSIGN", "EQ", "LT", "SHL_OP", "LTEQ", "GT", "SHR_OP", "GTEQ", "COMMA"};

int hello_world() {
    std::cout << "hello world" << std::endl;


    return 1;
}

void add_ID_terminal(int (&DFA)[80][128]) {
    char ter_symbols[] = {' ', '\n', '{', '}', '[', ']', '(', ')', ';', '+', '-', '*', '/', '&', '|', '!', '=', '<', '>', ','};
    for (int i = 1; i <= 49; i++) {
        for (int j = 0; j < sizeof(ter_symbols)/sizeof(char); j++) {
            int temp = ter_symbols[j];
            DFA[i][temp] = -12; // if the value < 0, accept
        }
    }
}

//state 0, 46, 47, 48, 49, token ID: 12
void ini_ID(int (&DFA)[80][128]) {
    // ID: 46, 47, 48, 49
    for (int i = 0; i <= 45; i++) {
        for (int j = 65; j <= 90; j++) {
            DFA[i][j] = 46;
            DFA[i][j + 32] = 46;
        }
        if (i == 0) continue;
        for (int k = 48; k <= 57; k++) {
            DFA[i][k] = 48;
        }
        DFA[i][95] = 49;
    } 
    //46
    for (int i = 65; i <= 90; i++) { // a-z A-Z
        DFA[46][i] = 47;
        DFA[46][i + 32] = 47;
    }
    for (int i = 48; i <= 57; i++) { // 0-9
        DFA[46][i] = 48;
    }
    DFA[46][95] = 49; // _
    //47
    for (int i = 65; i <= 90; i++) { // a-z A-Z
        DFA[47][i] = 47;
        DFA[47][i + 32] = 47;
    }
    for (int i = 48; i <= 57; i++) { // 0-9
        DFA[47][i] = 48;
    }
    DFA[47][95] = 49; // _
    //48
    for (int i = 65; i <= 90; i++) { // a-z A-Z
        DFA[48][i] = 47;
        DFA[48][i + 32] = 47;
    }
    for (int i = 48; i <= 57; i++) { // 0-9
        DFA[48][i] = 48;
    }
    DFA[48][95] = 49; // _
    //49
    for (int i = 65; i <= 90; i++) { // a-z A-Z
        DFA[49][i] = 47;
        DFA[49][i + 32] = 47;
    }
    for (int i = 48; i <= 57; i++) { // 0-9
        DFA[49][i] = 48;
    }
    DFA[49][95] = 49; // _

    add_ID_terminal(DFA);
}

//state 0, 1 to 45, token 1 to 11
void ini_keywords(int (&DFA)[80][128]) {
    //int: 1 if: 2
    DFA[0][105] = 1;
    DFA[1][110] = 2;
    DFA[2][116] = 3;
    DFA[3][32] = -1; // if the value < 0, accept
    DFA[3][10] = -1; // empty space: 32, new line: 10
    //if: 2
    DFA[1][102] = 4;
    DFA[4][32] = -2;
    DFA[4][10] = -2;
    DFA[4]['('] = -2;

    //do: 3
    DFA[0][100] = 5;
    DFA[5][111] = 6;
    DFA[6][32] = -3;
    DFA[6][10] = -3;
    DFA[6]['{'] = -3;

    //return: 4
    DFA[0][114] = 7;
    DFA[7][101] = 8;
    DFA[8][116] = 9;
    DFA[9][117] = 10;
    DFA[10][114] = 11;
    DFA[11][110] = 12;
    DFA[12][32] = -4;
    DFA[12][10] = -4;
    DFA[12][';'] = -4;

    //main: 5
    DFA[0][109] = 13;
    DFA[13][97] = 14;
    DFA[14][105] = 15;
    DFA[15][110] = 16;
    DFA[16][32] = -5;
    DFA[16][10] = -5;
    DFA[16]['('] = -5;

    //else: 6
    DFA[0][101] = 17;
    DFA[17][108] = 18;
    DFA[18][115] = 19;
    DFA[19][101] = 20;
    DFA[20][32] = -6;
    DFA[20][10] = -6;
    DFA[20]['{'] = -6;

    //void: 7
    DFA[0][118] = 21;
    DFA[21][111] = 22;
    DFA[22][105] = 23;
    DFA[23][100] = 24;
    DFA[24][32] = -7;
    DFA[24][10] = -7;

    //scanf: 8
    DFA[0][115] = 25;
    DFA[25][99] = 26;
    DFA[26][97] = 27;
    DFA[27][110] = 28;
    DFA[28][102] = 29;
    DFA[29]['('] = -8;

    //break: 9
    DFA[0][98] = 30;
    DFA[30][114] = 31;
    DFA[31][101] = 32;
    DFA[32][97] = 33;
    DFA[33][107] = 34;
    DFA[34][32] = -9;
    DFA[34][10] = -9;
    DFA[34][';'] = -9;

    //while: 10
    DFA[0][119] = 35;
    DFA[35][104] = 36;
    DFA[36][105] = 37;
    DFA[37][108] = 38;
    DFA[38][101] = 39;
    DFA[39][32] = -10;
    DFA[39][10] = -10;
    DFA[39]['('] = -10;

    //printf: 11
    DFA[0][112] = 40;
    DFA[40][114] = 41;
    DFA[41][105] = 42;
    DFA[42][110] = 43;
    DFA[43][116] = 44;
    DFA[44][102] = 45;
    DFA[45]['('] = -11;
}

//state 0, 50, 51, token INT_NUM: 13
void ini_int(int (&DFA)[80][128]) {
    for (int i = 48; i <= 57; i++) {
        DFA[0][i] = 50;
        DFA[50][i] = 51;
        DFA[51][i] = 51;
    }

    char ter_symbols[] = {' ', '\n', '{', '}', '[', ']', '(', ')', ';', '+', '-', '*', '/', '&', '|', '!', '=', '<', '>', ','};
    for (int i = 50; i <= 51; i++) {
        for (int j = 0; j < sizeof(ter_symbols)/sizeof(char); j++) {
            int temp = ter_symbols[j];
            DFA[i][temp] = -13; // if the value < 0, accept
        }
    }
}

//state 0, 52 to 77, token 14 to 39
void ini_symbols(int (&DFA)[80][128]) {
    char single_symbols[] = {'{', '}', '[', ']', '(', ')', ';', '+', '-', '*', '/'};
    for (int i = 0; i < sizeof(single_symbols)/sizeof(char); i++) {
        DFA[0][single_symbols[i]] = 52 + i;
        for (int j = 0; j < 128; j++) DFA[52 + i][j] = - (14 + i);
    }
    // ,: 39
    DFA[0][','] = 77;
    for (int j = 0; j < 128; j++) DFA[77][j] = - 39;

    // &: 25 &&: 26
    DFA[0]['&'] = 63;
    for (int j = 0; j < 128; j++) DFA[63][j] = - 25;
    DFA[63]['&'] = 64;
    for (int j = 0; j < 128; j++) DFA[64][j] = - 26;

    // |: 27 ||: 28
    DFA[0]['|'] = 65;
    for (int j = 0; j < 128; j++) DFA[65][j] = - 27;
    DFA[65]['|'] = 66;
    for (int j = 0; j < 128; j++) DFA[66][j] = - 28;

    // !: 29 !=: 30
    DFA[0]['!'] = 67;
    for (int j = 0; j < 128; j++) DFA[67][j] = - 29;
    DFA[67]['='] = 68;
    for (int j = 0; j < 128; j++) DFA[68][j] = - 30;

    // =: 31 ==: 32
    DFA[0]['='] = 69;
    for (int j = 0; j < 128; j++) DFA[69][j] = - 31;
    DFA[69]['='] = 70;
    for (int j = 0; j < 128; j++) DFA[70][j] = - 32;

    // <: 33 <<: 34 <= 35
    DFA[0]['<'] = 71;
    for (int j = 0; j < 128; j++) DFA[71][j] = - 33;
    DFA[71]['<'] = 72;
    DFA[71]['='] = 73;
    for (int j = 0; j < 128; j++) DFA[72][j] = - 34;
    for (int j = 0; j < 128; j++) DFA[73][j] = - 35;

    // >: 36 >>: 37 >=: 38
    DFA[0]['>'] = 74;
    for (int j = 0; j < 128; j++) DFA[74][j] = - 36;
    DFA[74]['>'] = 75;
    DFA[74]['='] = 76;
    for (int j = 0; j < 128; j++) DFA[75][j] = - 37;
    for (int j = 0; j < 128; j++) DFA[76][j] = - 38;
}

void initialize_DFA(int (&DFA)[80][128]) {
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 128; j++) {
            DFA[i][j] = 0;
        }
    }

    ini_ID(DFA);

    ini_keywords(DFA);

    ini_symbols(DFA);  

    ini_int(DFA);
}

void get_token(int token_no, struct record_struct & record) {
    token_no = - token_no;

    std::ofstream outfile;
    outfile.open("token_list.txt", std::ios::app);
    outfile << tokens[token_no - 1] << ":";
    for (int i = 0; i < record.size; i++) outfile << record.rec[i];
    outfile << std::endl;
    outfile.close();

    return;
}