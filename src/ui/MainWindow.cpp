// MainWindow.cpp
#include "MainWindow.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

MainWindow::MainWindow()
    : m_mainBox(Gtk::ORIENTATION_VERTICAL, 10),
      m_srcBox(Gtk::ORIENTATION_HORIZONTAL, 5),
      m_destBox(Gtk::ORIENTATION_HORIZONTAL, 5),
      m_algBox(Gtk::ORIENTATION_HORIZONTAL, 5),
      m_pwdBox(Gtk::ORIENTATION_HORIZONTAL, 5),
      m_srcEntry(),
      m_destEntry(),
      m_packCombo(),
      m_compressCombo(),
      m_cryptoCombo(),
      m_pwdEntry(),
      m_srcBtn("选择源目录"),
      m_destBtn("选择目标目录"),
      m_backupBtn("执行备份"),
      m_restoreBtn("执行还原"),
      m_statusLabel("就绪"),
      m_timerInterval(3600),
      m_backupSuccess(false),
      m_restoreSuccess(false),
      m_currentTask(""),
      // 初始化手动创建的线程池：参数1=是否自动增长线程数，参数2=初始线程数（设为1即可满足需求）
      m_threadPool(true, 1)
{
    // 设置窗口属性
    this->set_title("数据备份软件（GTKmm版）");
    this->set_default_size(600, 300);
    this->set_border_width(15);

    // 1. 源路径布局
    m_srcEntry.set_placeholder_text("请选择待备份的目录...");
    m_srcBox.pack_start(m_srcEntry, Gtk::PACK_EXPAND_WIDGET);
    m_srcBox.pack_start(m_srcBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_srcBox, Gtk::PACK_SHRINK);

    // 2. 目标路径布局
    m_destEntry.set_placeholder_text("请选择备份存储目录...");
    m_destBox.pack_start(m_destEntry, Gtk::PACK_EXPAND_WIDGET);
    m_destBox.pack_start(m_destBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_destBox, Gtk::PACK_SHRINK);

    // 3. 算法选择布局
    m_packCombo.append("tar");
    m_packCombo.append("MyPack");
    m_packCombo.set_active(0);
    m_compressCombo.append("LZ77");
    m_compressCombo.append("Haff");
    m_compressCombo.set_active(0);
    m_cryptoCombo.append("AES");
    m_cryptoCombo.append("DES");
    m_cryptoCombo.set_active(0);

    Gtk::Label packLabel("打包算法：");
    Gtk::Label compressLabel("压缩算法：");
    Gtk::Label cryptoLabel("加密算法：");
    m_algBox.pack_start(packLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_packCombo, Gtk::PACK_SHRINK);
    m_algBox.pack_start(compressLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_compressCombo, Gtk::PACK_SHRINK);
    m_algBox.pack_start(cryptoLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_cryptoCombo, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_algBox, Gtk::PACK_SHRINK);

    // 4. 密码布局
    m_pwdEntry.set_visibility(false);
    m_pwdEntry.set_placeholder_text("请输入加密密码...");
    Gtk::Label pwdLabel("加密密码：");
    m_pwdBox.pack_start(pwdLabel, Gtk::PACK_SHRINK);
    m_pwdBox.pack_start(m_pwdEntry, Gtk::PACK_EXPAND_WIDGET);
    m_mainBox.pack_start(m_pwdBox, Gtk::PACK_SHRINK);

    // 5. 功能按钮布局
    Gtk::Box btnBox(Gtk::ORIENTATION_HORIZONTAL, 10);
    btnBox.pack_start(m_backupBtn, Gtk::PACK_SHRINK);
    btnBox.pack_start(m_restoreBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(btnBox, Gtk::PACK_SHRINK);

    // 6. 状态提示
    m_statusLabel.set_halign(Gtk::ALIGN_CENTER);
    m_mainBox.pack_start(m_statusLabel, Gtk::PACK_SHRINK);

    // 添加主布局到窗口
    this->add(m_mainBox);

    // 绑定信号处理函数
    m_srcBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_src_clicked));
    m_destBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_dest_clicked));
    m_backupBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_backup_clicked));
    m_restoreBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_restore_clicked));
    // 绑定dispatcher信号（后台任务完成后触发UI更新）
    m_dispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_task_completed));

    // 显示所有控件
    this->show_all_children();
}

MainWindow::~MainWindow() {
    // 断开定时器连接
    if (m_backupTimerConn.connected()) {
        m_backupTimerConn.disconnect();
    }

    // 修复：低版本glibmm线程池shutdown无需传递枚举参数
    m_threadPool.shutdown(); // 手动关闭线程池，等待任务完成
}

// 选择源目录
void MainWindow::on_select_src_clicked() {
    Gtk::FileChooserDialog dialog("选择源目录", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_srcEntry.set_text(path);
        m_backupConfig.srcPath = path;
    }
}

// 选择目标目录
void MainWindow::on_select_dest_clicked() {
    Gtk::FileChooserDialog dialog("选择目标目录", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_destEntry.set_text(path);
        m_backupConfig.destPath = path;
    }
}

