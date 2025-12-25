#include "Filter.h"
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

bool FilterRule::match(const fs::path& filePath) const {
    fs::file_status status = fs::status(filePath);

    // 1. 匹配文件类型
    bool typeMatch = false;
    for (const auto& type : includeTypes) {
        if (type == "all") { typeMatch = true; break; }
        if (type == "file" && fs::is_regular_file(status)) typeMatch = true;
        if (type == "dir" && fs::is_directory(status)) typeMatch = true;
        if (type == "symlink" && fs::is_symlink(status)) typeMatch = true;
        // 支持管道、设备文件等（扩展）
    }
    if (!typeMatch) return false;

    // 2. 匹配文件大小 (仅普通文件)
    if (fs::is_regular_file(status)){ 
        uint64_t fileSize = fs::file_size(filePath);
        if (fileSize < minSize || fileSize > maxSize) return false;
    }
    // // 3. 匹配时间（创建/修改）
    // auto createTime = fs::last_write_time(filePath); // 简化：用写入时间代替创建时间
    // auto modifyTime = fs::last_write_time(filePath);
    // if (createTime. < minCreateTime || createTime > maxCreateTime) return false;
    // if (modifyTime < minModifyTime || modifyTime > maxModifyTime) return false;

    // 4. 排除规则：路径
    for (const auto& path : excludePaths) {
        if (filePath.string().find(path) != std::string::npos) return false;
    }

    // 5. 排除规则：用户/用户组（Linux仅支持）
#ifdef __linux__
    struct stat st;
    stat(filePath.c_str(), &st);
    std::string user = getpwuid(st.st_uid)->pw_name;
    std::string group = getgrgid(st.st_gid)->gr_name;
    for (const auto& u : excludeUsers) {
        if (user == u) return false;
    }
    for (const auto& g : excludeGroups) {
        if (group == g) return false;
    }
#endif

    // 6. 排除规则：文件名（正则）
    for (const auto& name : excludeNames) {
        std::regex reg(name);
        if (regex_match(filePath.filename().string(), reg)) return false;
    }

    return true;
}