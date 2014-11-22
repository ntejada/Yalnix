#include <yalnix.h>
#include "sem.h"
#include "std.h"

int main(int argc, char *argv[]) {
    int sem, rc;
    rc = SemInit(&sem, 5); 
    TtyPrintf(0, "Starting tests\n");

    for (int i = 0; i < 10; i++) {
        rc = Fork();
        if (0 == rc) {
            switch (i) {
                case 0:
                    // Test upping a semaphore that this process does not own
                    rc = SemUp(sem);
                    if (ERROR == rc) {
                        TtyPrintf(0, "Upping semaphore not owned failed as expected\n");
                        Exit(0);
                    } else {
                        TtyPrintf(0, "Upping semaphore not owned succeeded, unexpected behavior\n");
                        Exit(1);
                    }
                    break;
                case 1:
                    rc = SemDown(sem);
                    // Test double SemDown, have the behavior set to error if double request
                default:
                    rc = SemDown(sem);
                    Delay(10);
                    if (1 == i) {
                        if (ERROR == rc) {
                            TtyPrintf(0, "Double SemDown failed as expected\n");
                        } else {
                            TtyPrintf(0, "Double SemDown succeeded as unexpected behavior\n");
                            Exit(1);
                        }
                    } else {
                        if (SUCCESS == rc) {
                            TtyPrintf(0, "Successfully requested sem\n");
                        } else {
                            TtyPrintf(0, "Error on requesting sem\n");
                            Exit(1);
                        }
                    }
                    rc = SemUp(sem);
                    if (SUCCESS == rc) {
                        TtyPrintf(0, "Successfully released sem\n");
                        Exit(0);
                    } else {
                        TtyPrintf(0, "Error on releasing sem\n");
                        Exit(1);
                    }
                    break;
            } 
        }
    }

    // Parent wait for children
    for (int i = 0; i < 10; i++) {
        Wait(&rc);
        if (1 == rc) {
            TtyPrintf(0, "Something failed in the test, aborting\n");
            Exit(1);
        }
    }
    TtyPrintf(0, "All clear\n");
    Reclaim(sem);
}
