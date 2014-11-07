int main(int argc, char *argv[]) {
	char * buf = (char*)malloc(sizeof(char)*11);
	int i;
	while(1){
		TracePrintf(1, "gonna try TtyRead\n");
		TtyRead(0, (void*)(buf), 10); 
		TtyPrintf(0, "%s", buf); 
	}
}
