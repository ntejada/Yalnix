int
main(int argc, char *argv[]) {

    int rc, resource_cnt;

    int lock, cvar;
    LockInit(&lock);
    CvarInit(&cvar);

    for (int i = 0; i < 6; i++) {

	TracePrintf(2, "Producer: forking off child %d\n", i+1);
	rc = Fork();

	if (0 == rc) {
	    TracePrintf(2, "Producer: Process %d trying to acquire lock\n", GetPid());
	    Acquire(lock);
	    TracePrintf(2, "Producer: Process %d has lock, resource_cnt: %d\n", GetPid(), resource_cnt);
	    while (resource_cnt >=  2) {
		TracePrintf(2, "Producer: Process %d condition not met. Will call cvarwait.\n", GetPid());
		CvarWait(cvar, lock);
	    }
	    resource_cnt++;
	    Release(lock);

	    TracePrintf(2, "Producer: Process %d outside first critical section. About to delay and then re-acquire lock.\n", GetPid());

	    Delay(2);


	    Acquire(lock);
	    resource_cnt--;
	    Release(lock);
	    CvarSignal(cvar);
	    Exit(1);
	}

    }

    TracePrintf(2, "Producer: Parent done with while loop\n");
    for (int i = 0; i < 6; i++)
	Wait(&rc);

    TracePrintf(2, "Producer: Parent done waiting for children. Reclaiming cvar and lock\n");
    Reclaim(lock);
    Reclaim(cvar);

}
