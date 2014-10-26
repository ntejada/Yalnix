int main(int argc, char *argv[]) {
    int rc;
    rc = Fork();
    TracePrintf(3, "rc = %d\n", rc);

    if (rc == 0) {
        TracePrintf(3, "==========> In the child\n");
    } else {
        TracePrintf(3, "In the parent\n");
    }
}
