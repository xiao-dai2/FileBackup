#include "MainWindow.h"
#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;


MainWindow::MainWindow()
    : // 1. Layout Members
      m_mainBox(Gtk::ORIENTATION_VERTICAL, 15),
      m_srcBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_destBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_algBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_pwdBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_btnBox(Gtk::ORIENTATION_HORIZONTAL, 15),
      m_statusBox(Gtk::ORIENTATION_HORIZONTAL, 5),

      // 7. Input/Select Widgets
      m_srcEntry(),
      m_destEntry(),
      m_packCombo(),
      m_compressCombo(),
      m_cryptoCombo(),
      m_pwdEntry(),
      m_pwdVisibleBtn("Show Password"),

      // 13. Button Widgets
      m_srcBtn("Browse"),
      m_destBtn("Browse"),
      m_backupBtn("Execute Backup"),
      m_restoreBtn("Execute Restore"),

      // 18. Label Widgets
      m_srcLabel("Source Dir:"),
      m_destLabel("Target Dir:"),
      m_statusLabel("Ready"),

      // 文件类型页面
      m_vbox_file_type(Gtk::ORIENTATION_VERTICAL, 20),
      m_label_type_title("type"),
      m_hbox_type_options(Gtk::ORIENTATION_HORIZONTAL, 50),
      m_vbox_type_left(Gtk::ORIENTATION_VERTICAL, 10),
      m_vbox_type_right(Gtk::ORIENTATION_VERTICAL, 10),
      m_check_normal_file("- regular file"),
      m_check_link("l symbolic link"),
      m_check_block_device("b block device"),
      m_check_pipe("p pipe file"),
      m_check_directory("d directory"),
      m_check_char_device("c character device"),
      m_check_socket("s socket file"),
      m_check_all_types("all"),
      
      // 时间过滤页面
      m_vbox_time(Gtk::ORIENTATION_VERTICAL, 20),
      m_hbox_create_time(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_check_create_time("create time"),
      m_label_create_to("-"),
      m_hbox_modify_time(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_check_modify_time("modify time"),
      m_label_modify_to("-"),
      
      // 文件大小页面
      m_vbox_size(Gtk::ORIENTATION_VERTICAL, 20),
      m_hbox_size_filter(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_check_size_filter("file size"),
      m_label_size_to("-"),
      
      // 排除用户
      m_vbox_exclude_user(Gtk::ORIENTATION_VERTICAL, 20),
      m_hbox_exclude_user_title(Gtk::ORIENTATION_HORIZONTAL, 20),
      m_check_exclude_user("exclude user"),
      m_check_exclude_group("exclude group"),
      m_hbox_exclude_lists(Gtk::ORIENTATION_HORIZONTAL, 20),
      m_vbox_user_list(Gtk::ORIENTATION_VERTICAL, 10),
      m_vbox_group_list(Gtk::ORIENTATION_VERTICAL, 10),
      m_hbox_user_add(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_hbox_group_add(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_label_user("User"),
      m_label_group("Group"),
      m_btn_add_user("add user"),
      m_btn_add_group("add group"),
      
      // 排除文件名
      m_vbox_exclude_name(Gtk::ORIENTATION_VERTICAL, 20),
      m_hbox_exclude_name(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_check_exclude_name("exclude file name"),
      
      // 排除目录
      m_vbox_exclude_dir(Gtk::ORIENTATION_VERTICAL, 20),
      m_hbox_exclude_dir(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_check_exclude_dir("exclude directory"),

      // 21. Core Business & Thread Related
      m_backupCore(),
      m_backupConfig(),
      m_backupTimerConn(),
      m_timerInterval(60),
      m_timerRunning(false),
      m_dispatcher(),
      m_backupSuccess(false),
      m_restoreSuccess(false),
      m_currentTask(""),
      m_threadPool(true, 1) // Thread pool initialization
{
    // ============== Window Basic Configuration ==============
    this->set_title("File Backup Tool");
    this->set_default_size(700, 400);
    this->set_border_width(20);
    this->set_resizable(true);
    this->signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::on_window_delete_event));

    // ============== Source Path Layout Configuration ==============
    m_srcLabel.set_width_chars(8);
    m_srcLabel.set_halign(Gtk::ALIGN_END);
    m_srcEntry.set_placeholder_text("Please select or enter the directory to back up...");
    m_srcEntry.set_max_length(256);
    m_srcBtn.set_size_request(80, 30);

    m_srcBox.pack_start(m_srcLabel, Gtk::PACK_SHRINK);
    m_srcBox.pack_start(m_srcEntry, Gtk::PACK_EXPAND_WIDGET);
    m_srcBox.pack_start(m_srcBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_srcBox, Gtk::PACK_SHRINK);

    // ============== Target Path Layout Configuration ==============
    m_destLabel.set_width_chars(8);
    m_destLabel.set_halign(Gtk::ALIGN_END);
    m_destEntry.set_placeholder_text("Please select or enter backup storage/restore directory...");
    m_destEntry.set_max_length(256);
    m_destBtn.set_size_request(80, 30);

    m_destBox.pack_start(m_destLabel, Gtk::PACK_SHRINK);
    m_destBox.pack_start(m_destEntry, Gtk::PACK_EXPAND_WIDGET);
    m_destBox.pack_start(m_destBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_destBox, Gtk::PACK_SHRINK);


    //  // === 过滤条件标签页 ===
    m_mainBox.pack_start(m_notebook_filters, Gtk::PACK_EXPAND_WIDGET);
    
    // 1. 文件类型页面
    m_notebook_filters.append_page(m_vbox_file_type, "file type");
    m_vbox_file_type.set_border_width(20);
    m_label_type_title.set_halign(Gtk::ALIGN_START);
    m_vbox_file_type.pack_start(m_label_type_title, Gtk::PACK_SHRINK);
    m_vbox_file_type.pack_start(m_hbox_type_options, Gtk::PACK_EXPAND_WIDGET);
    
    // 左右两列
    m_hbox_type_options.pack_start(m_vbox_type_left, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_type_options.pack_start(m_vbox_type_right, Gtk::PACK_EXPAND_WIDGET);
    
    m_vbox_type_left.pack_start(m_check_normal_file, Gtk::PACK_SHRINK);
    m_vbox_type_left.pack_start(m_check_link, Gtk::PACK_SHRINK);
    m_vbox_type_left.pack_start(m_check_block_device, Gtk::PACK_SHRINK);
    m_vbox_type_left.pack_start(m_check_pipe, Gtk::PACK_SHRINK);
    
    m_vbox_type_right.pack_start(m_check_directory, Gtk::PACK_SHRINK);
    m_vbox_type_right.pack_start(m_check_char_device, Gtk::PACK_SHRINK);
    m_vbox_type_right.pack_start(m_check_socket, Gtk::PACK_SHRINK);
    m_vbox_type_right.pack_start(m_check_all_types, Gtk::PACK_SHRINK);
    
    // 默认选中
    m_check_normal_file.set_active(true);
    m_check_link.set_active(true);
    m_check_directory.set_active(true);
    
    // 2. 时间过滤页面
    m_notebook_filters.append_page(m_vbox_time, "time");
    m_vbox_time.set_border_width(20);
    
    // 创建时间
    m_vbox_time.pack_start(m_hbox_create_time, Gtk::PACK_SHRINK);
    m_hbox_create_time.pack_start(m_check_create_time, Gtk::PACK_SHRINK);
    m_hbox_create_time.pack_start(m_entry_create_from, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_create_time.pack_start(m_label_create_to, Gtk::PACK_SHRINK);
    m_hbox_create_time.pack_start(m_entry_create_to, Gtk::PACK_EXPAND_WIDGET);
    
    m_entry_create_from.set_text("2000/1/1 00:00");
    m_entry_create_to.set_text("2030/1/1 00:00");
    m_check_create_time.set_active(true);
    
    // 修改时间
    m_vbox_time.pack_start(m_hbox_modify_time, Gtk::PACK_SHRINK);
    m_hbox_modify_time.pack_start(m_check_modify_time, Gtk::PACK_SHRINK);
    m_hbox_modify_time.pack_start(m_entry_modify_from, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_modify_time.pack_start(m_label_modify_to, Gtk::PACK_SHRINK);
    m_hbox_modify_time.pack_start(m_entry_modify_to, Gtk::PACK_EXPAND_WIDGET);
    
    m_entry_modify_from.set_text("2000/1/1 00:00");
    m_entry_modify_to.set_text("2030/1/1 00:00");
    // m_entry_modify_from.set_sensitive(false);
    // m_entry_modify_to.set_sensitive(false);
    
    // 3. 文件大小页面
    m_notebook_filters.append_page(m_vbox_size, "file size");
    m_vbox_size.set_border_width(20);
    m_vbox_size.pack_start(m_hbox_size_filter, Gtk::PACK_SHRINK);
    
    m_hbox_size_filter.pack_start(m_check_size_filter, Gtk::PACK_SHRINK);
    m_hbox_size_filter.pack_start(m_entry_size_from, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_size_filter.pack_start(m_combo_size_unit_from, Gtk::PACK_SHRINK);
    m_hbox_size_filter.pack_start(m_label_size_to, Gtk::PACK_SHRINK);
    m_hbox_size_filter.pack_start(m_entry_size_to, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_size_filter.pack_start(m_combo_size_unit_to, Gtk::PACK_SHRINK);
    
    m_combo_size_unit_from.append("B");
    m_combo_size_unit_from.append("KB");
    m_combo_size_unit_from.append("MB");
    m_combo_size_unit_from.append("GB");
    m_combo_size_unit_from.set_active(0);
    
    m_combo_size_unit_to.append("B");
    m_combo_size_unit_to.append("KB");
    m_combo_size_unit_to.append("MB");
    m_combo_size_unit_to.append("GB");
    m_combo_size_unit_to.set_active(2);
    
    // 4. 排除用户页面
    m_notebook_filters.append_page(m_vbox_exclude_user, "exclude user/group");
    m_vbox_exclude_user.set_border_width(20);
    
    // 标题栏：两个复选框
    m_vbox_exclude_user.pack_start(m_hbox_exclude_user_title, Gtk::PACK_SHRINK);
    // 设置 hbox 为均匀分配空间
    m_hbox_exclude_user_title.set_homogeneous(true);
    // 将复选框放入 hbox，设置 expand 和 fill 为 true
    m_hbox_exclude_user_title.pack_start(m_check_exclude_user, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_exclude_user_title.pack_start(m_check_exclude_group, Gtk::PACK_EXPAND_WIDGET);

    m_check_exclude_user.set_active(false);
    m_check_exclude_group.set_active(false);
    
    // 左右分栏：用户列表和用户组列表
    m_vbox_exclude_user.pack_start(m_hbox_exclude_lists, Gtk::PACK_EXPAND_WIDGET);
    
    // 左侧：排除用户
    m_hbox_exclude_lists.pack_start(m_vbox_user_list, Gtk::PACK_EXPAND_WIDGET);
    m_vbox_user_list.pack_start(m_scrolled_user_list, Gtk::PACK_EXPAND_WIDGET);
    m_scrolled_user_list.add(m_text_user_list);
    m_scrolled_user_list.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_scrolled_user_list.set_min_content_height(200);
    m_text_user_list.get_buffer()->set_text("root\n");
    m_text_user_list.set_editable(true);
    
    // 用户添加区域
    m_vbox_user_list.pack_start(m_hbox_user_add, Gtk::PACK_SHRINK);
    m_hbox_user_add.pack_start(m_label_user, Gtk::PACK_SHRINK);
    m_hbox_user_add.pack_start(m_entry_user, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_user_add.pack_start(m_btn_add_user, Gtk::PACK_SHRINK);
    m_entry_user.set_placeholder_text("Enter user name");
    
    // 右侧：排除用户组
    m_hbox_exclude_lists.pack_start(m_vbox_group_list, Gtk::PACK_EXPAND_WIDGET);
    m_vbox_group_list.pack_start(m_scrolled_group_list, Gtk::PACK_EXPAND_WIDGET);
    m_scrolled_group_list.add(m_text_group_list);
    m_scrolled_group_list.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_scrolled_group_list.set_min_content_height(200);
    m_text_group_list.get_buffer()->set_text("root");
    m_text_group_list.set_editable(true);
    
    // 用户组添加区域
    m_vbox_group_list.pack_start(m_hbox_group_add, Gtk::PACK_SHRINK);
    m_hbox_group_add.pack_start(m_label_group, Gtk::PACK_SHRINK);
    m_hbox_group_add.pack_start(m_entry_group, Gtk::PACK_EXPAND_WIDGET);
    m_hbox_group_add.pack_start(m_btn_add_group, Gtk::PACK_SHRINK);
    m_entry_group.set_placeholder_text("Enter group name");
    
    // 5. 排除文件名页面
    m_notebook_filters.append_page(m_vbox_exclude_name, "exclude file name");
    m_vbox_exclude_name.set_border_width(20);
    m_vbox_exclude_name.pack_start(m_hbox_exclude_name, Gtk::PACK_SHRINK);
    m_hbox_exclude_name.pack_start(m_check_exclude_name, Gtk::PACK_SHRINK);
    
    m_vbox_exclude_name.pack_start(m_scrolled_exclude_name, Gtk::PACK_EXPAND_WIDGET);
    m_scrolled_exclude_name.add(m_text_exclude_name);
    m_scrolled_exclude_name.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_text_exclude_name.get_buffer()->set_text("*.tmp\n*.log\n*.cache");
    
    // 6. 排除目录页面
    m_notebook_filters.append_page(m_vbox_exclude_dir, "exclude directory");
    m_vbox_exclude_dir.set_border_width(20);
    m_vbox_exclude_dir.pack_start(m_hbox_exclude_dir, Gtk::PACK_SHRINK);
    m_hbox_exclude_dir.pack_start(m_check_exclude_dir, Gtk::PACK_SHRINK);
    
    m_vbox_exclude_dir.pack_start(m_scrolled_exclude_dir, Gtk::PACK_EXPAND_WIDGET);
    m_scrolled_exclude_dir.add(m_text_exclude_dir);
    m_scrolled_exclude_dir.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_text_exclude_dir.get_buffer()->set_text("/tmp\n/var/cache");

    // ============== Algorithm Selection Layout Configuration ==============
    m_packCombo.append("tar");
    m_packCombo.set_active(0);
    m_compressCombo.append("LZ77");
    m_compressCombo.append("Haff");
    m_compressCombo.set_active(0);
    m_cryptoCombo.append("AES");
    m_cryptoCombo.append("DES");
    m_cryptoCombo.set_active(0);

    Gtk::Label packLabel("Pack Alg:");
    Gtk::Label compressLabel("Compress Alg:");
    Gtk::Label cryptoLabel("Crypto Alg:");
    m_algBox.pack_start(packLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_packCombo, Gtk::PACK_SHRINK);
    m_algBox.pack_start(compressLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_compressCombo, Gtk::PACK_SHRINK);
    m_algBox.pack_start(cryptoLabel, Gtk::PACK_SHRINK);
    m_algBox.pack_start(m_cryptoCombo, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_algBox, Gtk::PACK_SHRINK);

    // ============== Password Layout Configuration ==============
    Gtk::Label pwdLabel("Password:");
    pwdLabel.set_width_chars(8);
    pwdLabel.set_halign(Gtk::ALIGN_END);
    m_pwdEntry.set_placeholder_text("Please enter backup encryption password (optional)...");
    m_pwdEntry.set_visibility(false);
    m_pwdEntry.set_max_length(32);

    m_pwdBox.pack_start(pwdLabel, Gtk::PACK_SHRINK);
    m_pwdBox.pack_start(m_pwdEntry, Gtk::PACK_EXPAND_WIDGET);
    m_pwdBox.pack_start(m_pwdVisibleBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_pwdBox, Gtk::PACK_SHRINK);

    // ============== Function Button Layout Configuration ==============
    m_backupBtn.set_size_request(120, 35);
    m_restoreBtn.set_size_request(120, 35);

    m_btnBox.pack_start(m_backupBtn, Gtk::PACK_SHRINK);
    m_btnBox.pack_start(m_restoreBtn, Gtk::PACK_SHRINK);
    m_btnBox.set_halign(Gtk::ALIGN_CENTER);
    m_mainBox.pack_start(m_btnBox, Gtk::PACK_SHRINK);

    // ============== Status Prompt Layout Configuration ==============
    Gtk::Label statusTitleLabel("Status:");
    statusTitleLabel.set_width_chars(8);
    statusTitleLabel.set_halign(Gtk::ALIGN_END);
    m_statusLabel.set_halign(Gtk::ALIGN_START);
    m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>");

    m_statusBox.pack_start(statusTitleLabel, Gtk::PACK_SHRINK);
    m_statusBox.pack_start(m_statusLabel, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_statusBox, Gtk::PACK_EXPAND_WIDGET);

    // ============== Signal Binding ==============
    m_srcBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_src_clicked));
    m_destBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_dest_clicked));
    m_backupBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_backup_clicked));
    m_restoreBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_restore_clicked));
    m_pwdVisibleBtn.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_pwd_visible_toggled));
    m_srcEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_src_entry_changed));
    m_destEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_dest_entry_changed));
    m_dispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_task_completed));
    m_btn_add_user.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_add_user_clicked));
    m_btn_add_group.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_add_group_clicked));

    // ============== Show All Widgets ==============
    this->add(m_mainBox);
    this->show_all_children();
}

