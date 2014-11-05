int main(int argc, char*argv[]){
	int rc;
	rc = Fork();
	if(rc == 0) {
		/*char *argv[2];
        argv[0] = "0";
        argv[1] = 0;
        Exec("printfTestExec", argv);*/
		TtyPrintf(0, "print test from child\n");	
	}
	else{
		TtyPrintf(0, "print test from parent\n"); 
		TracePrintf(1, "printfTest: about to wait for child to exit\n");
		Wait(&rc);
	}
}
