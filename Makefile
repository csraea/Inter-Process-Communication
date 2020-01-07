all: zadanie proc_p1 proc_p2 proc_t proc_d proc_serv2

zadanie: zadanie.c
	gcc zadanie.c -o zadanie

proc_p1: proc_p1.c
	gcc proc_p1.c -o proc_p1

proc_p2: proc_p2.c
	gcc proc_p2.c -o proc_p2

proc_t: proc_t.c
	gcc proc_t.c -o proc_t

proc_d: proc_d.c
	gcc proc_d.c -o proc_d

proc_serv2: proc_serv2.c
	gcc proc_serv2.c -o proc_serv2

clean:
	rm -f zadanie proc_p1 proc_p2 proc_d proc_t proc_serv2

