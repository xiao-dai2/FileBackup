# 基础镜像：选择支持GTKmm 3.0的Ubuntu 22.04（LTS版本，依赖更稳定）
FROM ubuntu:22.04


# 设置环境变量：避免交互性提示，指定UTF-8编码，打通X11显示
ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    QT_X11_NO_MITSHM=1 \
    XDG_RUNTIME_DIR=/tmp/runtime-root

# 安装依赖：编译+运行GTKmm 3.0所需的所有库和工具
RUN apt-get update && apt-get install -y --no-install-recommends \
    # 基础编译工具
    build-essential \
    cmake \
    make \
    g++ \
    # GTKmm 3.0 核心依赖
    libgtkmm-3.0-dev \
    # sigc++ 依赖（GTKmm信号槽）
    libsigc++-2.0-dev \
    # 线程池/线程相关依赖
    libglibmm-2.4-dev \
    # X11显示依赖（容器内运行GUI必需）
    x11-apps \
    libx11-dev \
    libxext-dev \
    libxcb1-dev \
    libxrender-dev \
    libxi-dev \
    # 清理缓存，减小镜像体积
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /app

# 复制项目源码到容器（假设本地源码根目录与Dockerfile同级别）
# 注意：根据你的实际源码结构调整复制路径
COPY . /app

# 创建编译目录并编译项目
RUN mkdir -p /app/build && cd /app/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

# 创建运行时目录（存放备份数据/配置，可选）
RUN mkdir -p /app/backup_data /app/icons && chmod 777 /app/backup_data /tmp/runtime-root

# 暴露端口（GUI程序无需暴露端口，仅为标识）
EXPOSE 8080

# 设置容器启动命令：运行编译后的备份工具
CMD ["/app/build/BackupTool"]