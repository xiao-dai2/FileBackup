// PackUnpack.cpp
#include "PackUnpack.h"
#include <filesystem>
#include <libtar.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

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

// 原tarUnpack函数保持不变（内部辅助函数）
bool tarUnpack(const std::string& packFile, const std::string& destDir) {
    TAR* tar = nullptr;

    if (tar_open(&tar, packFile.c_str(), NULL, O_RDONLY, 0, TAR_GNU) != 0) {
        std::cerr << "无法打开tar归档文件：" << packFile << std::endl;
        return false;
    }

    // 创建目标目录（若不存在）
    if (!fs::exists(destDir)) {
        if (!fs::create_directories(destDir)) {
            std::cerr << "无法创建目标目录：" << destDir << std::endl;
            tar_close(tar);
            return false;
        }
    }

    // 切换到目标目录（确保解压到正确路径）
    if (chdir(destDir.c_str()) != 0) {
        std::cerr << "无法切换到目标目录：" << destDir << std::endl;
        tar_close(tar);
        return false;
    }

    // 传递2个参数，显式定义char数组作为前缀
    char prefix[] = "";
    if (tar_extract_all(tar, prefix) != 0) {
        std::cerr << "解压tar包失败" << std::endl;
        tar_close(tar);
        return false;
    }

    // 关闭归档文件
    if (tar_close(tar) != 0) {
        std::cerr << "关闭tar归档文件失败" << std::endl;
        return false;
    }

    std::cout << "tar解包成功：" << packFile << " -> " << destDir << std::endl;
    return true;
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