// 执行备份（触发后台任务）
void MainWindow::on_backup_clicked() {
    // 更新备份配置
    m_backupConfig.packAlg = m_packCombo.get_active_text();
    m_backupConfig.compressAlg = m_compressCombo.get_active_text();
    m_backupConfig.cryptoAlg = m_cryptoCombo.get_active_text();
    m_backupConfig.password = m_pwdEntry.get_text();

    // 筛选规则配置
    m_backupConfig.filterRule.includeTypes = {"普通文件", "目录文件"};
    m_backupConfig.filterRule.minSize = 0;
    m_backupConfig.filterRule.maxSize = UINT64_MAX;

    // 校验路径
    if (m_backupConfig.srcPath.empty() || m_backupConfig.destPath.empty()) {
        m_statusLabel.set_text("错误：源目录和目标目录不能为空！");
        m_statusLabel.override_color(Gdk::RGBA("#FF0000"));
        return;
    }

    // 设置任务状态，禁用按钮避免重复点击
    m_currentTask = "backup";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_statusLabel.set_text("正在备份中...");
    m_statusLabel.override_color(Gdk::RGBA("#008000"));

    // 修复：使用手动创建的线程池提交任务（替代get_default()）
    m_threadPool.push(
        sigc::mem_fun(*this, &MainWindow::do_backup_task)
    );
}

// 后台备份任务（在子线程执行，不操作UI）
void MainWindow::do_backup_task() {
    // 执行备份逻辑，不直接操作UI
    m_backupSuccess = m_backupCore.backup(m_backupConfig);
    // 触发dispatcher，通知UI线程更新（线程安全）
    m_dispatcher.emit();
}

// 执行还原（触发后台任务）
void MainWindow::on_restore_clicked() {
    // 选择备份文件
    Gtk::FileChooserDialog dialog("选择备份文件", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    std::string backupFile;
    if (dialog.run() == Gtk::RESPONSE_OK) {
        backupFile = dialog.get_filename();
    } else {
        return;
    }

    // 还原路径
    std::string restorePath = m_destEntry.get_text();
    if (restorePath.empty()) {
        m_statusLabel.set_text("错误：还原目录不能为空！");
        m_statusLabel.override_color(Gdk::RGBA("#FF0000"));
        return;
    }

    // 获取加密算法和密码
    std::string cryptoAlg = m_cryptoCombo.get_active_text();
    std::string password = m_pwdEntry.get_text();

    // 设置任务状态，禁用按钮避免重复点击
    m_currentTask = "restore";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_statusLabel.set_text("正在还原中...");
    m_statusLabel.override_color(Gdk::RGBA("#008000"));

    // 修复：使用手动创建的线程池提交带参数的任务
    m_threadPool.push(
        sigc::bind(
            sigc::mem_fun(*this, &MainWindow::do_restore_task),
            backupFile,
            restorePath,
            cryptoAlg,
            password
        )
    );
}

// 后台还原任务（在子线程执行，不操作UI）
void MainWindow::do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& password) {
    // 执行还原逻辑，不直接操作UI
    m_restoreSuccess = m_backupCore.restore(backupFile, restorePath, cryptoAlg, password);
    // 触发dispatcher，通知UI线程更新（线程安全）
    m_dispatcher.emit();
}

// UI更新回调（由dispatcher触发，在主线程执行，安全操作UI）
void MainWindow::on_task_completed() {
    // 恢复按钮可用
    m_backupBtn.set_sensitive(true);
    m_restoreBtn.set_sensitive(true);

    // 根据任务类型更新UI状态
    if (m_currentTask == "backup") {
        if (m_backupSuccess) {
            m_statusLabel.set_text("备份成功！");
            m_statusLabel.override_color(Gdk::RGBA("#008000"));
        } else {
            m_statusLabel.set_text("备份失败！");
            m_statusLabel.override_color(Gdk::RGBA("#FF0000"));
        }
    } else if (m_currentTask == "restore") {
        if (m_restoreSuccess) {
            m_statusLabel.set_text("还原成功！");
            m_statusLabel.override_color(Gdk::RGBA("#008000"));
        } else {
            m_statusLabel.set_text("还原失败（密码错误或文件损坏）！");
            m_statusLabel.override_color(Gdk::RGBA("#FF0000"));
        }
    }

    // 重置任务类型
    m_currentTask = "";
}

// 修复：定时器回调函数返回bool类型（返回true表示继续定时，false表示停止）
bool MainWindow::on_timer_timeout() {
    on_backup_clicked(); // 复用备份逻辑
    return true; // 返回true，保持定时器持续运行
}

// 启动定时备份
void MainWindow::start_timer(int interval_min) {
    // 断开原有定时器
    if (m_backupTimerConn.connected()) {
        m_backupTimerConn.disconnect();
    }
    // 设置新间隔（分钟转秒，毫秒为单位）
    m_timerInterval = interval_min * 60;
    m_backupTimerConn = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &MainWindow::on_timer_timeout),
        m_timerInterval * 1000
    );
    m_statusLabel.set_text("定时备份已开启，每" + std::to_string(interval_min) + "分钟执行一次");
    m_statusLabel.override_color(Gdk::RGBA("#008000"));
}