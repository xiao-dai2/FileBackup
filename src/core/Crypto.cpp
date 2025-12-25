// Crypto.cpp
#include "Crypto.h"
#include <fstream>
#include <iostream>
#include <crypto++/aes.h>
#include <crypto++/des.h>
#include <crypto++/modes.h>
#include <crypto++/filters.h>
#include <crypto++/hex.h>
#include <crypto++/files.h> // 必须包含，提供FileSource/FileSink支持

using namespace std;
using namespace CryptoPP;

// AES加密（ECB模式，修复类型转换+流操作）
bool aesEncrypt(const string& srcFile, const string& destFile, const string& password) {
    try {
        // 密码填充到16字节（AES-128密钥长度）
        string key(16, 0);
        memcpy(&key[0], password.c_str(), min(password.size(), (size_t)16));

        // 显式类型转换：char* -> const unsigned char*（const unsigned char*）
        const unsigned char* aesKey = reinterpret_cast<const unsigned char*>(key.data());
        AES::Encryption aesEnc(aesKey, AES::DEFAULT_KEYLENGTH);
        ECB_Mode_ExternalCipher::Encryption ecbEnc(aesEnc);

        // CryptoPP原生文件流处理，无需C++标准fstream
        FileSource(
            srcFile.c_str(),  // 源文件路径
            true,             // 处理完成后自动关闭文件
            new StreamTransformationFilter( // 加密过滤器链
                ecbEnc,
                new FileSink(destFile.c_str()) // 加密后写入目标文件
            )
        );

        return true;
    } catch (const Exception& e) {
        cerr << "AES加密异常：" << e.what() << endl;
        return false;
    }
}

// AES解密（修复类型转换+流操作）
bool aesDecrypt(const string& encryptFile, const string& destFile, const string& password) {
    try {
        string key(16, 0);
        memcpy(&key[0], password.c_str(), min(password.size(), (size_t)16));

        // 显式类型转换：char* -> const unsigned char*
        const unsigned char* aesKey = reinterpret_cast<const unsigned char*>(key.data());
        AES::Decryption aesDec(aesKey, AES::DEFAULT_KEYLENGTH);
        ECB_Mode_ExternalCipher::Decryption ecbDec(aesDec);

        FileSource(
            encryptFile.c_str(),
            true,
            new StreamTransformationFilter(
                ecbDec,
                new FileSink(destFile.c_str())
            )
        );

        return true;
    } catch (const Exception& e) {
        cerr << "AES解密异常：" << e.what() << endl;
        return false;
    }
}

// DES加密
bool desEncrypt(const string& srcFile, const string& destFile, const string& password) {
    try {
        // 密码填充到8字节（DES密钥长度）
        string key(8, 0);
        memcpy(&key[0], password.c_str(), min(password.size(), (size_t)8));

        // 显式类型转换：char* -> const unsigned char*
        const unsigned char* desKey = reinterpret_cast<const unsigned char*>(key.data());
        DES::Encryption desEnc(desKey, DES::DEFAULT_KEYLENGTH);
        ECB_Mode_ExternalCipher::Encryption ecbEnc(desEnc);

        FileSource(
            srcFile.c_str(),
            true,
            new StreamTransformationFilter(
                ecbEnc,
                new FileSink(destFile.c_str())
            )
        );

        return true;
    } catch (const Exception& e) {
        cerr << "DES加密异常：" << e.what() << endl;
        return false;
    }
}

// DES解密（修复类型转换，解决核心编译错误）
bool desDecrypt(const string& encryptFile, const string& destFile, const string& password) {
    try {
        string key(8, 0);
        memcpy(&key[0], password.c_str(), min(password.size(), (size_t)8));

        // 显式类型转换：char* -> const unsigned char*（关键修复点）
        const unsigned char* desKey = reinterpret_cast<const unsigned char*>(key.data());
        DES::Decryption desDec(desKey, DES::DEFAULT_KEYLENGTH); // 现在类型匹配
        ECB_Mode_ExternalCipher::Decryption ecbDec(desDec);

        FileSource(
            encryptFile.c_str(),
            true,
            new StreamTransformationFilter(
                ecbDec,
                new FileSink(destFile.c_str())
            )
        );

        return true;
    } catch (const Exception& e) {
        cerr << "DES解密异常：" << e.what() << endl;
        return false;
    }
}

// 加密入口
bool Crypto::encrypt(const string& srcFile, const string& destFile, const string& alg, const string& password) {
    if (alg == "AES") {
        return aesEncrypt(srcFile, destFile, password);
    } else if (alg == "DES") {
        return desEncrypt(srcFile, destFile, password);
    } else {
        cerr << "不支持的加密算法：" << alg << endl;
        return false;
    }
}

// 解密入口
bool Crypto::decrypt(const string& encryptFile, const string& destFile, const string& alg, const string& password) {
    if (alg == "AES") {
        return aesDecrypt(encryptFile, destFile, password);
    } else if (alg == "DES") {
        return desDecrypt(encryptFile, destFile, password);
    } else {
        cerr << "不支持的解密算法：" << alg << endl;
        return false;
    }
}