all : bank
	
	
bank: bank.o string_parser.o account.o
	gcc -g -o bank bank.o string_parser.o account.o
	
bank.o: bank.c
	gcc -g -c bank.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c

account.o: account.c account.h
	gcc -g -c account.c 
	
clean:
	rm -f core *.o bank
	rm -f account_[0-9].txt
	rm -f output.txt