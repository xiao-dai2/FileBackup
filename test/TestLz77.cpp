#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

class LZ77Compress{
private:
    static const int WINDOW_SIZE = 4096;      // 滑动窗口大小
    static const int LOOKAHEAD_SIZE = 18;     // 前向缓冲区大小
    // LZ77编码结构体：(偏移量, 匹配长度, 下一个字符)
    struct LZ77Token {
        int offset;      // 偏移量
        int length;      // 匹配长度
        char nextChar;   // 下一个字符
        
        LZ77Token(int o = 0, int l = 0, char c = '\0') 
            : offset(o), length(l), nextChar(c) {}
    };
    
    // 在滑动窗口中查找最长匹配
    LZ77Token findLongestMatch(const string& data, int pos) {
        int windowStart = max(0, pos - WINDOW_SIZE);
        int matchOffset = 0;
        int matchLength = 0;
        
        // 在滑动窗口中搜索最长匹配
        for (int i = windowStart; i < pos; i++) {
            int length = 0;
            // 计算从位置i开始的匹配长度
            while (length < LOOKAHEAD_SIZE && 
                   pos + length < data.length() && 
                   data[i + length] == data[pos + length]) {
                length++;
            }
            
            // 更新最长匹配
            if (length > matchLength) {
                matchLength = length;
                matchOffset = pos - i;
            }
        }
        
        // 获取下一个字符
        char nextChar = (pos + matchLength < data.length()) 
                        ? data[pos + matchLength] : '\0';
        
        return LZ77Token(matchOffset, matchLength, nextChar);
    }

public:
    // 压缩函数
    bool Compress(const string& srcFile, const string& destFile) {
        ifstream in(srcFile, ios::binary);
        ofstream out(destFile, ios::binary);
        if (!in.is_open() || !out.is_open()) return false;

        string input((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        int pos = 0;
        
        while (pos < input.length()) {
            LZ77Token token = findLongestMatch(input, pos);
            // 写入压缩数据：offset(2字节), length(1字节), nextChar(1字节)
            out.write((char*)&token.offset, 2);
            out.write((char*)&token.length, 2);
            out.write(&token.nextChar, 1);
            // 移动位置：匹配长度 + 1（下一个字符）
            pos += token.length + 1;
        }
        in.close();
        out.close();
        return true;
    }
    
    // 解压函数
    bool Decompress(const string& compressFile, const string& destFile) {
        ifstream in(compressFile, ios::binary);
        ofstream out(destFile, ios::binary);
        if (!in.is_open() || !out.is_open()) return false;
        string output;
        while(in.peek() != EOF) {
            LZ77Token token;
            // 读取压缩数据：offset(2字节), length(1字节), nextChar(1字节)
            in.read((char*)&token.offset, 2);
            in.read((char*)&token.length, 2);
            in.read(&token.nextChar, 1);

            // 从之前的位置复制匹配的字符串
            if (token.length > 0 && token.offset > 0) {
                int startPos = output.length() - token.offset;
                for (int i = 0; i < token.length; i++) {
                    output += output[startPos + i];
                }
            }
            // 添加下一个字符
            if (token.nextChar != '\0') {
                output += token.nextChar;
            }
        }
        out.write(output.c_str(), output.size());
        in.close();
        out.close();
        return true;
    }
    
};


int main() {
    LZ77Compress lz77;
    
    // 测试示例1：简单重复字符串
    cout << "========== 测试1：重复字符串 ==========" << endl;
    string srcfile = "./data/file.txt";
    string destfile = "./data/file.lz77";
    string decompressedfile = "./data/file_decompressed.txt";
    
    lz77.Compress(srcfile,destfile);
    lz77.Decompress(destfile,decompressedfile);

    string cmd = "diff " + srcfile + " " + decompressedfile;
    int result = system(cmd.c_str());
    if (result == 0) {
        cout << "测试1通过：解压文件与原文件一致" << endl;
    } else {
        cout << "测试1失败：解压文件与原文件不一致" << endl;
    }
    return 0;
}