MainWindow::~MainWindow() {
    stop_timer();
    m_threadPool.shutdown();
}

bool MainWindow::on_window_delete_event(GdkEventAny* event) {
    (void) event;
    std::cout << "Program is exiting gracefully..." << std::endl;
    stop_timer();
    Gtk::Main::quit();
    return true; // 返回true表示已处理事件，阻止默认关闭行为（此处我们主动调用quit，不影响）
}
void MainWindow::on_select_src_clicked() {
    Gtk::FileChooserDialog dialog("Select Directory to Back Up", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.set_transient_for(*this);

    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_srcEntry.set_text(path);
        m_backupConfig.srcPath = path;
    }
}

void MainWindow::on_select_dest_clicked() {
    Gtk::FileChooserDialog dialog("Select Backup Storage/Restore Directory", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.set_transient_for(*this);

    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_destEntry.set_text(path);
        m_backupConfig.destPath = path;
    }
}

void MainWindow::on_src_entry_changed() {
    std::string srcPath = std::string(m_srcEntry.get_text());
    m_backupConfig.srcPath = srcPath;
    
    if (!srcPath.empty() && !fs::exists(srcPath)) {
        m_statusLabel.set_markup("<span color='#DC143C'>Warning: Source directory does not exist</span>");
    } else if (std::string(m_destEntry.get_text()).empty()) {
        m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>");
    }
}

