#include "MainWindow.h"
#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

MainWindow::MainWindow()
    : // Layout initialization (Unified spacing, optimized hierarchy)
      m_mainBox(Gtk::ORIENTATION_VERTICAL, 15),
      m_srcBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_destBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_algBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_pwdBox(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_btnBox(Gtk::ORIENTATION_HORIZONTAL, 15),
      m_statusBox(Gtk::ORIENTATION_HORIZONTAL, 5),

      // Label initialization (Supplement prompt text, optimize style)
      m_srcLabel("Source Dir:"),
      m_destLabel("Target Dir:"),
      m_statusLabel("Ready"),

      // Button initialization (Unified style, supplement text)
      m_srcBtn("Browse"),
      m_destBtn("Browse"),
      m_backupBtn("Execute Backup"),
      m_restoreBtn("Execute Restore"),
      m_timerBtn("Start Scheduled Backup"),
      m_pwdVisibleBtn("Show Password"),

      // Input box initialization (Optimize placeholders)
      m_srcEntry(),
      m_destEntry(),
      m_pwdEntry(),

      // Combo box initialization
      m_packCombo(),
      m_compressCombo(),
      m_cryptoCombo(),

      // Business parameter initialization
      m_timerInterval(60),
      m_timerRunning(false),
      m_backupSuccess(false),
      m_restoreSuccess(false),
      m_currentTask(""),
      m_threadPool(true, 1) // Thread pool initialization
{
    // ============== Window Basic Configuration ==============
    this->set_title("Data Backup Tool (GTKmm Full Version)");
    this->set_default_size(700, 400); // Enlarge window size for better experience
    this->set_border_width(20);
    this->set_resizable(true); // Allow window resizing
    this->signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::on_window_delete_event));

    // ============== Source Path Layout Configuration ==============
    m_srcLabel.set_width_chars(8); // Fixed label width for alignment
    m_srcLabel.set_halign(Gtk::ALIGN_END);
    m_srcEntry.set_placeholder_text("Please select or enter the directory to back up...");
    m_srcEntry.set_max_length(256); // Limit input length
    m_srcBtn.set_size_request(80, 30); // Fixed button size

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

    // ============== Algorithm Selection Layout Configuration ==============
    // Packing algorithm sub-layout
    Gtk::Box packSubBox(Gtk::ORIENTATION_HORIZONTAL, 5);
    Gtk::Label packLabel("Pack Alg:");
    packLabel.set_width_chars(8);
    m_packCombo.append("tar");
    m_packCombo.append("zip"); // Supplement optional algorithms
    m_packCombo.set_active(0);
    m_packCombo.set_size_request(100, 30);
    packSubBox.pack_start(packLabel, Gtk::PACK_SHRINK);
    packSubBox.pack_start(m_packCombo, Gtk::PACK_SHRINK);

    // Compression algorithm sub-layout
    Gtk::Box compressSubBox(Gtk::ORIENTATION_HORIZONTAL, 5);
    Gtk::Label compressLabel("Compress Alg:");
    compressLabel.set_width_chars(8);
    m_compressCombo.append("LZ77");
    m_compressCombo.append("Deflate");
    m_compressCombo.set_active(0);
    m_compressCombo.set_size_request(100, 30);
    compressSubBox.pack_start(compressLabel, Gtk::PACK_SHRINK);
    compressSubBox.pack_start(m_compressCombo, Gtk::PACK_SHRINK);

    // Encryption algorithm sub-layout
    Gtk::Box cryptoSubBox(Gtk::ORIENTATION_HORIZONTAL, 5);
    Gtk::Label cryptoLabel("Crypto Alg:");
    cryptoLabel.set_width_chars(8);
    m_cryptoCombo.append("AES");
    m_cryptoCombo.append("DES");
    m_cryptoCombo.set_active(0);
    m_cryptoCombo.set_size_request(100, 30);
    cryptoSubBox.pack_start(cryptoLabel, Gtk::PACK_SHRINK);
    cryptoSubBox.pack_start(m_cryptoCombo, Gtk::PACK_SHRINK);

    // Assemble algorithm layout
    m_algBox.pack_start(packSubBox, Gtk::PACK_SHRINK);
    m_algBox.pack_start(compressSubBox, Gtk::PACK_SHRINK);
    m_algBox.pack_start(cryptoSubBox, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_algBox, Gtk::PACK_SHRINK);

    // ============== Password Layout Configuration (Supplement Visibility Toggle) ==============
    Gtk::Label pwdLabel("Password:");
    pwdLabel.set_width_chars(8);
    pwdLabel.set_halign(Gtk::ALIGN_END);
    m_pwdEntry.set_placeholder_text("Please enter backup encryption password (optional)...");
    m_pwdEntry.set_visibility(false); // Hide password by default
    m_pwdEntry.set_max_length(32);

    m_pwdBox.pack_start(pwdLabel, Gtk::PACK_SHRINK);
    m_pwdBox.pack_start(m_pwdEntry, Gtk::PACK_EXPAND_WIDGET);
    m_pwdBox.pack_start(m_pwdVisibleBtn, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_pwdBox, Gtk::PACK_SHRINK);

    // ============== Function Button Layout Configuration (Supplement Timer Button) ==============
    m_backupBtn.set_size_request(120, 35);
    m_restoreBtn.set_size_request(120, 35);
    m_timerBtn.set_size_request(120, 35);

    m_btnBox.pack_start(m_backupBtn, Gtk::PACK_SHRINK);
    m_btnBox.pack_start(m_restoreBtn, Gtk::PACK_SHRINK);
    m_btnBox.pack_start(m_timerBtn, Gtk::PACK_SHRINK);
    m_btnBox.set_halign(Gtk::ALIGN_CENTER); // Buttons centered
    m_mainBox.pack_start(m_btnBox, Gtk::PACK_SHRINK);

    // ============== Status Prompt Layout Configuration (Optimize Style) ==============
    Gtk::Label statusTitleLabel("Status:");
    statusTitleLabel.set_width_chars(8);
    statusTitleLabel.set_halign(Gtk::ALIGN_END);
    m_statusLabel.set_halign(Gtk::ALIGN_START);
    m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>"); // Rich text style

    m_statusBox.pack_start(statusTitleLabel, Gtk::PACK_SHRINK);
    m_statusBox.pack_start(m_statusLabel, Gtk::PACK_SHRINK);
    m_mainBox.pack_start(m_statusBox, Gtk::PACK_EXPAND_WIDGET); // Occupy remaining space

    // ============== Signal Binding (Supplement All Interactive Events) ==============
    // Path selection buttons
    m_srcBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_src_clicked));
    m_destBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_select_dest_clicked));

    // Function buttons
    m_backupBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_backup_clicked));
    m_restoreBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_restore_clicked));
    m_timerBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_timer_btn_clicked));

    // Password visibility toggle
    m_pwdVisibleBtn.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_pwd_visible_toggled));

    // Input box content change
    m_srcEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_src_entry_changed));
    m_destEntry.signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_dest_entry_changed));

    // Cross-thread notification
    m_dispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_task_completed));

    // ============== Show All Widgets ==============
    this->add(m_mainBox);
    this->show_all_children();
}

