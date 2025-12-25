// PackUnpack.h
#pragma once
#include <vector>
#include <string>
#include <filesystem>

// 若BackupCore中使用了PackUnpack命名空间，需添加该命名空间
namespace PackUnpack {
    // 统一函数名为pack（与BackupCore调用一致）
    bool pack(const std::vector<std::filesystem::path>& srcPaths, const std::string& destFile, const std::string& packType);

    // 统一函数名为unpack（与BackupCore调用一致）
    bool unpack(const std::string& packFile, const std::string& destDir, const std::string& packType);
}