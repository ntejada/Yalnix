#include "std.h"

int main(int argc, char *argv[]) {
    int rc;
    rc = Fork();
    TracePrintf(3, "forkExecTest: rc = %d\n", rc);

    if (rc == 0) {
        TracePrintf(3, "==========> forkExecTest: In the child. About to exec exit56 then exit.\n");
        char *argv[3];
        argv[0] = "1";
        argv[1] = "exit56";
        argv[2] = NULL;
        Exec("exit56", argv);
    } else {
        TracePrintf(3, "forkExecTest: In the parent. About to Delay and call Wait.\n");
        int status;
        Delay(5);
        rc = Wait(&status);
        TracePrintf(3, "forkExecTest: Back from wait. status = %d, rc = %d\n", status, rc);
    }
}
