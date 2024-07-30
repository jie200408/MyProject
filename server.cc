#include "Shm.hpp"

int main() {
    Shm shm(SHPathName, SHProj_Id, SHCreater);
    // sleep(5);
    char* addr = (char*)shm.AttachShm();
    // sleep(5);


    shm.DettachShm((void*)addr);
    return 0;
}