MainWindow::~MainWindow() {
    // Stop timer
    stop_timer();
    // Shutdown thread pool
    m_threadPool.shutdown();
}

// ============== Window Close Event (Graceful Exit) ==============
bool MainWindow::on_window_delete_event() {
    std::cout << "Program is exiting gracefully..." << std::endl;
    stop_timer();
    Gtk::Main::quit();
    return true;
}

// ============== Source Directory Selection Event ==============
void MainWindow::on_select_src_clicked() {
    Gtk::FileChooserDialog dialog("Select Directory to Back Up", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.set_transient_for(*this); // Dialog attaches to main window

    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_srcEntry.set_text(path);
        m_backupConfig.srcPath = path;
    }
}

// ============== Target Directory Selection Event ==============
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

// ============== Source Input Box Content Change Event ==============
void MainWindow::on_src_entry_changed() {
    m_backupConfig.srcPath = m_srcEntry.get_text();
    // Real-time path validity check
    if (!m_srcEntry.get_text().empty() && !fs::exists(m_srcEntry.get_text())) {
        m_statusLabel.set_markup("<span color='#DC143C'>Warning: Source directory does not exist</span>");
    } else if (m_destEntry.get_text().empty()) {
        m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>");
    }
}

// ============== Target Input Box Content Change Event ==============
void MainWindow::on_dest_entry_changed() {
    m_backupConfig.destPath = m_destEntry.get_text();
    // Real-time path validity check
    if (!m_destEntry.get_text().empty() && !fs::exists(m_destEntry.get_text())) {
        m_statusLabel.set_markup("<span color='#DC143C'>Warning: Target directory does not exist</span>");
    } else if (m_srcEntry.get_text().empty()) {
        m_statusLabel.set_markup("<span color='#2E8B57'>Ready</span>");
    }
}

