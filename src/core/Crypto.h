#pragma once
#include <string>

class Crypto {
public:
    // 加密：srcFile-源文件，destFile-加密文件，alg-算法（AES/DES），password-密码
    static bool encrypt(const std::string& srcFile, const std::string& destFile, 
                        const std::string& alg, const std::string& password);

    // 解密：encryptFile-加密文件，destFile-解密文件，alg-算法（AES/DES），password-密码
    static bool decrypt(const std::string& encryptFile, const std::string& destFile, 
                        const std::string& alg, const std::string& password);
};
