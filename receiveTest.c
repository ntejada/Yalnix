int main(int argc, char *argv[]) {
	char * buf = (char*)malloc((sizeof(char)*1000));
	TtyPrintf(0, "%d\n", sizeof(*buf));
	int i;
	int len;
	while(1){
		TracePrintf(1, "gonna try TtyRead\n");
		len = TtyRead(0, (void*)(buf), 999); 
		TtyPrintf(0, "%s", buf);
		TtyPrintf(0, "\n%d received", len); 
	}
}
