all : table parser scanner

scanner : ./scanner/added.o ./scanner/scanner.cpp
	g++ -o ./scanner/scanner ./scanner/added.o ./scanner/scanner.cpp

added.o : ./scanner/added.cpp
	g++ -c ./scanner/added.cpp

table : ./lr1_table/lr1_added.o ./lr1_table/lr1_table.cpp
	g++ -o ./lr1_table/table_gen ./lr1_table/lr1_added.o ./lr1_table/lr1_table.cpp

lr1_added.o : ./lr1_table/lr1_added.cpp
	g++ -c ./lr1_table/lr1_added.cpp

parser : code_gen.o parser.cpp
	g++ -o parser code_gen.o parser.cpp

code_gen.o : code_gen.cpp
	g++ -c code_gen.cpp

clean:
	rm -rf ./lr1_table/lr1_added.o ./lr1_table/table_gen parser code_gen.o ./scanner/added.o ./scanner/scanner token_list.txt parser_output.txt compiler_output.txt
