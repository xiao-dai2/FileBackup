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

    // 过滤标签页
    Gtk::Notebook m_notebook_filters;
    
    // === 文件类型过滤页面 ===
    Gtk::Box m_vbox_file_type;
    Gtk::Label m_label_type_title;
    Gtk::Box m_hbox_type_options;
    
    // 左列
    Gtk::Box m_vbox_type_left;
    Gtk::CheckButton m_check_normal_file;
    Gtk::CheckButton m_check_link;
    Gtk::CheckButton m_check_block_device;
    Gtk::CheckButton m_check_pipe;
    
    // 右列
    Gtk::Box m_vbox_type_right;
    Gtk::CheckButton m_check_directory;
    Gtk::CheckButton m_check_char_device;
    Gtk::CheckButton m_check_socket;
    Gtk::CheckButton m_check_all_types;
    
    // === 时间过滤页面 ===
    Gtk::Box m_vbox_time;
    
    // 创建时间
    Gtk::Box m_hbox_create_time;
    Gtk::CheckButton m_check_create_time;
    Gtk::Label m_label_create_from;
    Gtk::Entry m_entry_create_from;
    Gtk::Label m_label_create_to;
    Gtk::Entry m_entry_create_to;
    
    // 修改时间
    Gtk::Box m_hbox_modify_time;
    Gtk::CheckButton m_check_modify_time;
    Gtk::Label m_label_modify_from;
    Gtk::Entry m_entry_modify_from;
    Gtk::Label m_label_modify_to;
    Gtk::Entry m_entry_modify_to;
    
    // === 文件大小过滤页面 ===
    Gtk::Box m_vbox_size;
    Gtk::Box m_hbox_size_filter;
    Gtk::CheckButton m_check_size_filter;
    Gtk::Label m_label_size_from;
    Gtk::Entry m_entry_size_from;
    Gtk::ComboBoxText m_combo_size_unit_from;
    Gtk::Label m_label_size_to;
    Gtk::Entry m_entry_size_to;
    Gtk::ComboBoxText m_combo_size_unit_to;
    
    // === 排除用户/组页面 ===
    Gtk::Box m_vbox_exclude_user;
    
    // 排除用户区域
    Gtk::Box m_hbox_exclude_user_title;
    Gtk::CheckButton m_check_exclude_user;
    Gtk::CheckButton m_check_exclude_group;
    
    Gtk::Box m_hbox_exclude_lists;
    
    // 左侧：排除用户列表
    Gtk::Box m_vbox_user_list;
    Gtk::ScrolledWindow m_scrolled_user_list;
    Gtk::TextView m_text_user_list;
    Gtk::Box m_hbox_user_add;
    Gtk::Label m_label_user;
    Gtk::Entry m_entry_user;
    Gtk::Button m_btn_add_user;
    
    // 右侧：排除用户组列表
    Gtk::Box m_vbox_group_list;
    Gtk::ScrolledWindow m_scrolled_group_list;
    Gtk::TextView m_text_group_list;
    Gtk::Box m_hbox_group_add;
    Gtk::Label m_label_group;
    Gtk::Entry m_entry_group;
    Gtk::Button m_btn_add_group;

    // === 排除文件名页面 ===
    Gtk::Box m_vbox_exclude_name;
    Gtk::Box m_hbox_exclude_name;
    Gtk::CheckButton m_check_exclude_name;
    Gtk::ScrolledWindow m_scrolled_exclude_name;
    Gtk::TextView m_text_exclude_name;
    
    // === 排除目录页面 ===
    Gtk::Box m_vbox_exclude_dir;
    Gtk::Box m_hbox_exclude_dir;
    Gtk::CheckButton m_check_exclude_dir;
    Gtk::ScrolledWindow m_scrolled_exclude_dir;
    Gtk::TextView m_text_exclude_dir;

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
    void on_add_user_clicked();
    void on_add_group_clicked();
    bool on_window_delete_event(GdkEventAny* event); 

    void start_timer(int interval_min);
    void stop_timer();
    void on_task_completed();
    void do_backup_task();
    void do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& password);
};