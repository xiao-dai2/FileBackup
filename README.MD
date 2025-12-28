目前lz77压缩实现有bug, 暂不支持

1.依赖安装
# 安装libtar（打包解包）
sudo apt-get install libtar-dev

# 安装Crypto++（加密解密）
sudo apt-get install libcrypto++-dev

# GTKmm (ui)
sudo apt-get install libgtkmm-3.0-dev
# 线程池/线程相关依赖
sudo apt-get install libglibmm-2.4-dev 

2.构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4

docker
# 授权容器访问宿主机 X Server
xhost +local:docker
# 构建镜像
docker build -t backup-tool:v1.0 .
# run
sudo docker run -it --rm \
    --net=host \
    --privileged \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $HOME/.Xauthority:/root/.Xauthority \
    backup-tool:v1.0

