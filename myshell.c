#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define SIZE 512 
#define NUM 32
enum{NoRedir, InRedir, OutRedir, AppRedir};
int Redir = 0;
int RedirPos = 0;

char cwd[SIZE * 2];
char* MyArgv[NUM];
char tmp[SIZE * 2];
int lastcode = 0;

const char* GetUserName(){
    const char* name = getenv("USER");
    if(name == NULL) return "None";
    return name;
}

const char* GetHostName(){
    const char* hostname = getenv("HOSTNAME");
    if(hostname == NULL) return "None";
    return hostname;
}

const char* GetCwd(){
    const char* cwd = getenv("PWD");
    if(cwd == NULL) return "None";
    return cwd;
}

void MakeCommandLine(){
    const char* name = GetUserName();
    const char* hostname = GetHostName();
    const char* cwd = GetCwd();
    if(strlen(cwd) != 1){
        cwd += strlen(cwd) - 1;
        while(*cwd != '/')
            cwd--;
        cwd++;
    }
    printf("[%s@%s %s]> ", name, hostname, cwd);
}

char* GetCommandFromStdin(){
    char* command = fgets(tmp, sizeof(tmp), stdin);
    if(command == NULL) exit(1);
    command[strlen(command) - 1] = '\0';
    return command;
}

void SplitComandToArgv(char* command){
    // 开始截取
    MyArgv[0] = strtok(command, " ");
    //MyArgv[0] = command;
    //char* tmp = strtok(command, " ");
    //tmp = '\0';
    int index = 1;
    while((MyArgv[index++] = strtok(NULL, " "))){

    }
    MyArgv[index] = NULL;
}

void ExecuteCommand(){
    pid_t id = fork();
    if(id < 0) exit(1);
    else if(id == 0){
        execvp(MyArgv[0], MyArgv);
        exit(errno);
    }else{
        int status = 0;
        pid_t rid = waitpid(id, &status, 0);
        if(rid > 0){
            lastcode = WEXITSTATUS(status);
            if(lastcode != 0)
                printf("%s:%s:%d\n", MyArgv[0], strerror(lastcode), lastcode);
        }
    }
}

void ChangeDir(){
    char* path = MyArgv[1];
    if(strcmp(MyArgv[1], "-") == 0) path = getenv("HOME");
    chdir(path);
    // 更新环境变量
    getcwd(tmp, sizeof(tmp));
    snprintf(cwd, sizeof(cwd), "PWD=%s", tmp);
    putenv(cwd);
}

int BuildInCommand(){
    int yes = 0;
    if(strcmp(MyArgv[0], "cd") == 0){
        yes = 1;
        ChangeDir();
    }
    return yes;
}

void CheckRedir(char* cmd){
    // 检测是否存在重定向
    int len = strlen(cmd);
    int i = 0;
    for(; i < len; i++){
        if(cmd[i] == '<'){
            Redir = InRedir;
            cmd[i++] = 0;
            while(i < len && cmd[i] == ' ')
                i++;
            RedirPos = i;
            break;
        }else if(cmd[i] == '>' && i != len -1){
            cmd[i++] = 0;
            if(i < len && cmd[i] == '>')
                Redir = AppRedir, cmd[i++] = 0;
            else
                Redir = OutRedir;
            while(i < len && cmd[i] == ' ')
                i++;
            RedirPos = i;
            break;
        }
    }
    if(i == len) Redir = NoRedir;
}

void RedirSet(char* cmd){
    if(Redir == NoRedir) return;
    char* filename = cmd + RedirPos;
    if(filename == NULL) return;
    if(Redir == InRedir){
        int fd = open(filename, O_RDONLY);
        dup2(fd, 0);
    }else if(Redir == OutRedir){
        int fd = open(cmd + RedirPos, O_WRONLY | O_CREAT| O_TRUNC, 0666);
        dup2(fd, 1);
    }else{
        int fd = open(cmd + RedirPos, O_WRONLY | O_CREAT| O_APPEND, 0666);
        dup2(fd, 1);
    }
    
}

int main(){
    // 先创建一个命令行输入
    int quit = 1;
    while(quit != 0){
        Redir = NoRedir;
        RedirPos = 0;
        MakeCommandLine();    
         // 创建一个读入字符串
        char* command = GetCommandFromStdin();
        //printf("%s", command);
        // 将command指令截取到argv表格中
        
        // 检测是否存在重定向
        CheckRedir(command);
        RedirSet(command); 
        //printf("cmd: %s\n", command);
        //printf("file: %s\n", command + RedirPos);
        SplitComandToArgv(command);
        //int i = 0;
        //for(; MyArgv[i]; i++){
        //    printf("%s\n", MyArgv[i]);
        //}
        // 截取命令之后，就可以开始执行程序了
        int buildin = BuildInCommand();
        if(buildin) continue;
        ExecuteCommand();
    }
    return 0;
}
