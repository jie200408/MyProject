#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "sqlite.hpp"

// int select_stu_callback(void* arg, int col_num, char** result, char** name) {
//     std::vector<std::string>* array = (std::vector<std::string>*)arg;
//     array->push_back(result[0]);
//     return 0;
// }

// 对于改函数。存在多少个元素，就会调用多少次
int select_stu_callback(void* arg,int col_count,char** result,char** fields_name) {
    std::vector<std::string> *arry = (std::vector<std::string>*)arg;
    // std::cout << "xx" << std::endl;
    arry->push_back(result[0]);
    return 0; //必须有！！！
}

int main() {
    SqliteHelper helper("test.db");
    assert(helper.open());

    // 创建表、增加元素、修改元素、查看元素
    std::string create_sql = "create table if not exists student (sn int primary key, name varchar(20), age int);";
    assert(helper.exec(create_sql, nullptr, nullptr));

    // 增加元素
    std::string insert_sql = "insert into student values (3, '小黄', 15), (4, '小白', 16), (5, '小黑', 19);";
    assert(helper.exec(insert_sql, nullptr, nullptr));

    // 修改元素
    // std::string modify_sql = "update student set name='阿黄' where sn = 1;";
    // assert(helper.exec(modify_sql, nullptr, nullptr));

    // 删除元素
    // std::string delete_sql = "delete from student where sn = 3;";
    // assert(helper.exec(delete_sql, nullptr, nullptr));


    // 查看元素
    std::string search_sql = "select name from student;";
    std::vector<std::string> v_name;
    assert(helper.exec(search_sql, select_stu_callback, &v_name));
    
    for (auto& it : v_name)
        std::cout << it << std::endl;
    
    helper.close();

    return 0;
}