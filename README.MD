目前lz77压缩实现有bug, 暂不支持

1.依赖安装
# 安装libtar（打包解包）
sudo apt-get install libtar-dev

# 安装Crypto++（加密解密）
sudo apt-get install libcrypto++-dev

# GTKmm (ui)
sudo apt-get install libgtkmm-3.0-dev

2.构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
