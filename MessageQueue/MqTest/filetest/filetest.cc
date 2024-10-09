#include "../../MqCommon/helper.hpp"
#include "../../MqCommon/logger.hpp"

int main() {
    // mq::FileHelper file_helper("../../MqCommon/logger.hpp");
    // DLOG("文件是否存在：%d\n", file_helper.exists());
    // DLOG("文件大小：%ld\n", file_helper.size());

    // mq::FileHelper tmp_file("./a/b/c/tmp.cc");
    // if (!tmp_file.exists()) {
    //     // 若当前文件不存在，则先创建文件夹，然后创建文件
    //     std::string path = mq::FileHelper::parentDirectory("./a/b/c/tmp.cc");
    //     if (mq::FileHelper(path).exists() == false) 
    //         // 则直接创建目录
    //         mq::FileHelper::createDirectory(path);
    //     mq::FileHelper::createFile("./a/b/c/tmp.cc");
               
    // }
    // DLOG("文件是否存在：%d\n", tmp_file.exists());
    
    // mq::FileHelper file_helper("../../MqCommon/logger.hpp");
    // // 从对应的文件中读出数据
    // std::string body;
    // file_helper.read(body);
    // ILOG("\n%s\n", body.c_str());
    // mq::FileHelper tmp_file("new_temp.cc");
    // // tmp_file.write(body);
    // tmp_file.rename("./a/b/c/tmp.cc");
    // mq::FileHelper tmp_file("./a/b/c/tmp.cc");
    // char str[30] = { 0 };
    // tmp_file.read(str, 8, 11);
    // ILOG("[%s]\n", str);
    // std::string temp("12345678901");
    // tmp_file.write(temp.c_str(), 8, temp.size());
    // mq::FileHelper::removeFile("./a/b/c/tmp.cc");
    mq::FileHelper::removeDirectory("./a");
    return 0;
}