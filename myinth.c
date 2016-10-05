
#include "clib.h"
extern int KeyBuffer;

int counter = 0;
int i=0;
int k=0;

void rISR(){
	exit(0);
}

void tick(){
	counter++;
	printNewLine();
	printString("TICK ");
	printInt(counter);
	printNewLine();
}

void keystroke(){
	if(KeyBuffer!='d'){
		printNewLine();
		printString("KEYPRESS ");
		printChar(KeyBuffer);
		printString(" IGNORED");
		printNewLine();
	}
	else{
		printNewLine();
		printString("DELAY KEY PRESSED");
		printNewLine();
		i=0;
		for(k=0;k<5000;k++){
			i++;
		}
		printString("DELAY COMPLETE");
		printNewLine();
	}
}
