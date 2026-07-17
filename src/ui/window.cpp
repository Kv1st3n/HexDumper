#include <sstream>

// to link the back-end C stuff to the GUI
extern "C" {
    #include "hexDumper.h"
    #include "scanner.h"
    #include "checksum.h"
    #include "sigdb.h"
    #include "file_handler.h"
}

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h> 
#include <gdkmm/display.h> 
#include "CustomButton.h"
#include "CustomWindow.h"

// fix stack with widgets

class ForxWindow : public Gtk::ApplicationWindow {  
public:
    ForxWindow() {
        set_title("forx");
        set_default_size(1200, 800);

        setup_custom_header();
        setup_main_widget();
        
        m_main_box.set_orientation(Gtk::Orientation::VERTICAL);
        m_main_box.set_margin(15);

        m_label.set_text("Welcome to Forx");
        m_main_box.append(m_label);
        m_main_box.append(*m_master_view_manager);
        
        set_child(m_main_box);
    }

    virtual ~ForxWindow() {}

    // setup button actions
protected:
    void on_open_clicked() {
        if (m_selected_mode == "directory_scanner") {
            m_file_picker_manager.show_directory_picker(*this, "", [this](const std::string &path) {
                m_loaded_file = path;
                std::cout << "Directory loaded: " << path << "\n";
                
                m_master_view_manager->set_visible_child("grid_layout");
                set_grid_text("Directory loaded: " + path + "\nClick 'Run' to scan.");
            });
        } else {
            m_file_picker_manager.show_picker(*this, "", [this](const std::string &path) {
                m_loaded_file = path;
                std::cout << "File loaded: " << path << "\n";
                m_hex_text_view->get_buffer()->set_text(
                    "File loaded: " + path + "\nSelect a mode and click Run.");
                m_master_view_manager->set_visible_child("hex_layout");
            });
        }
    }

    void on_dir_clicked() {
        m_file_picker_manager.show_directory_picker(*this, "", [this](const std::string &path) {
            m_loaded_file = path;
            std::cout << "Directory loaded: " << path << "\n";
            
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text("Directory loaded: " + path + "\nSelect 'Directory Scanner' mode and click 'Run'.");
            
            m_selected_mode = "directory_scanner";
        });
    }

    void on_mode_clicked() {
        m_mode_selector.show_mode_menu(m_button_mode, [this](const std::string &mode) {
            m_selected_mode = mode;

            if (mode == "hex_dump" || mode == "reverse_mode" || mode == "string_extractor" ||
                mode == "compact" || mode == "lowercase") {
                
                m_master_view_manager->set_visible_child("hex_layout");
                
                auto buf = m_hex_text_view->get_buffer();
                buf->set_text("Switched to mode: " + mode + "\nClick 'Run' to analyze.");
            } 
            else if (mode == "md5" || mode == "sha1" || mode == "sha256" || 
                    mode == "sha512" || mode == "sha224" || mode == "sha384" || mode == "file_identifier" || mode == "directory_scanner") {
                
                m_master_view_manager->set_visible_child("grid_layout");
                
                std::cout << "Switched layout to grid view for hashing/scanning mode: " << mode << std::endl;
            }
        });
    }

