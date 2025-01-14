threaded: threaded.c
	clang threaded.c randtools.c -o threaded -I. -pthread
naive: naive.c
	clang randtools.c naive.c -o naive -I.
clean:
	rm threaded
	rm naive