// ============== Password Visibility Toggle Event ==============
void MainWindow::on_pwd_visible_toggled() {
    m_pwdEntry.set_visibility(m_pwdVisibleBtn.get_active());
}

// ============== Execute Backup Event ==============
void MainWindow::on_backup_clicked() {
    // Path validity check
    if (m_backupConfig.srcPath.empty()) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Please select source directory</span>");
        return;
    }
    if (m_backupConfig.destPath.empty()) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Please select target directory</span>");
        return;
    }
    if (!fs::exists(m_backupConfig.srcPath)) {
        m_statusLabel.set_markup("<span color='#DC143C'>Error: Source directory does not exist</span>");
        return;
    }
    if (!fs::exists(m_backupConfig.destPath)) {
        // Auto create target directory
        try {
            fs::create_directories(m_backupConfig.destPath);
            m_statusLabel.set_markup("<span color='#FF8C00'>Tip: Target directory created automatically</span>");
        } catch (const fs::filesystem_error& e) {
            m_statusLabel.set_markup("<span color='#DC143C'>Error: Failed to create target directory</span>");
            return;
        }
    }

    // Update backup configuration
    m_backupConfig.packAlg = m_packCombo.get_active_text();
    m_backupConfig.compressAlg = m_compressCombo.get_active_text();
    m_backupConfig.cryptoAlg = m_cryptoCombo.get_active_text();
    m_backupConfig.password = m_pwdEntry.get_text();
    m_backupConfig.filterRule.includeTypes = {"Regular File", "Directory"};
    m_backupConfig.filterRule.minSize = 0;
    m_backupConfig.filterRule.maxSize = UINT64_MAX;

    // Set task status
    m_currentTask = "backup";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_timerBtn.set_sensitive(false);
    m_statusLabel.set_markup("<span color='#FF8C00'>Backing up, please wait...</span>");

    // Submit background task
    m_threadPool.push(sigc::mem_fun(*this, &MainWindow::do_backup_task));
}

// ============== Background Backup Task ==============
void MainWindow::do_backup_task() {
    m_backupSuccess = m_backupCore.backup(m_backupConfig);
    m_dispatcher.emit();
}

