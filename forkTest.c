int main(int argc, char *argv[]) {
    int rc;
    rc = Fork();
    TracePrintf(3, "forkTest: rc = %d\n", rc);

    if (rc == 0) {
        TracePrintf(3, "==========> forkTest: In the child. About to delay then exit.\n");
	Delay(4);
	Exit(14);
	
    } else {
        TracePrintf(3, "forkTest: In the parent. About to call Wait.\n");
	int status;
        rc = Wait(&status);
	TracePrintf(3, "forkTest: Back from 1st wait. status = %d, rc = %d\nforkTest: calling fork again\n", status, rc);
	
	rc = Fork();
        TracePrintf(3, "forkTest: 2nd test: rc = %d\n", rc);
	if (0 == rc) {
	    TracePrintf(3, "=======> forkTest: In the 2nd child: About to delay then exit.\n");
	    Delay(3);
	    Exit(11);
	} else {
	    TracePrintf(3, "forkTest: Parent about to call wait again.\n", rc);
	    rc = Wait(&status);
	    TracePrintf(3, "forkTest: Back from 2nd wait. status = %d, rc = %d\n", status, rc);
	    
	}
    }
}
