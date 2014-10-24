int main(int argc, char*argv[]){
	TracePrintf(1, "in brkTest\n");
	int pid;
	int i = 10;
	void *rc;
	while (i-->0) {
		pid = GetPid();
		TracePrintf(1, "Hello World I am process %d\n", pid);
		rc = (void *)malloc(4000);
		Pause(1);
	}
	free(rc);
}