void MainWindow::on_dest_entry_changed() {
    std::string destPath = std::string(m_destEntry.get_text());
    m_backupConfig.destPath = destPath;
    
    if (!destPath.empty() && !fs::exists(destPath)) {
        m_statusLabel.set_markup("<span color='#DC143C'>Warning: Target directory does not exist</span>");
    } else if (std::string(m_srcEntry.get_text()).empty()) {
        m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>");
    }
}

void MainWindow::on_pwd_visible_toggled() {
    m_pwdEntry.set_visibility(m_pwdVisibleBtn.get_active());
}

void MainWindow::on_backup_clicked() {
    std::string srcPath = std::string(m_srcEntry.get_text());
    std::string destPath = std::string(m_destEntry.get_text());

    if (srcPath.empty()) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Please select source directory</span>");
        return;
    }
    if (destPath.empty()) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Please select target directory</span>");
        return;
    }
    if (!fs::exists(srcPath)) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Source directory does not exist</span>");
        return;
    }
    if (!fs::exists(destPath)) {
        try {
            fs::create_directories(destPath);
            m_statusLabel.set_markup("<span color='#FF8C00'>Tip: Target directory created automatically</span>");
        } catch (const fs::filesystem_error& e) {
            m_statusLabel.set_markup("<span color='#DC143C'>Error: Failed to create target directory</span>");
            return;
        }
    }

    m_backupConfig.packAlg = std::string(m_packCombo.get_active_text());
    m_backupConfig.compressAlg = std::string(m_compressCombo.get_active_text());
    m_backupConfig.cryptoAlg = std::string(m_cryptoCombo.get_active_text());
    m_backupConfig.password = std::string(m_pwdEntry.get_text());
    m_backupConfig.filterRule.includeTypes = {"all"};
    m_backupConfig.filterRule.minSize = 0;
    m_backupConfig.filterRule.maxSize = UINT64_MAX;

    m_currentTask = "backup";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_statusLabel.set_markup("<span color='#FF8C00'>Backing up, please wait...</span>");

    m_threadPool.push(sigc::mem_fun(*this, &MainWindow::do_backup_task));
}

