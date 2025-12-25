// MainWindow.h
#pragma once
#include <gtkmm.h>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>
#include <glibmm/threadpool.h>
#include "core/BackupCore.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    ~MainWindow() override;

private:
    // ------------- Layout Members (Improved Hierarchy) -------------
    Gtk::Box m_mainBox;                  // Main layout (Vertical)
    Gtk::Box m_srcBox, m_destBox;        // Source/Target path layout (Horizontal)
    Gtk::Box m_algBox;                   // Algorithm selection layout (Horizontal)
    Gtk::Box m_pwdBox;                   // Password layout (Horizontal)
    Gtk::Box m_btnBox;                   // Function button layout (Horizontal, Independent)
    Gtk::Box m_statusBox;                // Status prompt layout (Horizontal, New)

    // ------------- Input/Select Widgets (Supplement Missing) -------------
    Gtk::Entry m_srcEntry, m_destEntry;  // Path input boxes
    Gtk::ComboBoxText m_packCombo;       // Packing algorithm combo box
    Gtk::ComboBoxText m_compressCombo;   // Compression algorithm combo box
    Gtk::ComboBoxText m_cryptoCombo;     // Encryption algorithm combo box
    Gtk::Entry m_pwdEntry;               // Password input box
    Gtk::CheckButton m_pwdVisibleBtn;    // Password visibility toggle (New)

    // ------------- Button Widgets (Improve Naming & Function) -------------
    Gtk::Button m_srcBtn;                // Select source directory button
    Gtk::Button m_destBtn;               // Select target directory button
    Gtk::Button m_backupBtn;             // Execute backup button
    Gtk::Button m_restoreBtn;            // Execute restore button
    Gtk::Button m_timerBtn;              // Start/Stop scheduled backup button (New)

    // ------------- Label Widgets (Optimize Prompt) -------------
    Gtk::Label m_srcLabel;               // Source path label (New)
    Gtk::Label m_destLabel;              // Target path label (New)
    Gtk::Label m_statusLabel;            // Status prompt label

    // ------------- Core Business & Thread Related -------------
    BackupCore m_backupCore;
    BackupCore::BackupConfig m_backupConfig;
    sigc::connection m_backupTimerConn;
    int m_timerInterval;                 // Scheduled backup interval (minutes)
    bool m_timerRunning;                 // Whether scheduled backup is running (New)

    Glib::Dispatcher m_dispatcher;       // Cross-thread notification
    bool m_backupSuccess;
    bool m_restoreSuccess;
    std::string m_currentTask;
    Glib::ThreadPool m_threadPool;       // Manually created thread pool

    // ------------- Signal Handling Functions (Supplement Missing) -------------
    void on_select_src_clicked();
    void on_select_dest_clicked();
    void on_backup_clicked();
    void on_restore_clicked();
    bool on_timer_timeout();
    void on_timer_btn_clicked();         // Scheduled backup button click event (New)
    void on_pwd_visible_toggled();       // Password visibility toggle event (New)
    void on_src_entry_changed();         // Source input box content change event (New)
    void on_dest_entry_changed();        // Target input box content change event (New)
    bool on_window_delete_event();       // Window close event (New)

    void start_timer(int interval_min);
    void stop_timer();                   // Stop scheduled backup (New)
    void on_task_completed();
    void do_backup_task();
    void do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& packType);
};