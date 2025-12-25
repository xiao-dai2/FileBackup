// MainWindow.h
#pragma once
#include <gtkmm.h>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>
#include <glibmm/threadpool.h> // 线程池头文件
#include "core/BackupCore.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    ~MainWindow() override;

    // 公共接口获取备份核心实例
    BackupCore& getBackupCore() { return m_backupCore; }

private:
    // 布局类成员
    Gtk::Box m_mainBox;
    Gtk::Box m_srcBox, m_destBox;
    Gtk::Box m_algBox;
    Gtk::Box m_pwdBox;
    // 输入框/下拉框
    Gtk::Entry m_srcEntry, m_destEntry;
    Gtk::ComboBoxText m_packCombo;
    Gtk::ComboBoxText m_compressCombo;
    Gtk::ComboBoxText m_cryptoCombo;
    Gtk::Entry m_pwdEntry;
    // 按钮/标签
    Gtk::Button m_srcBtn, m_destBtn;
    Gtk::Button m_backupBtn, m_restoreBtn;
    Gtk::Label m_statusLabel;

    // 核心业务类
    BackupCore m_backupCore;
    BackupCore::BackupConfig m_backupConfig;

    // 定时器
    sigc::connection m_backupTimerConn;
    int m_timerInterval;

    // 跨线程通知与后台任务状态
    Glib::Dispatcher m_dispatcher;
    bool m_backupSuccess;
    bool m_restoreSuccess;
    std::string m_currentTask;

    // 新增：手动创建线程池实例（替代get_default()）
    Glib::ThreadPool m_threadPool; // 低版本glibmm适配，手动初始化

    // 信号处理函数
    void on_select_src_clicked();
    void on_select_dest_clicked();
    void on_backup_clicked();
    void on_restore_clicked();
    // 修复：定时器回调返回bool类型
    bool on_timer_timeout();
    void start_timer(int interval_min);
    void on_task_completed();
    void do_backup_task();
    void do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& password);
};