// ============== Execute Restore Event ==============
void MainWindow::on_restore_clicked() {
    // Select backup file
    Gtk::FileChooserDialog dialog("Select Backup File", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.set_transient_for(*this);

    // Add file filter (only show tar/zip backup files)
    Gtk::FileFilter filter;
    filter.set_name("Backup Files (*.tar *.zip)");
    filter.add_pattern("*.tar");
    filter.add_pattern("*.zip");
    dialog.add_filter(filter);

    std::string backupFile;
    if (dialog.run() == Gtk::RESPONSE_OK) {
        backupFile = dialog.get_filename();
    } else {
        return;
    }

    // Restore path check
    std::string restorePath = m_backupConfig.destPath;
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

    // Get configuration parameters
    std::string cryptoAlg = m_cryptoCombo.get_active_text();
    std::string packType = m_packCombo.get_active_text();
    std::string password = m_pwdEntry.get_text();

    // Set task status
    m_currentTask = "restore";
    m_backupBtn.set_sensitive(false);
    m_restoreBtn.set_sensitive(false);
    m_timerBtn.set_sensitive(false);
    m_statusLabel.set_markup("<span color='#FF8C00'>Restoring, please wait...</span>");

    // Submit background task
    m_threadPool.push(
        sigc::bind(
            sigc::mem_fun(*this, &MainWindow::do_restore_task),
            backupFile,
            restorePath,
            cryptoAlg,
            packType
        )
    );
}

// ============== Background Restore Task ==============
void MainWindow::do_restore_task(const std::string& backupFile, const std::string& restorePath, const std::string& cryptoAlg, const std::string& packType) {
    m_restoreSuccess = m_backupCore.restore(backupFile, restorePath, cryptoAlg, packType, m_pwdEntry.get_text());
    m_dispatcher.emit();
}

// ============== Scheduled Backup Button Event ==============
void MainWindow::on_timer_btn_clicked() {
    if (!m_timerRunning) {
        // Start scheduled backup
        if (m_backupConfig.srcPath.empty() || m_backupConfig.destPath.empty()) {
            m_statusLabel.set_markup("<span color='#DC143C'>Error: Please configure source and target directories first</span>");
            return;
        }
        // Pop up interval input dialog
        Gtk::Dialog dialog("Set Scheduled Backup Interval", *this);
        dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

        Gtk::Box* contentArea = dialog.get_content_area();
        Gtk::Box inputBox(Gtk::ORIENTATION_HORIZONTAL, 10);
        Gtk::Label label("Backup Interval (minutes):");
        Gtk::Entry intervalEntry;
        intervalEntry.set_text(std::to_string(m_timerInterval / 60));
        intervalEntry.set_width_chars(10);

        inputBox.pack_start(label, Gtk::PACK_SHRINK);
        inputBox.pack_start(intervalEntry, Gtk::PACK_SHRINK);
        contentArea->pack_start(inputBox, Gtk::PACK_SHRINK);
        dialog.show_all_children();

        if (dialog.run() == Gtk::RESPONSE_OK) {
            try {
                int interval = std::stoi(intervalEntry.get_text());
                if (interval < 1) {
                    m_statusLabel.set_markup("<span color='#DC143C'>Error: Interval cannot be less than 1 minute</span>");
                    return;
                }
                start_timer(interval);
                m_timerRunning = true;
                m_timerBtn.set_label("Stop Scheduled Backup");
                m_statusLabel.set_markup("<span color='#4169E1'>Scheduled backup started, execute every " + std::to_string(interval) + " minutes</span>");
            } catch (const std::invalid_argument& e) {
                m_statusLabel.set_markup("<span color='#DC143C'>Error: Please enter a valid number</span>");
            }
        }
    } else {
        // Stop scheduled backup
        stop_timer();
        m_timerRunning = false;
        m_timerBtn.set_label("Start Scheduled Backup");
        m_statusLabel.set_markup("<span color='#2E8B57'>Scheduled backup stopped</span>");
    }
}

// ============== Start Scheduled Backup ==============
void MainWindow::start_timer(int interval_min) {
    stop_timer(); // Stop existing timer first
    m_timerInterval = interval_min * 60; // Convert to seconds
    m_backupTimerConn = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &MainWindow::on_timer_timeout),
        m_timerInterval * 1000 // Convert to milliseconds
    );
}

// ============== Stop Scheduled Backup ==============
void MainWindow::stop_timer() {
    if (m_backupTimerConn.connected()) {
        m_backupTimerConn.disconnect();
    }
}

// ============== Timer Timeout Callback ==============
bool MainWindow::on_timer_timeout() {
    if (m_timerRunning) {
        on_backup_clicked(); // Reuse backup logic
        return true; // Continue timing
    }
    return false; // Stop timing
}

// ============== UI Update After Background Task Completion ==============
void MainWindow::on_task_completed() {
    // Restore button availability
    m_backupBtn.set_sensitive(true);
    m_restoreBtn.set_sensitive(true);
    m_timerBtn.set_sensitive(true);

    // Update status prompt
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

    // Reset task type
    m_currentTask = "";
}