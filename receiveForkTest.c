#include "std.h"

int main(int argc, char *argv[]) {
	char * buf = (char*)calloc(sizeof(char)*11);
	int children = 5;
	int rc = 1;
	int status;	
	while(children--){
		if(rc != 0){
			rc = Fork();
		}
		else{
			break;
		}		
	}
	if(rc == 0) {
			TtyRead(0, ((void*)(buf)), 10);
			TtyPrintf(0, "%s\n", buf);
	}
	children = 5;
	while(children--){
		Wait(&status);
	}
}
