#####################################################################
# ECEn 425 Lab 4 Makefile

YAK.bin:	YAKFinal.s
		nasm YAKFinal.s -o YAK.bin -l YAK.lst

YAKFinal.s:	clib.s myisr.s yaks.s myinth.s yakc.s lab4bapp.s
		cat clib.s myisr.s yaks.s myinth.s yakc.s lab4bapp.s > YAKFinal.s

myinth.s:	myinth.c
		cpp myinth.c myinth.i
		c86 -g myinth.i myinth.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab4bapp.s:	lab4b_app.c
		cpp lab4b_app.c lab4bapp.i
		c86 -g lab4bapp.i lab4bapp.s

clean:
		rm YAK.bin YAK.lst YAKfinal.s myinth.s myinth.i \
		yakc.s yakc.i lab4bapp.s lab4bapp.i
