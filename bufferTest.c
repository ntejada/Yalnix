int main(int argc, char *argv[]) {
    char *buf = 0;
    int rc;
    TracePrintf(2, "bufferTest: About to write with back buffer.\n");
    
    rc = TtyWrite(1, buf, 30);

    TracePrintf(2, "bufferTest: Back from writing with return code: %d\n", rc);


}
