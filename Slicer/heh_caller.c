#include <process.h>
#include <stdio.h>

int main() {
   // call a function from another directory without any command line arguments
    _spawnv(_P_WAIT, "heh.exe", NULL);
    return 0;
}