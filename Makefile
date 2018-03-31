all:
	gcc -c command.c
	yacc -d shell.y
	lex shell.l
	gcc y.tab.c -ll -ly command.o