    void on_run_clicked() {
        std::cout << "Run: mode=" << m_selected_mode << "  file=" << m_loaded_file << "\n";

        if (m_loaded_file.empty()) {
            m_hex_text_view->get_buffer()->set_text("No file loaded. Click Open File first.");
            return;
        }

        std::string out;
        bool is_grid_layout = false;

        // ── hex layout modes ─────────────────────────────────────
        if (m_selected_mode == "hex_dump") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                dump_hex(raw_file, f, 0, 0, 0);
            });
        } else if (m_selected_mode == "reverse_mode") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                reverse_dump(raw_file, f);
            });
        } else if (m_selected_mode == "string_extractor") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                extract_strings(raw_file, f);
            });
        } else if (m_selected_mode == "file_identifier") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                uint8_t buf[16];
                size_t n = fread(buf, 1, 16, raw_file);
                fprintf(f, "Type: %s\n", sigdb_identify(buf, n));
            });
        } else if (m_selected_mode == "compact") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                dump_hex(raw_file, f, 0, 1, 0);
            });
        } else if (m_selected_mode == "lowercase") {
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) {
                dump_hex(raw_file, f, 1, 0, 0);
            });

        // ── grid layout modes ────────────────────────────────────
        } else if (m_selected_mode == "md5") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 1, f); });
        } else if (m_selected_mode == "sha1") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 2, f); });
        } else if (m_selected_mode == "sha256") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 3, f); });
        } else if (m_selected_mode == "sha512") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 4, f); });
        } else if (m_selected_mode == "sha224") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 5, f); });
        } else if (m_selected_mode == "sha384") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [](FILE* raw_file, FILE* f) { print_checksum(raw_file, 6, f); });
        } else if (m_selected_mode == "directory_scanner") {
            is_grid_layout = true;
            out = execute_with_file(m_loaded_file, [this](FILE*, FILE* f) {
                scan_directory(m_loaded_file.c_str(), f);
            });
        }

        // ── Single UI Update logic ───────────────────────────────
        if (is_grid_layout) {
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);
        } else {
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);
        }
    }

    std::string execute_with_file(const std::string& filepath, const std::function<void(FILE*, FILE*)>& action) {
        return capture([&](FILE *f) {
            if (m_selected_mode == "directory_scanner") {
                action(nullptr, f);
                return;
            }

            BinaryFileHandle *bfh = bfile_open(filepath.c_str());
            if (!bfh) { 
                fprintf(f, "error: cannot open file\n"); 
                return; 
            }
            
            action(bfile_raw(bfh), f);
            
            bfile_close(bfh);
        });
    }

    void on_save_clicked() {
        std::string current_output;
        if (m_master_view_manager->get_visible_child_name() == "hex_layout") {
            current_output = m_hex_text_view->get_buffer()->get_text();
        } else {
            auto *child = m_scan_results_grid->get_first_child();
            if (child) {
                auto *lbl = dynamic_cast<Gtk::Label *>(child);
                if (lbl) {
                    current_output = lbl->get_text();
                }
            }
        }
        m_save_mode.show_save(*this, current_output, m_dark_mode);
    }

    void on_about_clicked() {
        m_about_mode.show_about(*this, m_dark_mode);
    }

    void on_settings_clicked() {
        m_settings_mode.show_settings(*this, [this](bool dark) {
            set_dark_mode(dark);
        }, m_dark_mode);
    }

