#include "std.h"
#define PROC_COUNT 50

int resource_cnt;
int lock, cvar;
int rc;

int
main(int argc, char *argv[]) {
    resource_cnt = 0;
    LockInit(&lock);
    CvarInit(&cvar);

    for (int i = 0; i < PROC_COUNT; i++) {
        TtyPrintf(0, "Parent: forking off child %d\n", i + 2);
        rc = Spoon();

        if (0 == rc) {
            if (i % 2) {
                // Producer
                Delay(5);
                Acquire(lock);
                TtyPrintf(1, "Producer\n");
                resource_cnt++;
                CvarSignal(cvar);
                Release(lock);
                Exit(1);
            } else {
                // Consumer
                Acquire(lock);
                while (resource_cnt <= 0) {
                    CvarWait(cvar, lock);
                }
                TtyPrintf(2, "Consumer\n");
                resource_cnt--;
                Release(lock);
                Exit(1);
            }
        }
    }

    for (int i = 0; i < PROC_COUNT; i++) {
        //PS();
        Wait(&rc);
    }

    Reclaim(lock);
    Reclaim(cvar);
}
