default: all;

all: frontend.o backend.o
	gcc frontend.o -o frontend -l pthread
	gcc backend.o users_lib.o -o backend -l pthread

fronted.o: frontend.c
	gcc -c frontend.c

backend.o: backend.c
	gcc -c backend.c

clean:
	@echo "A limpar..."
	rm frontend backend frontend.o backend.o /home/joaosantos/SO-PIPES/PIPE*
