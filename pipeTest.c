int main(int argc, char **argv) {
    int pipe = PipeInit();
    void *buf = malloc(33);
    int rc = Fork();
    if (rc = 0) {
        while (1) {
            memset(buf, 0, 33);
            PipeRead(pipe, buf, 32);
            TtyPrintf(1, "%s", buf);
        }
    } else {
        while (1) {
            int len = TtyRead(0, buf, 32);
            PipeWrite(pipe, buf, len);
        }
    }
}
