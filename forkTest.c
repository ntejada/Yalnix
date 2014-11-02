int main(int argc, char *argv[]) {
    int rc, status;
    rc = Fork();
    TracePrintf(3, "forkTest: rc = %d\n", rc);

    if (rc == 0) {
        TracePrintf(3, "==========> forkTest: In the child. About to call exit.\n");
	Exit(69);

    } else {
        TracePrintf(3, "forkTest: In the parent. About to call Delay so child can exit..\n");
	Delay(4);
	int status;
        TracePrintf(3, "forkTest: In the parent. About to call Wait.\n");
        rc = Wait(&status);
	TracePrintf(3, "forkTest: Back from wait. status = %d, rc = %d\n", status, rc);
		
    }
}
