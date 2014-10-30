int main(int argc, char *argv[]) {
    int rc;
    rc = Fork();
    TracePrintf(3, "zcbTest: rc = %d\n", rc);

    if (rc == 0) {
        TracePrintf(3, "==========> zcbTest: In the child. About to delay then exit.\n");
	//Delay(4);
	Exit(14);
	
    } 
	else {
        TracePrintf(3, "zcbTest: In the parent. About to call Wait.\n");
	int status;
        Delay(10);
	rc = Wait(&status);
	TracePrintf(3, "zcbTest: Back from 1st wait. status = %d, rc = %d\nzcbTest: calling fork again\n", status, rc);
	
	rc = Fork();
        TracePrintf(3, "zcbTest: 2nd test: rc = %d\n", rc);
	if (0 == rc) {
	    TracePrintf(3, "=======> zcbTest: In the 2nd child: About to delay then exit.\n");
	    Delay(3);
	    Exit(11);
	} else {
	    TracePrintf(3, "zcbTest: Parent about to call wait again.\n", rc);
	    rc = Wait(&status);
	    TracePrintf(3, "zcbTest: Back from 2nd wait. status = %d, rc = %d\n", status, rc);
	    
	}
    }
}