void MainWindow::on_add_user_clicked() {
    Glib::ustring user = m_entry_user.get_text();
    if (!user.empty()) {
        auto buffer = m_text_user_list.get_buffer();
        Glib::ustring current = buffer->get_text();
        if (!current.empty() && current[current.length()-1] != '\n') {
            current += "\n";
        }
        current += user;
        buffer->set_text(current);
        m_entry_user.set_text("");
        std::cout << "添加用户: " << user << std::endl;
    }
}

void MainWindow::on_add_group_clicked() {
    Glib::ustring group = m_entry_group.get_text();
    if (!group.empty()) {
        auto buffer = m_text_group_list.get_buffer();
        Glib::ustring current = buffer->get_text();
        if (!current.empty() && current[current.length()-1] != '\n') {
            current += "\n";
        }
        current += group;
        buffer->set_text(current);
        m_entry_group.set_text("");
        std::cout << "添加用户组: " << group << std::endl;
    }
}

void MainWindow::do_backup_task() {
    m_backupSuccess = m_backupCore.backup(m_backupConfig);
    m_dispatcher.emit();
}

void MainWindow::on_restore_clicked() {
    Gtk::FileChooserDialog dialog("Select Backup File", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.set_transient_for(*this);
    
    std::string backupFile;
    if (dialog.run() == Gtk::RESPONSE_OK) {
        backupFile = dialog.get_filename();
    } else {
        return;
    }

    std::string restorePath = std::string(m_destEntry.get_text());

    if (restorePath.empty()) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Please select restore directory</span>");
        return;
    }
    if (!fs::exists(restorePath)) {
        try {
            fs::create_directories(restorePath);
            m_statusLabel.set_markup("<span color='#FF8C00'>Tip: Restore directory created automatically</span>");
        } catch (const fs::filesystem_error& e) {
            m_statusLabel.set_markup("<span color='#DC143C'>Error: Failed to create restore directory</span>");
            return;
        }
    }

    std::string cryptoAlg = std::string(m_cryptoCombo.get_active_text());
    std::string password = std::string(m_pwdEntry.get_text());

    m_currentTask = "restore";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_statusLabel.set_markup("<span color='#FF8C00'>Restoring, please wait...</span>");

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

void MainWindow::do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& password) {
    m_restoreSuccess = m_backupCore.restore(backupFile, restorePath, cryptoAlg, password);
    m_dispatcher.emit();
}

