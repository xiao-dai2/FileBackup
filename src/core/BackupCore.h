#pragma once
#include <string>
#include <filesystem>
#include "Filter.h"
#include "PackUnpack.h"
#include "Compress.h"
#include "Crypto.h"

namespace fs = std::filesystem;

class BackupCore {
public:
    // 备份配置结构体
    struct BackupConfig {
        std::string srcPath;       // 源路径
        std::string destPath;      // 目标路径
        FilterRule filterRule;     // 筛选规则
        std::string packAlg;       // 打包算法（tar/MyPack）
        std::string compressAlg;   // 压缩算法（LZ77/Haff）
        std::string cryptoAlg;     // 加密算法（AES/DES）
        std::string password;      // 加密密码
    };

    // 数据备份（含筛选、打包、压缩、加密）
    bool backup(const BackupConfig& config);

    // 数据还原（含解密、解压、解包）
    bool restore(const std::string& backupFile, const std::string& restorePath, 
                 const std::string& cryptoAlg, const std::string& password);

private:
    // 递归遍历目录树，收集待备份文件
    void traverseDir(const fs::path& srcDir, std::vector<fs::path>& fileList, const FilterRule& rule);
};