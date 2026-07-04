#include "CustomWindow.h"
#include "CustomButton.h"
#include "ExportFiletype.h"
#include <iostream>
#include <fstream>
#include <sstream>

CustomWindow::CustomWindow() {}

void CustomWindow::show_picker(Gtk::Window& parent_window, const std::string& default_path, std::function<void(const std::string &)> on_file_selected) {
    std::cout << "CustomWindow class: Launching file selection..." << std::endl;

    m_on_file_selected = on_file_selected;
    
    auto dialog = Gtk::FileDialog::create();

    if (!default_path.empty()) {
        auto file = Gio::File::create_for_path(default_path);
        dialog->set_initial_file(file);
    }

    auto filters = Gio::ListStore<Gtk::FileFilter>::create();
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    filters->append(filter_any);
    dialog->set_filters(filters);

    dialog->open(parent_window, sigc::bind(sigc::mem_fun(*this, &CustomWindow::on_file_dialog_finish), dialog));
}

// shows all the different modes available
void CustomWindow::show_mode_menu(Gtk::Button &parent_button, std::function<void(const std::string &)> on_selected) {
    // always rebuild — ensures callback is never stale
    if (m_mode_popover.get_parent()) {
        m_mode_popover.unparent();
    }

    auto *outer_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 0);

    const std::vector<std::pair<std::string, std::string>> modes = {
        { "hex_dump",          "Hex Dump"          },
        { "reverse_mode",      "Reverse Mode"      },
        { "file_identifier",   "File Identifier"   },
        { "directory_scanner", "Directory Scanner" },
        { "string_extractor",  "String Extractor"  },
        { "md5",               "MD5"               },
        { "sha1",              "SHA1"              },
        { "sha256",            "SHA256"            },
        { "sha512",            "SHA512"            },
    };

    for (const auto &[id, label] : modes) {
        auto *btn = Gtk::make_managed<Gtk::Button>(label);
        std::string mode_id = id;
        btn->signal_clicked().connect([this, mode_id, on_selected]() {
            m_mode_popover.popdown();
            on_selected(mode_id);
        });
        outer_box->append(*btn);
    }

    auto *div = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    outer_box->append(*div);

    const std::vector<std::pair<std::string, std::string>> sub_modes = {
        { "compact",   "Compact"   },
        { "lowercase", "Lowercase" },
    };

    for (const auto &[id, label] : sub_modes) {
        auto *btn = Gtk::make_managed<Gtk::Button>(label);
        std::string mode_id = id;
        btn->signal_clicked().connect([this, mode_id, on_selected]() {
            m_mode_popover.popdown();
            on_selected(mode_id);
        });
        outer_box->append(*btn);
    }

    m_mode_popover.set_child(*outer_box);
    m_mode_popover.set_parent(parent_button);
    m_mode_popover.popup();
}

// shows the about the tool
void CustomWindow::show_about(Gtk::Window& parent_window) {
    std::cout << "CustomWindow class: Launching About" << std::endl;

    auto* about_window = Gtk::make_managed<Gtk::Window>();
    about_window->set_title("About");
    about_window->set_default_size(750, 500);
    about_window->set_resizable(false);
    about_window->set_transient_for(parent_window);
    about_window->set_modal(true);

    // always create a 'master'-box, which then contains all the customizable features
    // due to gtk and its architecture, it is the only way
    auto* master_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    master_box->set_spacing(15); 
    master_box->set_margin(20);

    // specifies the current version of the tool
    auto* app_version = Gtk::make_managed<Gtk::Label>("Version 1.0.0 (Stable)");
    app_version->set_halign(Gtk::Align::START);
    app_version->add_css_class("about-subtitle");
    master_box->append(*app_version);

    // calls the customized functions for the text in about section
    fill_text_tag_table();
    fill_buffer();

    m_about_text_view.set_buffer(m_ref_text_buffer);
    m_about_text_view.set_editable(false);
    m_about_text_view.set_cursor_visible(false);

    auto* visual_divider = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    master_box->append(*visual_divider);

    // two buttons, left and right, left is about the tool, right is about us / license and stuff 
    Gtk::Grid* m_grid = create_button_grid_for_about_section();   

    master_box->append(*m_grid);

    auto* scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrolled_window->set_child(m_about_text_view);
    scrolled_window->set_min_content_width(400);
    scrolled_window->set_min_content_height(250);
    scrolled_window->set_expand(true);
    master_box->append(*scrolled_window);
    
    about_window->set_child(*master_box);
    about_window->present();
} 

