build_server: 
	gcc -o server.o src/main.c src/parser.c src/response.c 

clean: 
	rm -f server.o 
