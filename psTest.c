
int 
main(int argc, char *argv[]) {
    int rc, status;
    rc = Fork();
    if (0 == rc) {
	TracePrintf(1, "pcTest: In child. Calling PS\n");
		Custom1();
	Exit(13);
    } else {
	TracePrintf(1, "pcTest: In parent: about to call ps, then delay, then ps, then wait, then ps\n");
	Custom1();
	Delay(3);
	TracePrintf(1, "After delay\n");
	Custom1();
	rc = Wait(&status);
	TracePrintf(1, "pcTest: Parent back from wait. Child's exit status: %d\n", status);
	Custom1();

	return 1;
    }
    
}