// creates a grid of x-amount of buttons for about
Gtk::Grid* CustomWindow::create_button_grid_for_about_section() {
    m_button_about_tool.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &CustomWindow::fill_buffer_about), 1));
    m_button_about_us.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &CustomWindow::fill_buffer_about), 2));

    m_grid.attach(m_button_about_tool, 0, 0);
    m_grid.attach(m_button_about_us, 1, 0);

    return &m_grid;
}

void CustomWindow::fill_buffer_about(const int& choice) {
    if (!m_ref_text_buffer) {
        m_ref_text_buffer = Gtk::TextBuffer::create(m_ref_text_tag_table);
        m_about_text_view.set_buffer(m_ref_text_buffer);
    }
    
    // clear the current text
    m_ref_text_buffer->set_text(""); 
    auto iterable = m_ref_text_buffer->begin();

    // depending on choice, display the appropiate text
    if (choice == 1) {
        // ── overview ─────────────────────────────────────────
        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "forx — Binary Analysis Tool\n"
            "===========================\n\n"
            "forx is a forensics-focused workbench for binary visualisation, "
            "file auditing, and system data analysis. The graphical interface "
            "mirrors the CLI version, exposing every feature through a clean "
            "desktop workspace.\n\n",
            "plain-text");

        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "Features\n"
            "--------\n",
            "plain-text");

        // feature list — one entry per insert for clarity
        const std::vector<std::string> features = {
            "  Hex Viewer       Raw memory offsets, hex values, and ASCII column\n",
            "  Mode Selector    Switch between standard, compact, and lowercase output\n",
            "  File Identifier  Magic byte detection against a runtime signature database\n",
            "  Directory Scan   Recursive file type cataloguing across a directory tree\n",
            "  Checksum Engine  MD5, SHA1, and SHA256 integrity verification\n",
            "  String Extractor Isolates printable ASCII sequences from binary files\n",
        };

        for (const auto &line : features)
            iterable  = m_ref_text_buffer->insert_with_tag(iterable , line, "plain-text");

        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "\nPlatform\n"
            "--------\n"
            "  macOS — compiled with Apple Clang via Xcode Command Line Tools\n"
            "  GTK4 and gtkmm4 for the graphical layer\n",
            "plain-text");

    } else if (choice == 2) {
        // ── build info ───────────────────────────────────────
        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "Build Information\n"
            "=================\n\n"
            "  Version   :  forx 0.1.0\n"
            "  Language  :  C / C++17\n"
            "  Compiler  :  Apple Clang (clang-1700.4.4.1)\n"
            "  UI Layer  :  gtkmm4 — C++ bindings for GTK4\n\n",
            "plain-text");

        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "Dependencies\n"
            "------------\n\n",
            "plain-text");

        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "  # install dependencies\n"
            "  brew install openssl gtk4 gtkmm4\n\n"
            "  # build\n"
            "  make clean && make\n\n"
            "  # run CLI\n"
            "  ./forx -h <file>\n\n"
            "  # run GUI\n"
            "  ./forx --gui\n",
            "script");

        iterable  = m_ref_text_buffer->insert_with_tag(iterable ,
            "\nSource\n"
            "------\n"
            "  https://github.com/Kv1st3n/forx\n",
            "plain-text");

    } else {
        m_ref_text_buffer->insert_with_tag(iterable ,
            "Error: unknown section identifier.\n",
            "plain-text");
    }
}


