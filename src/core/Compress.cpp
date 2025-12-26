#include "Compress.h"
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <map>

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


class HuffmanComress{
    // 哈夫曼树节点
    struct HuffmanNode {
        char data;
        uint32_t freq;
        HuffmanNode* left;
        HuffmanNode* right;
        HuffmanNode(char d, uint32_t f) : data(d), freq(f), left(nullptr), right(nullptr) {}
    };
    char getMinChar(const HuffmanNode* node) {
        if (!node) return 127;  // 最大char值
        
        if (!node->left && !node->right) {
            return node->data;  // 叶子节点返回字符
        }
        
        // 非叶子节点返回左右子树的最小字符
        char leftMin = getMinChar(node->left);
        char rightMin = getMinChar(node->right);
        
        return min(leftMin, rightMin);
    }

    HuffmanNode *buildHuffmanTree(const map<char,uint32_t> &freqMap) {
        // 3. 构建哈夫曼树（使用稳定优先队列）
        // 自定义优先队列比较器，确保稳定性
        auto nodeCompare = [this](const HuffmanNode* a, const HuffmanNode* b) {
            if (a->freq != b->freq) {
                return a->freq > b->freq;  // 最小堆
            }
            // 频率相同时，比较节点中最小字符（确保稳定性）
            // 需要辅助函数获取节点中的最小字符
            return getMinChar(a) > getMinChar(b);
        };
        
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, 
                    decltype(nodeCompare)> pq(nodeCompare);
        
        // 创建叶子节点并加入优先队列
        for (const auto& p : freqMap) {
            pq.push(new HuffmanNode(p.first, p.second));
        }
        
        // 构建哈夫曼树
        while (pq.size() > 1) {
            HuffmanNode* left = pq.top(); pq.pop();
            HuffmanNode* right = pq.top(); pq.pop();
            
            HuffmanNode* parent = new HuffmanNode('\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            
            pq.push(parent);
        }
        
        return pq.empty() ? nullptr : pq.top();
    }

    void freeHuffmanTree(HuffmanNode* root) {
        if (!root) return;
        freeHuffmanTree(root->left);
        freeHuffmanTree(root->right);
        delete root;
    }

public:
    bool Compress(const string& srcFile, const string& destFile) {
        // 1. 统计字符频率（使用map，自动按键排序）
        ifstream in(srcFile, ios::binary);
        if (!in.is_open()) return false;
        
        map<char, uint32_t> freqMap;  // map会按键自动排序
        char c;
        
        while (in.get(c)) {
            freqMap[c]++;
        }
        in.close();
        
        HuffmanNode* root = buildHuffmanTree(freqMap);
        
        // 4. 生成哈夫曼编码（使用稳定遍历顺序）
        unordered_map<char, string> codeMap;
        
        function<void(HuffmanNode*, string)> generateCodes = 
            [&](HuffmanNode* node, string code) {
            if (!node) return;
            if (!node->left && !node->right) {
                codeMap[node->data] = code;
                return;
            }
            // 确保遍历顺序：总是先左后右
            generateCodes(node->left, code + "0");
            generateCodes(node->right, code + "1");
        };
        
        generateCodes(root, "");
        
        // 5. 写入压缩文件（保持写入顺序稳定）
        ofstream out(destFile, ios::binary);
        
        // 先写入字符数量
        uint32_t charCount = freqMap.size();
        out.write((char*)&charCount, sizeof(charCount));
        
        // 按排序后的顺序写入字符和频率
        for (auto [ch,freq] : freqMap) {
            out.write(&ch, 1);
            out.write((char*)&freq, sizeof(freq));
        }
        
        // 6. 编码数据并写入
        in.open(srcFile, ios::binary);
        char buf = 0;
        int bitPos = 0;
        
        while (in.get(c)) {
            string code = codeMap[c];
            for (char bit : code) {
                buf = (buf << 1) | (bit - '0');
                bitPos++;
                if (bitPos == 8) {
                    out.write(&buf, 1);
                    buf = 0;
                    bitPos = 0;
                }
            }
        }
        
        // 处理剩余位
        if (bitPos > 0) {
            buf <<= (8 - bitPos);
            out.write(&buf, 1);
            // 写入最后一个字节的有效位数
            out.write((char*)&bitPos, 1);
        } else {
            // 如果刚好是8的倍数，写入0表示没有剩余位
            char zero = 8;
            out.write(&zero, 1);
        }
        
        in.close();
        out.close();
        
        // 释放哈夫曼树
        freeHuffmanTree(root);
        return true;
    }
    // 哈夫曼解压
    bool Decompress(const string& compressFile, const string& destFile) {
        ifstream in(compressFile, ios::binary);
        ofstream out(destFile, ios::binary);
        if (!in.is_open() || !out.is_open()) return false;
        
        // 1. 读取频率表
        uint32_t charCount = 0;
        in.read((char*)&charCount, sizeof(charCount));
        
        map<char, uint32_t> freqMap;  // 使用map保持顺序
        for (uint32_t i = 0; i < charCount; i++) {
            char ch;
            uint32_t freq;
            in.read(&ch, 1);
            in.read((char*)&freq, sizeof(freq));
            freqMap[ch] = freq;
        }
        
        HuffmanNode* root = buildHuffmanTree(freqMap);
        
        // 3. 解码数据
        HuffmanNode* curr = root;
        char buf;
        
        // 读取最后一个字节的填充位数
        in.seekg(-1, ios::end);
        char paddingBits;
        in.read(&paddingBits, 1);
        in.seekg(sizeof(uint32_t) + charCount * (1 + sizeof(uint32_t)));
        
        streampos dataStart = in.tellg();
        streampos dataEnd = in.tellg();
        in.seekg(0, ios::end);
        streampos fileEnd = in.tellg();
        
        // 计算数据字节数
        long long dataBytes = fileEnd - dataEnd - 1;
        in.seekg(dataStart);
        
        for (long long i = 0; i < dataBytes; i++) {
            in.read(&buf, 1);
            int bitsToProcess = (i == dataBytes - 1) ? paddingBits : 8;
            
            for (int j = 7; j >= (8 - bitsToProcess); j--) {
                bool bit = (buf >> j) & 1;
                curr = bit ? curr->right : curr->left;
                
                if (!curr->left && !curr->right) {
                    out.write(&curr->data, 1);
                    curr = root;
                }
            }
        }
        
        in.close();
        out.close();
        freeHuffmanTree(root);
        return true;
    }

};
// 压缩入口
bool Compress::compress(const string& srcFile, const string& destFile, const string& alg) {
    if (alg == "LZ77") {
        LZ77Compress lz77;
        return lz77.Compress(srcFile, destFile);
    } else if (alg == "Haff") {
        HuffmanComress huffCompressor;
        return huffCompressor.Compress(srcFile, destFile);
    } else {
        return false;
    }
}

// 解压入口
bool Compress::decompress(const string& compressFile, const string& destFile, const string& alg) {
    if (alg == "LZ77") {
        LZ77Compress lz77;
        return lz77.Decompress(compressFile, destFile);
    } else if (alg == "Haff") {
        HuffmanComress huffCompressor;
        return huffCompressor.Decompress(compressFile, destFile);
    } else {
        return false;
    }
}