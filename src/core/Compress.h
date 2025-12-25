#pragma once
#include <string>

class Compress {
public:
    // 压缩：srcFile-源文件，destFile-压缩文件，alg-算法（LZ77/Haff）
    static bool compress(const std::string& srcFile, const std::string& destFile, const std::string& alg);

    // 解压：compressFile-压缩文件，destFile-解压文件，alg-算法（LZ77/Haff）
    static bool decompress(const std::string& compressFile, const std::string& destFile, const std::string& alg);
};