void CustomWindow::fill_text_tag_table() {
    m_ref_text_tag_table = Gtk::TextTagTable::create();
    auto text_tag = Gtk::TextTag::create("plain-text");
    text_tag->property_wrap_mode() = Gtk::WrapMode::WORD;

    m_ref_text_tag_table->add(text_tag);

    text_tag = Gtk::TextTag::create("script");
    text_tag->property_wrap_mode() = Gtk::WrapMode::NONE;
    text_tag->property_indent() = 20;
    text_tag->property_family() = "Monospace";
    text_tag->property_background() = "#EFEFEF";
    text_tag->property_foreground()  = "#1A1A2E";
    text_tag->property_pixels_above_lines() = 4;
    text_tag->property_pixels_below_lines() = 4;
    m_ref_text_tag_table->add(text_tag);

}

void CustomWindow::fill_buffer() {

    m_ref_text_buffer = Gtk::TextBuffer::create(m_ref_text_tag_table);

    auto iterable = m_ref_text_buffer->begin();

    const std::vector<std::pair<std::string, std::string>> sections = {
        { "forx — Binary Analysis Tool\n\n",              "plain-text" },
        { "A forensics workbench for binary inspection, ",  "plain-text" },
        { "file identification, and system data analysis.\n","plain-text" },
    };

    for (const auto &[text, tag] : sections) {
        iterable = m_ref_text_buffer->insert_with_tag(iterable, text, tag);
    }
}


// shows save menu
void CustomWindow::show_save(Gtk::Window &parent_window, const std::string &content) {
    m_export_content = content;

    auto* save_window = Gtk::make_managed<Gtk::Window>();
    save_window->set_title("Save");
    save_window->set_default_size(600, 250);
    save_window->set_resizable(true);
    save_window->set_transient_for(parent_window);
    save_window->set_modal(true);

    auto* master_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    master_box->set_margin(25); 
    master_box->set_spacing(15);

    auto* save_grid = Gtk::make_managed<Gtk::Grid>();
    save_grid->set_row_spacing(15);   
    save_grid->set_column_spacing(12);
    master_box->append(*save_grid);

    // Declare widgets locally to tie them securely to the save dialogue lifespan
    auto* local_entry = Gtk::make_managed<Gtk::Entry>();
    auto* local_button_file_name = Gtk::make_managed<Gtk::Button>("Submit");
    auto* local_button_file_save = Gtk::make_managed<Gtk::Button>("Save");

    // row 0, file name entry
    auto* type_file_name_label = Gtk::make_managed<Gtk::Label>("Save as:");
    type_file_name_label->set_halign(Gtk::Align::START);

    local_entry->set_placeholder_text("Enter file name (e.g., analysis_dump)...");
    
    local_entry->set_hexpand(true); 
    local_entry->set_halign(Gtk::Align::FILL);

    local_button_file_name->set_halign(Gtk::Align::END);
    local_button_file_name->signal_clicked().connect([local_entry]() {
        std::cout << "User entered: " << local_entry->get_text() << std::endl;
    });

    save_grid->attach(*type_file_name_label, 0, 0, 1, 1);
    save_grid->attach(*local_entry,           1, 0, 1, 1);
    save_grid->attach(*local_button_file_name,   2, 0, 1, 1);

    // row 1, file format choice
    auto* type_label = Gtk::make_managed<Gtk::Label>("Select Export Format:");
    type_label->set_halign(Gtk::Align::START);

    auto* type_button = Gtk::make_managed<Gtk::Button>("Choose Type...");
    
    type_button->set_hexpand(true);
    type_button->set_halign(Gtk::Align::FILL);

    save_grid->attach(*type_label,  0, 1, 1, 1);
    save_grid->attach(*type_button, 1, 1, 1, 1); 

    type_button->signal_clicked().connect([this, type_button]() {
        show_file_types(*type_button);
    });

    // row 2, file path dialog
    auto* path_label = Gtk::make_managed<Gtk::Label>("Destination Folder:");
    path_label->set_halign(Gtk::Align::START);

    auto* chosen_path_label = Gtk::make_managed<Gtk::Label>("No folder selected...");
    chosen_path_label->set_halign(Gtk::Align::START);
    chosen_path_label->set_hexpand(true);

    auto* browse_button = Gtk::make_managed<Gtk::Button>("Browse...");
    browse_button->set_halign(Gtk::Align::END);

    save_grid->attach(*path_label,         0, 2, 1, 1);
    save_grid->attach(*chosen_path_label,  1, 2, 1, 1);
    save_grid->attach(*browse_button,      2, 2, 1, 1);

    browse_button->signal_clicked().connect([this, save_window, chosen_path_label]() {
        auto folder_dialog = Gtk::FileDialog::create();
        folder_dialog->set_title("Select Destination Folder");

        folder_dialog->select_folder(*save_window, [this, folder_dialog, chosen_path_label](const Glib::RefPtr<Gio::AsyncResult>& result) {
            try {
                Glib::RefPtr<Gio::File> folder = folder_dialog->select_folder_finish(result);
                
                m_selected_folder_path = folder->get_path();
                
                chosen_path_label->set_text(m_selected_folder_path);
            }
            catch (const Gtk::DialogError& err) {
                std::cout << "Folder selection canceled by user." << std::endl;
            }
            catch (const Glib::Error& err) {
                std::cerr << "Folder selection core failure: " << err.what() << std::endl;
            }
        });
    });

    // row 3, final save button
    local_button_file_save->set_halign(Gtk::Align::FILL);
    // final function that saves the file
    save_grid->attach(*local_button_file_save, 2, 3, 1, 1);

    local_button_file_save->signal_clicked().connect([this, local_entry, save_window]() {
        std::string filename = local_entry->get_text();

        if (m_selected_folder_path.empty()) {
            std::cerr << "Error: Please choose a destination folder first!" << std::endl;
            return; 
        }
        if (filename.empty()) {
            std::cerr << "Error: Please enter a filename!" << std::endl;
            return;
        }

        std::string final_output_path = m_selected_folder_path + "/" + filename + "." + m_selected_format;
        std::cout << "Executing download/save to: " << final_output_path << std::endl;

        set_content(m_export_content);
        dispatch(final_output_path, m_selected_format);
        
        // Break any active reference link to the dynamic popover before tearing down the frame
        if (this->m_file_type_popover) {
            this->m_file_type_popover->unparent();
            this->m_file_type_popover = nullptr;
        }
        
        save_window->close(); // Close popup when done
    });

    // Handle background window close triggers to keep popover pointers valid
    save_window->signal_close_request().connect([this]() {
        if (this->m_file_type_popover) {
            this->m_file_type_popover->unparent();
            this->m_file_type_popover = nullptr;
        }
        return false; 
    }, false);

    save_window->set_child(*master_box);
    save_window->present();
}

