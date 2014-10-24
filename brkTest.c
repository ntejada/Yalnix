int main(int argc, char*argv[]){
	int i = 10;
	void * rc;
	while (i--) {
		TracePrintf(1, "in %s\n", argv[1]);
		rc = malloc(4000);	
		Pause(1);
	}
	free(rc);
}
