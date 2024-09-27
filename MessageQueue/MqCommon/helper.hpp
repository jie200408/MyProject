#ifndef __M_HELPER_H__
#define __M_HELPER_H__

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sqlite3.h>
#include "logger.hpp"

namespace mq {

    class SqliteHelper {
    private:
        typedef int(*SqliteCallback)(void*, int, char**, char**);
    public:
        SqliteHelper(const std::string& dbfile)
            : _dbfile(dbfile),
            _handler(nullptr)
        {}

        bool open(int safe_lavel = SQLITE_OPEN_FULLMUTEX) {
            int ret = sqlite3_open_v2(_dbfile.c_str(), &_handler, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | safe_lavel, nullptr);
            if (ret != SQLITE_OK) {
                ELOG("创建/打开sqlite失败: %s\n", sqlite3_errmsg(_handler));
                return false;
            }
            return true;
        }

        bool exec(const std::string& sql, SqliteCallback cb, void* arg) {
            // sqlite3_exec(_handler, sql.c_str(), cb, arg, nullptr);
            int ret = sqlite3_exec(_handler, sql.c_str(), cb, arg, nullptr);
            if (ret != SQLITE_OK) {
                ELOG("%s \n, 执行语句失败: %s\n", sql.c_str(), sqlite3_errmsg(_handler));
                return false;
            }
            return true;
        }

        void close() {
            if (_handler)
                sqlite3_close_v2(_handler);
        }

    private:
        std::string _dbfile;
        sqlite3* _handler;
    };

    class StrHelper {
    public:
        static size_t split(const std::string& str, const std::string& sep, std::vector<std::string>& result) {
            // new....music.#pop...
            size_t pos = 0, index = 0;
            while (index < str.size()) {
                pos = str.find(sep, index);
                if (pos == std::string::npos) {
                    // 最后一次没有找到
                    std::string tmp = str.substr(index);
                    result.push_back(std::move(tmp));
                    return result.size();
                }
                if (index == pos) {
                    index = pos + sep.size();
                    continue;
                }
                std::string tmp = str.substr(index, pos - index);
                result.push_back(std::move(tmp));
                index = pos + sep.size();
            }
            return result.size();
        }        
    };

    class UUIDHelper {
    public:
        static std::string uuid() {
            // uuid的数据格式为一个8-4-4-4-12的16进制字符串，如：7a91a05f-52a1-6a01-0000-000000000001
            std::random_device rd;
            // 使用一个机器随机数作为伪随机数的种子
            // 机器数：使用硬件生成的一个数据，生成效率较低
            std::mt19937_64 generator(rd());
            // 生成的数据范围为0~255
            std::uniform_int_distribution<int> distribution(0, 255);
            std::stringstream ss;
            for (int i = 0; i < 8; i++) {
                ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
                if (i == 3 || i == 5 || i == 7)
                    ss << "-";
            }
            // std::cout << ss.str() << std::endl;
            static std::atomic<size_t> seq(1);
            size_t num = seq.fetch_add(1);
            for (int i = 7; i >= 0; i--) {
                ss << std::setw(2) << std::setfill('0') << std::hex << (num >> (i * 8));
                if (i == 6)
                    ss << "-";
            }     
            return ss.str();   
        }
    };    

    class FileHelper {
    public:
        FileHelper(const std::string& filename)
            : _filename(filename)
        {}
        
        bool exists() {
            struct stat st;
            return (stat(_filename.c_str(), &st) == 0);
        }

        size_t size() {
            struct stat st;
            if (!exists()) 
                return 0;
            stat(_filename.c_str(), &st);
            return st.st_size;
        }

        bool read(char* body, size_t offset, size_t len) {
            std::ifstream ifs(_filename.c_str(), std::ios::binary | std::ios::in);
            if (!ifs.is_open()) {
                ELOG("%s, 打开文件失败\n", _filename.c_str());
                return false;
            }
            ifs.seekg(offset, std::ios::beg);
            ifs.read(body, len);
            if (!ifs.good()) {
                ELOG("%s, 文件读取失败\n", _filename.c_str());
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        bool read(std::string& body) {
            size_t filesize = this->size();
            return this->read(&body[0], 0, filesize);
        }

        bool write(const char* body, size_t offset, size_t len) {
            std::fstream fs(_filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);
            if (!fs.is_open()) {
                ELOG("%s, 打开文件失败\n", _filename.c_str());
                return false;
            }     
            fs.seekp(offset, std::ios::beg);
            fs.write(body, len);
            if (!fs.good()) {
                ELOG("%s, 文件写入失败\n", _filename.c_str());
                fs.close();
                return false;
            }           

            fs.close();
            return true;       
        }

        bool write(const std::string& body) {
            return this->write(body.c_str(), 0, body.size());
        }

        bool rename(const std::string& new_name) {
            std::string old_name(_filename);
            int ret = ::rename(old_name.c_str(), new_name.c_str());
            if (ret == 0) {
                _filename = new_name;
                return true;
            } else {
                return false;
            }
        }

        bool createFile() {
            std::ifstream ifs(_filename.c_str(), std::ios::binary | std::ios::in);
            if (!ifs.is_open()) {
                ELOG("%s, 文件创建失败\n", _filename.c_str());
                return false;
            }    
            ifs.close();
            return true;        
        }

        static bool removeFile(const std::string& filename) {
            return (::remove(filename.c_str()) == 0);
        }

        bool createDirectory(const std::string& path) {
            // "aaa/ccc/sss/qwqw"
            size_t pos = 0, index = 0;
            std::string sep = "/";
            while (index < path.size()) {
                pos = path.find(sep, index);
                if (pos == std::string::npos) {
                    int ret = ::mkdir(path.c_str(), 0775);
                    if (ret != 0) {
                        ELOG("%s 创建目录失败\n", path.c_str());
                        return false;
                    } else {
                        return true;
                    }
                }
                std::string sub_path = path.substr(0, pos);
                int res = ::mkdir(sub_path.c_str(), 0775);
                if (res != 0) {
                    ELOG("%s 创建目录失败\n", sub_path.c_str());
                    return false;
                }
                index = pos + sep.size();
            }

            return true;
        }

        bool removeDirectory(const std::string& path) {
            // 使用指令删除目录
            std::string cmd = "rm -rf " + path;
            return (::system(cmd.c_str()) != 0);
        }

        static std::string parentDirectory(const std::string& file_path) {
            // "aaa/ccc/sss/qwqw"
            size_t pos = file_path.find_last_of("/");
            if (pos == std::string::npos)
                return "./";
            return file_path.substr(0, pos);
        }

    private:
        std::string _filename;
    };
}



#endif