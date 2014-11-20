
int global = 0;

int main(int argc, char *argv[]) {
    
    TracePrintf(1, "globalTest: parent, global currently is %d\n", global);
    global++;
    int rc = Custom0();

    if (0 == rc) {
	TracePrintf(1, "globalTest: child, global is %d. About to increment.\n", global);
	global++;
	Exit(4);

    }
    int status;
    rc = Wait(&status);
    TracePrintf(1, "globalTest: parent. global now is %d\n", global);

}
