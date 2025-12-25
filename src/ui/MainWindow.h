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
    // ------------- Layout Members (Declaration Order: Keep This Order) -------------
    Gtk::Box m_mainBox;                  // 1. Main layout (Vertical)
    Gtk::Box m_srcBox, m_destBox;        // 2. Source/Target path layout (Horizontal)
    Gtk::Box m_algBox;                   // 3. Algorithm selection layout (Horizontal)
    Gtk::Box m_pwdBox;                   // 4. Password layout (Horizontal)
    Gtk::Box m_btnBox;                   // 5. Function button layout (Horizontal, Independent)
    Gtk::Box m_statusBox;                // 6. Status prompt layout (Horizontal, New)

    // ------------- Input/Select Widgets (Declaration Order: Keep This Order) -------------
    Gtk::Entry m_srcEntry, m_destEntry;  // 7. Path input boxes
    Gtk::ComboBoxText m_packCombo;
    Gtk::ComboBoxText m_compressCombo;
    Gtk::ComboBoxText m_cryptoCombo;
    Gtk::Entry m_pwdEntry;               // 11. Password input box
    Gtk::CheckButton m_pwdVisibleBtn;    // 12. Password visibility toggle (New)

    // ------------- Button Widgets (Declaration Order: Keep This Order) -------------
    Gtk::Button m_srcBtn;                // 13. Select source directory button
    Gtk::Button m_destBtn;               // 14. Select target directory button
    Gtk::Button m_backupBtn;             // 15. Execute backup button
    Gtk::Button m_restoreBtn;            // 16. Execute restore button

    // ------------- Label Widgets (Declaration Order: Keep This Order) -------------
    Gtk::Label m_srcLabel;               // 18. Source path label (New)
    Gtk::Label m_destLabel;              // 19. Target path label (New)
    Gtk::Label m_statusLabel;            // 20. Status prompt label

    // ------------- Core Business & Thread Related (Declaration Order: Keep This Order) -------------
    BackupCore m_backupCore;             // 21. Backup core instance
    BackupCore::BackupConfig m_backupConfig; // 22. Backup configuration
    sigc::connection m_backupTimerConn;  // 23. Timer connection
    int m_timerInterval;                 // 24. Scheduled backup interval (minutes)
    bool m_timerRunning;                 // 25. Whether scheduled backup is running (New)

    Glib::Dispatcher m_dispatcher;       // 26. Cross-thread notification
    bool m_backupSuccess;                // 27. Backup success flag
    bool m_restoreSuccess;               // 28. Restore success flag
    std::string m_currentTask;           // 29. Current task type
    Glib::ThreadPool m_threadPool;       // 30. Manually created thread pool

    // ------------- Signal Handling Functions -------------
    void on_select_src_clicked();
    void on_select_dest_clicked();
    void on_backup_clicked();
    void on_restore_clicked();
    bool on_timer_timeout();
    void on_timer_btn_clicked();
    void on_pwd_visible_toggled();
    void on_src_entry_changed();
    void on_dest_entry_changed();
    bool on_window_delete_event(GdkEventAny* event); 

    void start_timer(int interval_min);
    void stop_timer();
    void on_task_completed();
    void do_backup_task();
    void do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& password);
};