#include "std.h"

int resource_cnt;
int lock, cvar;

int
main(int argc, char *argv[]) {
    int rc;
    LockInit(&lock);
    CvarInit(&cvar);

    for (int i = 0; i < 3; i++) {
        TtyPrintf(0, "Parent: forking off child %d\n", i+1);
        rc = Spoon();

        if (0 == rc) {
            if (i % 2) {
                // Producer
                TtyPrintf(1, "Producer: Process %d trying to acquire lock\n", GetPid());
                Delay(2);
                Acquire(lock);
                TtyPrintf(1, "Producer: Process %d has lock, resource_cnt: %d\n", GetPid(), resource_cnt);
                resource_cnt++;
                TtyPrintf(1, "Producer: Process %d Incrementing resource_cnt: %d\n", GetPid(), resource_cnt);
                CvarSignal(cvar);
                Release(lock);
                Exit(1);
            } else {
                // Consumer
                TtyPrintf(2, "Consumer: Process %d outside first critical section.\n", GetPid());
                Acquire(lock);
                TtyPrintf(2, "Consumer: Process %d acquired lock. Resource count is %d.\n", GetPid(), resource_cnt);
                while (resource_cnt <= 0) {
                    TtyPrintf(2, "Consumer: Process %d condition not met. Will call cvarwait.\n", GetPid());
                    CvarWait(cvar, lock);
                }
                resource_cnt--;
                Release(lock);
                Exit(1);
            }
        }
    }

    TtyPrintf(0, "Parent done with while loop\n");
    for (int i = 0; i < 6; i++) {
        //PS();
        Wait(&rc);
    }
    TtyPrintf(0, "Parent done waiting for children. Reclaiming cvar and lock\n");
    //Reclaim(lock);
    //Reclaim(cvar);
}
