#include "CustomButton.h"
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <iostream>
#include <string>

Custom_Button::Custom_Button(const std::string& label) : Gtk::Button(label) {
    add_css_class("forx-button"); // 'flat' if i dont want the button shape

    //set_halign(Gtk::Align::CENTER);
    //set_size_request(-1, -1);

    set_margin_start(6);
    set_margin_end(6);
    set_margin_top(6);
    set_margin_bottom(6);
    set_hexpand(true); // <- can be false

}


// in case a specific button wants some unique scheme
Custom_Button::Custom_Button(const std::string &label_text, const std::string &extra_class) : Gtk::Button(label_text) {
    add_css_class("forx-button"); 
    add_css_class(extra_class);
    set_margin_start(6);
    set_margin_end(6);
    set_margin_top(6);
    set_margin_bottom(6);
}


 
