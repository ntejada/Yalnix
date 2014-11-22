#include "std.h"
#define children 50
int resource_cnt;
int lock, cvar;

int
main(int argc, char *argv[]) {
    int rc;
    LockInit(&lock);
    CvarInit(&cvar);

    for (int i = 0; i < children; i++) {
        TracePrintf(0, "Parent: forking off child %d\n", i+1);
        rc = Spoon();

        if (0 == rc) {
            char string[60];
			if (i % 2) {
                // Producer
                strcpy(string, "Producer\n");
                TtyWrite(0, string, 9);
                Delay(4);
                Acquire(lock);
                resource_cnt++;
                CvarSignal(cvar);
                Release(lock);
                Exit(1);
            } else {
                // Consumer
                strcpy(string, "Consumer\n");
				TtyWrite(1, string, 9);
                Acquire(lock);
                while (resource_cnt <= 0) {
                    CvarWait(cvar, lock);
                }
                resource_cnt--;
                Release(lock);
                Exit(1);
            }
        }
    }

    for (int i = 0; i < children; i++) {
        PS();
        Wait(&rc);
    }
    Reclaim(lock);
    Reclaim(cvar);
}
