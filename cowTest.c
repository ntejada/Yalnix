#include "std.h"

int main(int argc, char *argv[]) {
	int rc;
	int stackVar = 10;
	int *heapVar = (int*)malloc(sizeof(int));
	*heapVar = 20;
	rc = Fork();
	TracePrintf(3, "cowTest: rc = %d\n", rc);
	if (rc == 0) {
		TracePrintf(3, "==========> cowTest: in the child. now time to test out this cow thing!\n");
		TracePrintf(3, "==========> cowTest: heapvar = %d\n", *heapVar);
		TracePrintf(3, "==========> cowTest: stackvar = %d\n", stackVar);
		TracePrintf(3, "==========> cowTest: now to change some variables and get some new frames\n");
		stackVar--;
		TracePrintf(3, "==========> cowTest: changed stackVar, but not heapVar\n");
		(*heapVar)--;
		
		TracePrintf(3, "==========> cowTest: decremented the variables\n");
		
		TracePrintf(3, "==========> cowTest: heapvar = %d\n", *heapVar);
		TracePrintf(3, "==========> cowTest: stackvar = %d\n", stackVar);
		
		char *argv[3];
		argv[0] = "1";
		argv[1] = "exit56";
		argv[2] = NULL;
		Exec("exit56", argv);
	} else {
		TracePrintf(3, "cowTest: In the parent. About to Delay and call Wait.\n");
		int status;
		Delay(5);
		TracePrintf(3, "back from delay! %d\n", GetPid());
		rc = Wait(&status);
		TracePrintf(3, "cowTest: Back from wait. status = %d, rc = %d\n", status, rc);
		TracePrintf(3, "==========> cowTest: heapvar = %d\n", *heapVar);
		TracePrintf(3, "==========> cowTest: stackvar = %d\n", stackVar);
		stackVar++;
		(*heapVar)++;
		TracePrintf(3, "==========> cowTest: heapvar = %d\n", *heapVar);
		TracePrintf(3, "==========> cowTest: stackvar = %d\n", stackVar);
	
	}
}
