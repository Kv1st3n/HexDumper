#ifndef GTKMM_CUSTOM_WINDOW_H
#define GTKMM_CUSTOM_WINDOW_H

#include <gtkmm.h>
#include <string>
#include "CustomButton.h"
#include "ExportFiletype.h"

class CustomWindow : public Gtk::Window, protected ExportFiletype {
public:
    CustomWindow();
    ~CustomWindow() = default;

    void show_picker(Gtk::Window& parent_window, const std::string& default_path, std::function<void(const std::string &)> on_file_selected);
    void show_mode_menu(Gtk::Button &parent_button, std::function<void(const std::string &)> on_selected);
    void show_about(Gtk::Window& parent_window);
    Gtk::Grid* create_button_grid_for_about_section();
    void fill_buffer_about(const int& choice);
    void show_save(Gtk::Window &parent_window, const std::string &content);
    void show_file_types(Gtk::Button& parent_button);
    void show_settings(Gtk::Window& parent_window);
    void light_mode();
    void dark_mode();

protected:
    void on_file_dialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog);


private:
    Gtk::PopoverMenu m_mode_popover;
    Gtk::Popover* m_file_type_popover = nullptr;
    Gtk::PopoverMenu m_about_popover;
    Gtk::TextView m_about_text_view;
    Gtk::Grid m_grid;

    Glib::RefPtr<Gtk::TextTagTable> m_ref_text_tag_table;
    Glib::RefPtr<Gtk::TextBuffer> m_ref_text_buffer;

    Custom_Button m_button_about_tool{"About tool"};
    Custom_Button m_button_about_us{"About us"};

    std::string m_selected_folder_path;
    std::string m_selected_format = "pdf";

    void fill_text_tag_table();
    void fill_buffer();

    std::function<void(const std::string &)> m_on_file_selected;
    std::string m_export_content;

    std::string m_sigdb_path = "signature.txt";
};


#endif