#include <stdio.h>
void
DoIdle(void) {
  TracePrintf(1, "IN IDLE PROGRAM\n");
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}
int main(int argc, char*argv[]){
	DoIdle();
}
