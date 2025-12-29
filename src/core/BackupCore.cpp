#include "BackupCore.h"
#include <fstream>
#include <iostream>
#include "spdlog/spdlog.h"

using namespace std;
namespace fs = std::filesystem;

// 遍历目录树，筛选符合规则的文件
void BackupCore::traverseDir(const fs::path& srcDir, vector<fs::path>& fileList, const FilterRule& rule) {
    if (!fs::exists(srcDir) || !fs::is_directory(srcDir)) return;

    for (const auto& entry : fs::directory_iterator(srcDir)) {
        // 应用筛选规则（路径、类型、大小、时间等）
        if (rule.match(entry.path())) {
            fileList.push_back(entry.path());
        }
    }
}

// 备份核心逻辑
bool BackupCore::backup(const BackupConfig& config) {
    try {
        // 1. 筛选文件（自定义备份功能）
        vector<fs::path> fileList;
        traverseDir(config.srcPath, fileList, config.filterRule);
        if (fileList.empty()) {
            spdlog::warn("没有符合筛选规则的文件，备份终止！");
            return false;
        }
        for_each(fileList.begin(), fileList.end(), [](const fs::path& p) {
           spdlog::info("Selected file: {}", p.string());
        });
        // 2. 打包（tar/MyPack）
        string packFile = config.destPath + "/backup.pack";
        if (!PackUnpack::pack(fileList, packFile, config.packAlg)) {
            spdlog::error("打包失败！");
            return false;
        }

        // 3. 压缩（LZ77/Haff）
        string compressFile = packFile + "." + config.compressAlg;
        if (!Compress::compress(packFile, compressFile, config.compressAlg)) {
            spdlog::error("压缩失败！");
            return false;
        } 
        
        // 4. 加密（AES/DES）
        string encryptFile = compressFile + "." + config.cryptoAlg;
        if (!Crypto::encrypt(compressFile, encryptFile, config.cryptoAlg, config.password)) {
            spdlog::error("加密失败！");
            return false;
        }

        // 删除中间文件（打包文件、压缩文件）
        // fs::remove(packFile);
        // fs::remove(compressFile);

        return true;
    } catch (const fs::filesystem_error& e) {
        spdlog::error("备份异常：{}", e.what());
        return false;
    }
}

// 还原核心逻辑
bool BackupCore::restore(const string& backupFile, const string& restorePath, 
                         const string& cryptoAlg, const string& password) {
    try {
        // 1. 解密
        string decryptFile = backupFile + ".decrypt";
        if (!Crypto::decrypt(backupFile, decryptFile, cryptoAlg, password)) {
            spdlog::error("解密失败（密码错误或算法不匹配）！");
            return false;
        }

        // 2. 解压
        string decompressFile = decryptFile + ".decompress";

        size_t lastDot = backupFile.rfind('.');
        // 找到倒数第二个点号的位置
        size_t secondLastDot = backupFile.rfind('.', lastDot - 1);
        string compressAlg = backupFile.substr(secondLastDot + 1, lastDot - secondLastDot - 1);

        spdlog::info("Detected compress algorithm: {}", compressAlg);
        if (!Compress::decompress(decryptFile, decompressFile, compressAlg)) {
            spdlog::error("解压失败！");
            return false;
        }
        // 3. 解包
        string packAlg = "tar"; // 默认使用tar打包算法
        if (!PackUnpack::unpack(decompressFile, restorePath, packAlg)) {
            spdlog::error("解包失败！");
            return false;
        }

        // 删除中间文件
        // fs::remove(decryptFile);
        // fs::remove(decompressFile);
        spdlog::info("还原路径：{}", restorePath);
        return true;
    } catch (const fs::filesystem_error& e) {
        spdlog::error("还原异常：{}", e.what());
        return false;
    }
}