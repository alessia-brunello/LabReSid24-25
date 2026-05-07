#include <stdio.h>

int a;
static int b = 10;

void funzione(){
	static int c = 5;
	int d = 20;
	c++;
	d++;
}

int main(){
	funzione();
	return 0;

}