// Shows the different types of file formatting that can be saved as
void CustomWindow::show_file_types(Gtk::Button& parent_button) {
    if (m_file_type_popover) {
        m_file_type_popover->unparent();
    }

    // Assign the new instance to our tracked class member pointer
    m_file_type_popover = Gtk::make_managed<Gtk::Popover>();
    m_file_type_popover->set_parent(parent_button);

    auto* popover_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    popover_box->set_spacing(5);
    popover_box->set_margin(5);

    auto* btn_pdf = Gtk::make_managed<Gtk::Button>("PDF");
    auto* btn_png = Gtk::make_managed<Gtk::Button>("PNG");
    auto* btn_txt = Gtk::make_managed<Gtk::Button>("TXT");

    btn_pdf->signal_clicked().connect([this]() {
        this->m_selected_format = "pdf";
        std::cout << "Format set to: PDF" << std::endl;
        if (this->m_file_type_popover) this->m_file_type_popover->popdown(); 
    });

    btn_png->signal_clicked().connect([this]() {
        this->m_selected_format = "png";
        std::cout << "Format set to: PNG" << std::endl;
        if (this->m_file_type_popover) this->m_file_type_popover->popdown();
    });

    btn_txt->signal_clicked().connect([this]() {
        this->m_selected_format = "txt";
        std::cout << "Format set to: TXT" << std::endl;
        if (this->m_file_type_popover) this->m_file_type_popover->popdown();
    });

    popover_box->append(*btn_pdf);
    popover_box->append(*btn_png);
    popover_box->append(*btn_txt);

    m_file_type_popover->set_child(*popover_box);
    m_file_type_popover->popup();
}

