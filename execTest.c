
int main(int argc, char* argv[]){
	char* args[3];
	args[0] = "1";
	args[1] = "initInit";
	args[2] = 0;
	TracePrintf(1, "in exec test\n");
	Exec("./initInit", args);
}
