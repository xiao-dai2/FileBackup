#include "PackUnpack.h"
#include <filesystem>
#include <libtar.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace fs = std::filesystem;
// 匹配头文件的命名空间
namespace PackUnpack {

// 原tarPack函数保持不变（内部辅助函数）
bool tarPack(const std::vector<fs::path>& srcPaths, const std::string& destFile) {
    TAR* tar = nullptr;

    if (tar_open(&tar, destFile.c_str(), NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU) != 0) {
        std::cerr << "无法创建tar归档文件：" << destFile << std::endl;
        return false;
    }

    // 遍历所有待打包路径
    for (const auto& path : srcPaths) {
        if (!fs::exists(path)) {
            std::cerr << "路径不存在：" << path << std::endl;
            tar_close(tar);
            return false;
        }

        // 显式转换const char* to char*
        char* realdir = const_cast<char*>(path.c_str());
        char* savedir = const_cast<char*>(path.filename().c_str());
        if (tar_append_tree(tar, realdir, savedir) != 0) {
            std::cerr << "打包失败：" << path << std::endl;
            tar_close(tar);
            return false;
        }
    }

    // 关闭归档文件
    if (tar_close(tar) != 0) {
        std::cerr << "关闭tar归档文件失败" << std::endl;
        return false;
    }

    std::cout << "tar打包成功：" << destFile << std::endl;
    return true;
}
// 将 packFile 解包到 destDir 目录
bool tarUnpack(const std::string& packFile, const std::string& destDir) {

    if(!fs::exists(destDir)) {
        if(!fs::create_directories(destDir)){
            std::cerr << "创建目标目录失败：" << destDir << std::endl;
            return false;
        }
    }
    std::string command="tar -xf \"" + packFile + "\" -C \"" + destDir + "\" --overwrite";
    
    std::cout << "执行命令: " << command << std::endl;
    
    int result = system(command.c_str());
    
    if (result == 0) {
        std::cout << "tar解包成功" << std::endl;
        return true;
    } else {
        std::cerr << "tar命令执行失败，返回码: " << result << std::endl;
        return false;
    }
}

bool pack(const std::vector<fs::path>& srcPaths, const std::string& destFile, const std::string& packType) {
    if (packType == "tar") {
        return tarPack(srcPaths, destFile);
    } else {
        std::cerr << "不支持的打包格式：" << packType << std::endl;
        return false;
    }
}

bool unpack(const std::string& packFile, const std::string& destDir, const std::string& packType) {
    if (packType == "tar") {
        return tarUnpack(packFile, destDir);
    } else {
        std::cerr << "不支持的解压格式：" << packType << std::endl;
        return false;
    }
}

} // 结束PackUnpack命名空间