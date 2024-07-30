#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

const std::string SHPathName = "/home/JZhong/code/linux-code/test_7_20";
const int SHProj_Id = 0x8585;

#define SHCreater 1
#define SHUser    0 
#define SHsize 4096

class Shm {
    std::string ToHex(int n) {
        char buff[128];
        int cx = snprintf(buff, sizeof(buff) - 1, "0x%x", n);
        buff[cx] = '\0';
        return buff;
    }

    int GetShm(int shmflg) {
        int sh = shmget(_key, SHsize, shmflg);
        if (sh == -1) {
            std::cout << "Create shm fail!" << "the reason is " << strerror(errno) << std::endl;
        }
        return sh;
    }

    std::string WhoamI() {
        if (_who == SHCreater)
            return "Creater";
        else if (_who == SHUser)
            return "User";
        else
            return "None";
    }
public:
    Shm(const std::string& pathname, int proj_id, int who)
        : _pathname(pathname)
        , _proj_id(proj_id) 
        , _who(who)
    {
        _key = ftok(pathname.c_str(), proj_id);
        // 创建共享内存
        if (who == SHCreater) {
            // 加入 0666 权限才可以将共享内存挂接上
            _shmid = this->GetShm(IPC_CREAT | IPC_EXCL | 0666);
            std::cout << "Creater create the shm, the key: " << ToHex(_key) << " shmid:" << _shmid << std::endl;
        } else {
            _shmid = this->GetShm(IPC_CREAT | 0666);
            std::cout << "User get the shm, the key: " << ToHex(_key) << " shmid:" << _shmid << std::endl;
        }
    }

    void* AttachShm() {
        void* shmaddr = shmat(_shmid, nullptr, 0);
        if (shmaddr == nullptr) {
            perror("shm attach: ");
            return nullptr;
        }
        std::cout << WhoamI() << " attach the shm" << std::endl;
        return shmaddr;
    }

    void DettachShm(void* shmaddr) {
        if (shmaddr == nullptr)
            return;
        shmdt(shmaddr);
    }

    ~Shm() {
        if (_who == SHCreater) {
            shmctl(_shmid, IPC_RMID, nullptr);
        }
        std::cout << "shm remove done..." << std::endl;
    }
private:
    std::string _pathname;
    int _proj_id;
    int _who;

    int _shmid;
    key_t _key;
};