void MainWindow::start_timer(int interval_min) {
    stop_timer();
    m_timerInterval = interval_min * 60;
    m_backupTimerConn = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &MainWindow::on_timer_timeout),
        m_timerInterval * 1000
    );
}

void MainWindow::stop_timer() {
    if (m_backupTimerConn.connected()) {
        m_backupTimerConn.disconnect();
    }
}

bool MainWindow::on_timer_timeout() {
    if (m_timerRunning) {
        on_backup_clicked();
        return true;
    }
    return false;
}

void MainWindow::on_task_completed() {
    m_backupBtn.set_sensitive(true);
    m_restoreBtn.set_sensitive(true);

    if (m_currentTask == "backup") {
        if (m_backupSuccess) {
            m_statusLabel.set_markup("<span color='#2E8B57'>Backup succeeded!</span>");
        } else {
            m_statusLabel.set_markup("<span color='#DC143C'>Backup failed!</span>");
        }
    } else if (m_currentTask == "restore") {
        if (m_restoreSuccess) {
            m_statusLabel.set_markup("<span color='#2E8B57'>Restore succeeded!</span>");
        } else {
            m_statusLabel.set_markup("<span color='#DC143C'>Restore failed (Wrong password or damaged file)!</span>");
        }
    }

    m_currentTask = "";
}