#ifndef __M_ROUTE_H__
#define __M_ROUTE_H__

#include "../MqCommon/msg.pb.h"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace mq {
    class Router {
    public:
        static bool isLegalRoutingKey(const std::string& routing_key) {
            // 判断当前的routingkey是否合法
            for (auto& ch : routing_key) {
                if ((ch >= 'a' && ch <= 'z') ||
                    (ch >= 'A' && ch <= 'Z') ||
                    (ch >= '0' && ch <= '9') ||
                    (ch == '_' || ch == '.') )
                    continue;
                return false;
            }
            return true;
        }

        static bool isLegalBindingKey(const std::string& binding_key) {
            // 1. 先判断是否存在非法字符
            for (auto& ch : binding_key) {
                if ((ch >= 'a' && ch <= 'z') ||
                    (ch >= 'A' && ch <= 'Z') ||
                    (ch >= '0' && ch <= '9') ||
                    (ch == '_' || ch == '.') ||
                    (ch == '*' || ch == '#'))
                    continue;
                return false;
            }

            // 2. 判断* #是否和其他字符相连接
            std::vector<std::string> sub_words;
            StrHelper::split(binding_key, ".", sub_words);
            for (auto& word : sub_words) {
                if (word.size() > 1 && (word.find('#') != std::string::npos || word.find('*') != std::string::npos))
                    return false;
            }

            // 3. 判断* #是否连接一起
            for (int i = 1; i < sub_words.size(); i++) {
                if (sub_words[i] == "#" && sub_words[i - 1] == "*")
                    return false;
                if (sub_words[i] == "#" && sub_words[i - 1] == "#")
                    return false;
                if (sub_words[i] == "*" && sub_words[i - 1] == "#")
                    return false;                
            }
            return true;
        }

        static bool route(ExchangeType type, const std::string& routing_key, const std::string& binding_key) {
            if (type == ExchangeType::DIRECT)
                return routing_key == binding_key;
            else if (type == ExchangeType::FANOUT)
                return true;
            // 使用动态规划来判断当前是否匹配
            std::vector<std::string> bkeys, rkeys;
            int n_bkey = StrHelper::split(binding_key, ".", bkeys);
            int n_rkey = StrHelper::split(routing_key, ".", rkeys);
            std::vector<std::vector<bool>> dp(n_bkey + 1, std::vector<bool>(n_rkey + 1, false));
            dp[0][0] = true;
            if (n_bkey > 0 && bkeys[0] == "#")
                dp[1][0] = true;
            for (int i = 1; i <= n_bkey; i++)
                for (int j = 1; j <= n_rkey; j++) {
                    if (bkeys[i - 1] == "*" || rkeys[j - 1] == bkeys[i - 1])
                        dp[i][j] = dp[i - 1][j - 1];
                    else if (bkeys[i - 1] == "#")
                        dp[i][j] = dp[i - 1][j - 1] | dp[i][j - 1] | dp[i - 1][j];
                }
            return dp[n_bkey][n_rkey];
        }

    };

}


#endif