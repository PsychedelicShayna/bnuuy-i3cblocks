
SECOND_uS := 1000000

blocks/cpu: cpu.c
	cc cpu.c -O3 -Wall -Wpedantic -Werror -DUSLEEPFOR=$(SECOND_uS) -o blocks/cpu

cputest: cpu.c
	cc cpu.c -O3 -Wall -Wpedantic -Werror -DUSLEEPFOR=$(SECOND_uS) -o blocks/dcpu

clean:
	rm ./blocks/cpu ./blocks/dcpu 2>&1>/dev/null