private:

    // sets up header
    void setup_custom_header() {
        auto *title_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        title_box->set_halign(Gtk::Align::CENTER);

        auto *title_lable = Gtk::make_managed<Gtk::Label>("<b>forx</b>");
        title_lable->set_use_markup(true);
        title_lable->set_halign(Gtk::Align::CENTER);

        auto *subtitle_label = Gtk::make_managed<Gtk::Label>("Binary analysis tool");
        subtitle_label->set_css_classes({"subtitle"});
        subtitle_label->set_halign(Gtk::Align::CENTER);

        title_box->append(*title_lable);
        title_box->append(*subtitle_label);

        m_header_bar.set_title_widget(*title_box);

        setup_custom_buttons();

        // split buttons between left and right so title stays centered
        m_header_bar.pack_start(m_button_open);
        m_header_bar.pack_start(m_button_choose_directory);
        m_header_bar.pack_start(m_button_mode);
        m_header_bar.pack_start(m_button_run);

        m_header_bar.pack_end(m_button_about);
        m_header_bar.pack_end(m_button_settings);
        m_header_bar.pack_end(m_button_save);

        m_header_bar.set_show_title_buttons(true);
        set_titlebar(m_header_bar);
    }

    void setup_custom_buttons() {
        m_button_open.add_css_class("forx-button-open");
        m_button_mode.add_css_class("forx-button-mode");
        m_button_choose_directory.add_css_class("forx-button-dir");
        m_button_run.add_css_class("forx-button-run");
        m_button_save.add_css_class("forx-button-save");
        m_button_settings.add_css_class("forx-button-settings");
        m_button_about.add_css_class("forx-button-about");

        m_button_open.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_open_clicked));
        m_button_choose_directory.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_dir_clicked));
        m_button_mode.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_mode_clicked));
        m_button_run.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_run_clicked));
        m_button_save.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_save_clicked));
        m_button_settings.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_settings_clicked));
        m_button_about.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_about_clicked));
    }

    void setup_main_widget() {
        m_master_view_manager = Gtk::make_managed<Gtk::Stack>();
        m_master_view_manager->set_transition_type(Gtk::StackTransitionType::CROSSFADE);
        m_master_view_manager->set_vexpand(true); 
        m_master_view_manager->set_hexpand(true);

        auto *hex_scroller = Gtk::make_managed<Gtk::ScrolledWindow>();
        m_hex_text_view = Gtk::make_managed<Gtk::TextView>();
        m_hex_text_view->set_monospace(true);
        m_hex_text_view->set_editable(false);
        hex_scroller->set_child(*m_hex_text_view);
        hex_scroller->set_expand(true);

        auto ref_buffer = m_hex_text_view->get_buffer();
        ref_buffer->set_text("Open a file and select a mode, then click Run.");

        m_scan_results_grid = Gtk::make_managed<Gtk::Grid>();
        m_scan_results_grid->set_row_spacing(10);
        m_scan_results_grid->set_column_spacing(15);
        m_scan_results_grid->set_margin(10);
        m_scan_results_grid->set_expand(true);

        m_result_label = Gtk::make_managed<Gtk::Label>("");
        m_result_label->set_selectable(true);
        m_result_label->set_halign(Gtk::Align::START);
        m_result_label->set_valign(Gtk::Align::START);
        m_result_label->set_wrap(true);
        m_scan_results_grid->attach(*m_result_label, 0, 0, 1, 1);

        m_master_view_manager->add(*hex_scroller, "hex_layout");
        m_master_view_manager->add(*m_scan_results_grid, "grid_layout");

        m_master_view_manager->set_visible_child("hex_layout");
    }

    void set_grid_text(const std::string &text) {
        m_result_label->set_text(text);
    }

    Gtk::Box m_main_box;
    Gtk::Label m_label;
    Gtk::Button m_button;
    Gtk::HeaderBar m_header_bar;

    Gtk::Stack* m_master_view_manager = nullptr;
    Gtk::TextView* m_hex_text_view = nullptr;
    Gtk::Grid* m_scan_results_grid = nullptr;
    Gtk::Label *m_result_label = nullptr;
    std::string m_selected_mode = "hex_dump";

    Custom_Button m_button_open{"Open File"};
    Custom_Button m_button_choose_directory{"Choose Directory"};
    Custom_Button m_button_mode{"Mode"};
    Custom_Button m_button_run{"Run"};
    Custom_Button m_button_save{"Save"};
    Custom_Button m_button_settings{"Settings"};
    Custom_Button m_button_about{"About"};

    std::vector<Custom_Button*> buttons = {
        &m_button_open, &m_button_choose_directory, &m_button_mode, &m_button_run, 
        &m_button_save, &m_button_settings, &m_button_about
    };

    CustomWindow m_file_picker_manager;
    CustomWindow m_mode_selector;
    CustomWindow m_about_mode;
    CustomWindow m_save_mode;
    CustomWindow m_settings_mode;

    // for backend and connection to C functions
    std::string capture(std::function<void(FILE *)> file) {
        char *buf = nullptr;
        size_t size = 0;
        FILE *mem = open_memstream(&buf, & size);

        if (!mem) {
            return "error: open_memstream failed";
        }
        file(mem);
        fclose(mem);
        std::string result(buf, size);
        free(buf);
        return result;
    }

    std::string m_loaded_file;

    bool m_dark_mode = false;

    void set_dark_mode(bool dark) {
        m_dark_mode = dark;
        if (dark) {
            add_css_class("dark");
        }
        else {
            remove_css_class("dark");
        }
    }

};

class ForxApp : public Gtk::Application {
public:
    static Glib::RefPtr<ForxApp> create() {
        return Glib::make_refptr_for_instance<ForxApp>(
            new ForxApp()
        );
    }

protected:
    ForxApp() : Gtk::Application("org.example.forx") {}

    void on_activate() override {

        Gtk::Settings::get_default()->property_gtk_decoration_layout() = "close,minimize,maximize:";

        auto provider = Gtk::CssProvider::create();
        provider->load_from_path("style.css");
        Gtk::StyleContext::add_provider_for_display(
            Gdk::Display::get_default(),
            provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );

        auto *window = new ForxWindow();
        add_window(*window);
        window->present();
    }
};

extern "C" {
    void gui_launch(int argc, char **argv) {
        sigdb_load("signature.txt");
        auto app = ForxApp::create();
        app->run(argc, argv);
    }
}

