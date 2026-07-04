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

        // dark theme
        //Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() = true;

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
        m_file_picker_manager.show_picker(*this, "", [this](const std::string &path) {
            m_loaded_file = path;
            std::cout << "File loaded: " << path << "\n";
            m_hex_text_view->get_buffer()->set_text(
                "File loaded: " + path + "\nSelect a mode and click Run.");
            m_master_view_manager->set_visible_child("hex_layout");
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
                    mode == "sha512" || mode == "file_identifier" || mode == "directory_scanner") {
                
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

        // ── hex layout modes ─────────────────────────────────────
        if (m_selected_mode == "hex_dump") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                dump_hex(bfile_raw(bfh), f, 0, 0, 0);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        } else if (m_selected_mode == "reverse_mode") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                reverse_dump(bfile_raw(bfh), f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        } else if (m_selected_mode == "string_extractor") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                extract_strings(bfile_raw(bfh), f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        } else if (m_selected_mode == "file_identifier") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                uint8_t buf[16];
                size_t  n = fread(buf, 1, 16, bfile_raw(bfh));
                fprintf(f, "Type: %s\n", sigdb_identify(buf, n));
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        } else if (m_selected_mode == "compact") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                dump_hex(bfile_raw(bfh), f, 0, 1, 0);   // compact = 1
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        } else if (m_selected_mode == "lowercase") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); return; 
                }
                dump_hex(bfile_raw(bfh), f, 1, 0, 0);   // lowercase = 1
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("hex_layout");
            m_hex_text_view->get_buffer()->set_text(out);

        // ── grid layout modes ────────────────────────────────────
        } else if (m_selected_mode == "md5") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); 
                    return; 
                }
                print_checksum(bfile_raw(bfh), 1, f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);

        } else if (m_selected_mode == "sha1") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); 
                    return; 
                }
                print_checksum(bfile_raw(bfh), 2, f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);

        } else if (m_selected_mode == "sha256") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); 
                    return; 
                }
                print_checksum(bfile_raw(bfh), 3, f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);

        } else if (m_selected_mode == "sha512") {
            std::string out = capture([&](FILE *f) {
                BinaryFileHandle *bfh = bfile_open(m_loaded_file.c_str());
                if (!bfh) { 
                    fprintf(f, "error: cannot open file\n"); 
                    return; 
                }
                print_checksum(bfile_raw(bfh), 4, f);
                bfile_close(bfh);
            });
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);

        } else if (m_selected_mode == "directory_scanner") {
            std::string out = capture([&](FILE *f) {
                scan_directory(m_loaded_file.c_str(), f);
            });
            m_master_view_manager->set_visible_child("grid_layout");
            set_grid_text(out);
        }
    }

    void on_save_clicked() {
        std::string current_output;

        if (m_master_view_manager->get_visible_child_name() == "hex_layout") {
            current_output = m_hex_text_view->get_buffer()->get_text();
        } else {
            // grid layout — get the label text
            auto *child = m_scan_results_grid->get_first_child();
            if (child) {
                auto *lbl = dynamic_cast<Gtk::Label *>(child);
                if (lbl) current_output = lbl->get_text();
            }
        }

        m_save_mode.show_save(*this, current_output);
    }

    void on_about_clicked() {
        m_about_mode.show_about(*this);
    }

    void on_settings_clicked() {
        std::cout << "Settings clocked" << std::endl;
        m_settings_mode.show_settings(*this);
    }

private:

    // sets up header
    void setup_custom_header() {
        auto *title_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        auto *title_lable = Gtk::make_managed<Gtk::Label>("<b>forx</b>");
        title_lable -> set_use_markup(true);

        auto *subtitle_label = Gtk::make_managed<Gtk::Label>("Binary analysis tool");
        subtitle_label->set_css_classes({"subtitle"});

        title_box -> append(*title_lable);
        title_box -> append(*subtitle_label);

        m_header_bar.set_title_widget(*title_box);

        setup_custom_buttons();

        // iterates and packs each button
        for (auto &btn : buttons) {
            m_header_bar.pack_start(*btn);
        }

        m_header_bar.set_show_title_buttons(true);

        set_titlebar(m_header_bar);

    }

    void setup_custom_buttons() {
        m_button_open.signal_clicked().connect(sigc::mem_fun(*this, &ForxWindow::on_open_clicked));
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
    Custom_Button m_button_mode{"Mode"};
    Custom_Button m_button_run{"Run"};
    Custom_Button m_button_save{"Save"};
    Custom_Button m_button_settings{"Settings"};
    Custom_Button m_button_about{"About"};

    std::vector<Custom_Button*> buttons = {
        &m_button_open, &m_button_mode, &m_button_run, 
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