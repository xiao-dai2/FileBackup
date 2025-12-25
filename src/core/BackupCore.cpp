#include "BackupCore.h"
#include <fstream>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

// 遍历目录树，筛选符合规则的文件
void BackupCore::traverseDir(const fs::path& srcDir, vector<fs::path>& fileList, const FilterRule& rule) {
    if (!fs::exists(srcDir) || !fs::is_directory(srcDir)) return;

    for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
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
            cout << "无符合条件的文件待备份！" << endl;
            return false;
        }
        for_each(fileList.begin(), fileList.end(), [](const fs::path& p) {
            cout << "待备份文件：" << p << endl;
        });
        // 2. 打包（tar/MyPack）
        string packFile = config.destPath + "/backup.pack";
        if (!PackUnpack::pack(fileList, packFile, config.packAlg)) {
            cout << "打包失败！" << endl;
            return false;
        }

        // 3. 压缩（LZ77/Haff）
        string compressFile = packFile + "." + config.compressAlg;
        if (!Compress::compress(packFile, compressFile, config.compressAlg)) {
            cout << "压缩失败！" << endl;
            return false;
        }
        cout << "压缩成功：" << compressFile << endl;

        // 4. 加密（AES/DES）
        string encryptFile = compressFile + "." + config.cryptoAlg;
        if (!Crypto::encrypt(compressFile, encryptFile, config.cryptoAlg, config.password)) {
            cout << "加密失败！" << endl;
            return false;
        }

        // 删除中间文件（打包文件、压缩文件）
        // fs::remove(packFile);
        // fs::remove(compressFile);

        cout << "备份成功！备份文件：" << encryptFile << endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        cout << "备份异常：" << e.what() << endl;
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
            cout << "解密失败（密码错误或算法不匹配）！" << endl;
            return false;
        }

        // 2. 解压
        string decompressFile = decryptFile + ".decompress";

        size_t lastDot = backupFile.rfind('.');
        // 找到倒数第二个点号的位置
        size_t secondLastDot = backupFile.rfind('.', lastDot - 1);
        string compressAlg = backupFile.substr(secondLastDot + 1, lastDot - secondLastDot - 1);

        cout << "Detected compress algorithm: " << compressAlg << endl;
        if (!Compress::decompress(decryptFile, decompressFile, compressAlg)) {
            cout << "解压失败！" << endl;
            return false;
        }
        // 3. 解包
        string packAlg = "tar"; // 默认使用tar打包算法
        if (!PackUnpack::unpack(decompressFile, restorePath, packAlg)) {
            cout << "解包失败！" << endl;
            return false;
        }

        // 删除中间文件
        // fs::remove(decryptFile);
        // fs::remove(decompressFile);

        cout << "还原成功！还原路径：" << restorePath << endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        cout << "还原异常：" << e.what() << endl;
        return false;
    }
}