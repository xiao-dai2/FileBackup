#include <iostream>
#include <gtkmm.h>
#include "ui/MainWindow.h"
#include "core/BackupCore.h"
#include <signal.h>
#include <filesystem>
#include "spdlog/spdlog.h"

bool isCliMode = false;
// 信号处理函数（优雅退出）
void signalHandler(int signum) {
    spdlog::info("收到停止信号，正在退出程序...");
    if(!isCliMode) Gtk::Main::quit(); // 退出GTK主循环
    exit(signum);
}

int main(int argc, char *argv[]) {
    // 1. 初始化信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 2. 判断运行模式（CLI / GTK图形界面）
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--cli" || std::string(argv[i]) == "-c") {
            isCliMode = true;
            break;
        }
    }

    if (isCliMode) {
        // 命令行模式（复用之前的逻辑，无需修改）
        std::cout << "===== 数据备份软件（命令行模式）=====" << std::endl;
        std::cout << "支持命令：" << std::endl;
        std::cout << "  backup --src 源路径 --dest 目标路径 [--pack 打包算法] [--compress 压缩算法] [--crypto 加密算法] [--password 密码]" << std::endl;
        std::cout << "  restore --file 备份文件 --dest 还原路径 [--crypto 加密算法] [--password 密码]" << std::endl;

        BackupCore core;
        BackupCore::BackupConfig config;

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "backup") {
                // 解析备份参数
                for (int j = i + 1; j < argc; j++) {
                    std::string key = argv[j];
                    if (key == "--src" && j + 1 < argc) config.srcPath = argv[++j];
                    else if (key == "--dest" && j + 1 < argc) config.destPath = argv[++j];
                    else if (key == "--pack" && j + 1 < argc) config.packAlg = argv[++j];
                    else if (key == "--compress" && j + 1 < argc) config.compressAlg = argv[++j];
                    else if (key == "--crypto" && j + 1 < argc) config.cryptoAlg = argv[++j];
                    else if (key == "--password" && j + 1 < argc) config.password = argv[++j];
                }

                if (config.srcPath.empty() || config.destPath.empty()) {
                    spdlog::error("错误：源路径和目标路径不能为空！");
                    return 1;
                }

                // 默认算法
                if (config.packAlg.empty()) config.packAlg = "tar";
                if (config.compressAlg.empty()) config.compressAlg = "LZ77";
                if (config.cryptoAlg.empty()) config.cryptoAlg = "AES";

                spdlog::info("开始备份...");
                if (core.backup(config)) {
                    spdlog::info("备份成功！");
                } else {
                    spdlog::error("备份失败！");
                    return 1;
                }
                break;
            } else if (arg == "restore") {
                // 解析还原参数
                std::string backupFile, restorePath, cryptoAlg = "AES", password;
                for (int j = i + 1; j < argc; j++) {
                    std::string key = argv[j];
                    if (key == "--file" && j + 1 < argc) backupFile = argv[++j];
                    else if (key == "--dest" && j + 1 < argc) restorePath = argv[++j];
                    else if (key == "--crypto" && j + 1 < argc) cryptoAlg = argv[++j];
                    else if (key == "--password" && j + 1 < argc) password = argv[++j];
                }

                if (backupFile.empty() || restorePath.empty()) {
                    spdlog::error("错误：备份文件和还原路径不能为空！");
                    return 1;
                }

                spdlog::info("开始还原...");
                if (core.restore(backupFile, restorePath, cryptoAlg, password)) {
                    spdlog::info("还原成功！");
                } else {
                    spdlog::error("还原失败！");
                    return 1;
                }
                break;
            }
        }
    } else {
        // GTK图形界面模式
        Gtk::Main kit(argc, argv);
        MainWindow window;
        // 启动GTK主循环
        Gtk::Main::run(window);
    }

    return 0;
}