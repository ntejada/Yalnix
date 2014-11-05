int main(int argc, char *argv[]) {
	char * buf = (char*)calloc(sizeof(char)*11);
	while(1){
		TracePrintf(1, "gonna try TtyRead\n");
		TtyRead(0, (void*)(buf), 10); 
		TtyPrintf(0, "read first 10 chars from the terminal: %s\n", buf); 	
	}
}
