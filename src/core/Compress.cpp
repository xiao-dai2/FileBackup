#include "Compress.h"
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>

using namespace std;

// LZ77压缩（滑动窗口算法）
bool lz77Compress(const string& srcFile, const string& destFile) {
    ifstream in(srcFile, ios::binary);
    ofstream out(destFile, ios::binary);
    if (!in.is_open() || !out.is_open()) return false;

    const int WINDOW_SIZE = 4096;  // 滑动窗口大小
    const int LOOKAHEAD_SIZE = 15; // 前瞻缓冲区大小
    vector<char> window(WINDOW_SIZE, 0);
    vector<char> lookahead(LOOKAHEAD_SIZE, 0);
    int windowPos = 0;
    int tp = LOOKAHEAD_SIZE;

    while (!in.eof()) {
        // 读取前瞻缓冲区
        in.read(lookahead.data()+LOOKAHEAD_SIZE-tp, tp); //TODO 修复读取逻辑 
        int lookaheadLen = in.gcount();
        if (lookaheadLen == 0) break;

        // 查找最长匹配
        int offset = 0, length = 0;
       
        for (int j = 0; j < WINDOW_SIZE; j++) {
            int k = 0;
            while (k < lookaheadLen && window[(windowPos + j + k) % WINDOW_SIZE] == lookahead[k]) {
                k++;
            }
            if (k > length) {
                length = k;
                offset = j;
            }
        }

        // 写入压缩数据（offset, length, nextChar）
        out.write((char*)&offset, 2);
        out.write((char*)&length, 1);
        if (length < lookaheadLen) {
            out.write(&lookahead[length], 1);
        } else {
            // 需要写入一个标记或特殊处理
            // 比如写入一个0字符表示没有nextChar
            char nullChar = 0;
            out.write(&nullChar, 1);
        }
        tp = min(length + 1,lookaheadLen);
        // 更新滑动窗口
        for (int i = 0; i < tp; i++) {
            window[windowPos] = lookahead[i];
            windowPos = (windowPos + 1) % WINDOW_SIZE;
            if(i+tp<lookaheadLen)
                lookahead[i] = lookahead[i + tp];
        }
    }

    in.close();
    out.close();
    return true;
}

// LZ77解压
bool lz77Decompress(const string& compressFile, const string& destFile) {
    ifstream in(compressFile, ios::binary);
    ofstream out(destFile, ios::binary);
    if (!in.is_open() || !out.is_open()) return false;

    const int WINDOW_SIZE = 4096;
    vector<char> window(WINDOW_SIZE, 0);
    int windowPos = 0;

    while (!in.eof()) {
        // 读取offset和length
        uint16_t offset = 0;
        uint8_t length = 0;
        char nextChar = 0;
        
        in.read((char*)&offset, 2);
        in.read((char*)&length, 1);
        in.read(&nextChar, 1);

        if (in.gcount() < 4) break;  // 没有完整的数据块
        
        //TODO  复制匹配数据
        for (int i = 0; i < length; i++) {
            char c = window[(windowPos - offset + i + WINDOW_SIZE) % WINDOW_SIZE];
            out.write(&c, 1);
            window[windowPos] = c;
            windowPos = (windowPos + 1) % WINDOW_SIZE;
        }
  
        // 如果读取成功（不是EOF），写入该字符
        if (in.gcount() == 1) {
            out.write(&nextChar, 1);
            window[windowPos] = nextChar;
            windowPos = (windowPos + 1) % WINDOW_SIZE;
        }
    }

    in.close();
    out.close();
    return true;
}

// 哈夫曼树节点
struct HuffmanNode {
    char data;
    uint32_t freq;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(char d, uint32_t f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

// 哈夫曼压缩
bool haffCompress(const string& srcFile, const string& destFile) {
    // 1. 统计字符频率
    ifstream in(srcFile, ios::binary);
    if (!in.is_open()) return false;
    unordered_map<char, uint32_t> freqMap;
    char c;
    while (in.get(c)) {
        freqMap[c]++;
    }
    in.close();

    // 2. 构建哈夫曼树
    auto cmp = [](HuffmanNode* a, HuffmanNode* b) { return a->freq > b->freq; };
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, decltype(cmp)> pq(cmp);
    for (auto& [ch, freq] : freqMap) {
        pq.push(new HuffmanNode(ch, freq));
    }

    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto parent = new HuffmanNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    if (pq.empty()) return false;
    HuffmanNode* root = pq.top();

    // 3. 生成哈夫曼编码
    unordered_map<char, string> codeMap;
    function<void(HuffmanNode*, string)> dfs = [&](HuffmanNode* node, string code) {
        if (!node) return;
        if (!node->left && !node->right) {
            codeMap[node->data] = code;
            return;
        }
        dfs(node->left, code + "0");
        dfs(node->right, code + "1");
    };
    dfs(root, "");

    // 4. 写入压缩文件（频率表+编码数据）
    ofstream out(destFile, ios::binary);
    uint32_t freqCount = freqMap.size();
    out.write((char*)&freqCount, sizeof(freqCount));
    for (auto& [ch, freq] : freqMap) {
        out.write(&ch, 1);
        out.write((char*)&freq, sizeof(freq));
    }

    // 5. 编码数据并写入（按位打包）
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
    }

    in.close();
    out.close();
    // 释放哈夫曼树（省略）
    return true;
}

// 哈夫曼解压
bool haffDecompress(const string& compressFile, const string& destFile) {
    ifstream in(compressFile, ios::binary);
    ofstream out(destFile, ios::binary);
    if (!in.is_open() || !out.is_open()) return false;

    // 1. 读取频率表
    uint32_t freqCount = 0;
    in.read((char*)&freqCount, sizeof(freqCount));
    unordered_map<char, uint32_t> freqMap;
    for (uint32_t i = 0; i < freqCount; i++) {
        char ch;
        uint32_t freq;
        in.read(&ch, 1);
        in.read((char*)&freq, sizeof(freq));
        freqMap[ch] = freq;
    }

    // 2. 重建哈夫曼树（同压缩逻辑，省略）
    HuffmanNode* root = nullptr; // 重建逻辑省略，与压缩时一致

    // 3. 解码数据
    HuffmanNode* curr = root;
    char buf;
    while (in.read(&buf, 1)) {
        for (int i = 7; i >= 0; i--) {
            bool bit = (buf >> i) & 1;
            if (bit) {
                curr = curr->right;
            } else {
                curr = curr->left;
            }
            // 到达叶子节点，输出字符
            if (!curr->left && !curr->right) {
                out.write(&curr->data, 1);
                curr = root;
            }
        }
    }

    in.close();
    out.close();
    // 释放哈夫曼树（省略）
    return true;
}

// 压缩入口
bool Compress::compress(const string& srcFile, const string& destFile, const string& alg) {
    if (alg == "LZ77") {
        return lz77Compress(srcFile, destFile);
    } else if (alg == "Haff") {
        return haffCompress(srcFile, destFile);
    } else {
        return false;
    }
}

// 解压入口
bool Compress::decompress(const string& compressFile, const string& destFile, const string& alg) {
    if (alg == "LZ77") {
        return lz77Decompress(compressFile, destFile);
    } else if (alg == "Haff") {
        return haffDecompress(compressFile, destFile);
    } else {
        return false;
    }
}