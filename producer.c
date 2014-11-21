int resource_cnt;

int
main(int argc, char *argv[]) {

    int rc;

    int lock, cvar;
    LockInit(&lock);
    CvarInit(&cvar);

    for (int i = 0; i < 6; i++) {

	TracePrintf(2, "Producer: forking off child %d\n", i+1);
	rc = Custom0();

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
	    TracePrintf(2, "Producer: Process %d reacquired lock. Resource count is %d. Will decrement it then leave and signal next\n", GetPid(), resource_cnt);
	    resource_cnt--;
	    Release(lock);
	    CvarSignal(cvar);
	    Exit(1);
	}

    }

    TracePrintf(2, "Producer: Parent done with while loop\n");
    for (int i = 0; i < 6; i++) {
	Custom1();
	Wait(&rc);
    }
    TracePrintf(2, "Producer: Parent done waiting for children. Reclaiming cvar and lock\n");
    Reclaim(lock);
    Reclaim(cvar);

}
