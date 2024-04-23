build_server: 
	gcc -o server.o src/main.c src/parser.c src/response.c src/request.c src/log.c -DLOG_USE_COLOR

clean: 
	rm -f server.o 
