#pragma once
#include <string>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

struct FilterRule {
    // 包含规则
    std::vector<std::string> includeTypes{"all"};  // 文件类型（普通文件/目录/链接等）
    uint64_t minSize = 0;         // 最小大小（字节）
    uint64_t maxSize = UINT64_MAX;// 最大大小（字节）
    time_t minCreateTime = 0;     // 最小创建时间
    time_t maxCreateTime = INT64_MAX;// 最大创建时间
    time_t minModifyTime = 0;     // 最小修改时间
    time_t maxModifyTime = INT64_MAX;// 最大修改时间

    // 排除规则
    std::vector<std::string> excludePaths;  // 排除目录
    std::vector<std::string> excludeUsers;  // 排除用户
    std::vector<std::string> excludeGroups; // 排除用户组
    std::vector<std::string> excludeNames;  // 排除文件名（支持正则）

    // 匹配文件是否符合规则
    bool match(const fs::path& filePath) const;
};