// wip, but general settings menu
void CustomWindow::show_settings(Gtk::Window& parent_window) {
    auto* settings_window = Gtk::make_managed<Gtk::Window>();
    settings_window->set_title("Settings");
    settings_window->set_default_size(900, 700);
    settings_window->set_resizable(true);
    settings_window->set_transient_for(parent_window);
    settings_window->set_modal(true);

    auto* master_window = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    master_window->set_margin(15);
    master_window->set_spacing(15);

    // top will be path and a text view that shows the file / path of chosen sigd.txt
    // below two buttons, one for light mode and one for dark mode
    auto* grid_window = Gtk::make_managed<Gtk::Grid>();
    grid_window->set_row_spacing(10);
    grid_window->set_column_spacing(10);

    auto* path_label = Gtk::make_managed<Gtk::Label>("Choose a file");
    path_label->set_halign(Gtk::Align::START);

    std::string default_path = "/forx/signature.txt";

    auto* file_entry = Gtk::make_managed<Gtk::Entry>();
    file_entry->set_hexpand(true);
    file_entry->set_editable(false);
    file_entry->set_text(default_path);

    auto* browse_button = Gtk::make_managed<Gtk::Button>("Browse...");

    grid_window->attach(*path_label,   0, 0, 3, 1); 
    grid_window->attach(*file_entry,   0, 1, 2, 1); 
    grid_window->attach(*browse_button, 2, 1, 1, 1);

    // todo, make sigdb.txt default
    browse_button->signal_clicked().connect([this, settings_window, file_entry, default_path]() {
        show_picker(*settings_window, default_path, [file_entry](const std::string& path) {
            file_entry->set_text(path); 
            std::cout << "Filepath " << path << std::endl;
        });
    });

    auto* save_path_button = Gtk::make_managed<Gtk::Button>("Save");
    save_path_button->set_halign(Gtk::Align::FILL);
    grid_window->attach(*save_path_button, 0, 2, 1, 1);

    save_path_button->signal_clicked().connect([file_entry, settings_window]() {
        std::string filename = file_entry->get_text();

        if (filename.empty()) {
            std::cerr << "Error: Please enter a filename!" << std::endl;
            return;
        }

        std::cout << "Saving configuration with filename/path: " << filename << std::endl;
        
        settings_window->close();
    });
    
    // light and dark mode here
    auto* theme_label = Gtk::make_managed<Gtk::Label>("Choose theme");
    theme_label->set_halign(Gtk::Align::START);

    auto* light_button = Gtk::make_managed<Gtk::Button>("Light Mode");
    light_button->signal_clicked().connect(sigc::mem_fun(*this, &CustomWindow::light_mode));

    auto* dark_button = Gtk::make_managed<Gtk::Button>("Dark Mode");
    dark_button->signal_clicked().connect(sigc::mem_fun(*this, &CustomWindow::dark_mode));

    grid_window->attach(*theme_label,  0, 3, 3, 1);
    grid_window->attach(*light_button, 0, 4, 1, 1);
    grid_window->attach(*dark_button,  1, 4, 1, 1);

    master_window->append(*grid_window);
    settings_window->set_child(*master_window);
    settings_window->present();


    // path for sigdb.txt being a choice

}

void CustomWindow::light_mode() {
    Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() = false;
}

void CustomWindow::dark_mode() {
    Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() = true;
}

void CustomWindow::on_file_dialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        auto file = dialog->open_finish(result);
        std::string filename = file->get_path();
        
        std::cout << "CustomWindow successfully caught file path: " << filename << std::endl;
        
        if (m_on_file_selected)
            m_on_file_selected(filename);
    } catch (const Gtk::DialogError& err) {
        std::cout << "CustomWindow: User cancelled choice. " << err.what() << std::endl;
    } catch (const Glib::Error& err) {
        std::cout << "CustomWindow: Unexpected exception. " << err.what() << std::endl;
    }
}

// settings, light and dark mode

// maybe a function (for each) that does all of the set, append, etc
// maybe make a central function, that returns window, and in the function it does all the necessary setups like title, 
// resizable, etc

