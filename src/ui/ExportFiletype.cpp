#include "ExportFiletype.h"
#include <fstream>
#include <iostream>
#include <gtkmm.h>

void ExportFiletype::set_content(const std::string &content) {
    m_content = content;
}

void ExportFiletype::dispatch(const std::string &path, const std::string &format) {
    if (format == "txt") {
        export_as_txt(path);
    }
    else if (format == "pdf") {
        export_as_pdf(path);
    }
    else if (format == "png") {
        export_as_png(path);
    }
    else if (format == "csv") {
        export_as_csv(path);
    }
    else {
        std::cerr << "Unknown format: " << format << "\n";
    }
}

void ExportFiletype::export_as_txt(const std::string &path) {
    std::ofstream out(path);
    if (!out) { 
        std::cerr << "Cannot write: " << path << "\n"; 
        return; 
    }
    out << m_content;
    std::cout << "Saved TXT: " << path << "\n";
}

void ExportFiletype::export_as_csv(const std::string &path) {
    std::ofstream out(path);
    if (!out) { 
        std::cerr << "Cannot write: " << path << "\n"; 
        return; 
    }
    out << "offset,hex,ascii\n" << m_content;
    std::cout << "Saved CSV: " << path << "\n";
}

// todo, fix these
void ExportFiletype::export_as_pdf(const std::string &path) {
    auto op = Gtk::PrintOperation::create();
    op->set_export_filename(path);

    std::string content = m_content;

    op->signal_begin_print().connect(
        [op](const Glib::RefPtr<Gtk::PrintContext> &context) {
            (void)context;
            op->set_n_pages(1);
        }
    );

    op->signal_draw_page().connect(
        [content](const Glib::RefPtr<Gtk::PrintContext> &context, int page_nr) {
            (void)page_nr;
            auto cr = context->get_cairo_context();
            cr->set_source_rgb(0.0, 0.0, 0.0);
            cr->select_font_face("Monospace",
                Cairo::ToyFontFace::Slant::NORMAL,
                Cairo::ToyFontFace::Weight::NORMAL);
            cr->set_font_size(11);

            double x = 50.0;
            double y = 50.0;
            double line_height = 14.0;

            std::istringstream stream(content);
            std::string line;
            while (std::getline(stream, line)) {
                cr->move_to(x, y);
                cr->show_text(line);
                y += line_height;
                if (y > 750.0) break;
            }
        }
    );

    auto result = op->run(Gtk::PrintOperation::Action::EXPORT);

    if (result == Gtk::PrintOperation::Result::ERROR)
        std::cerr << "PDF export failed: " << path << "\n";
    else
        std::cout << "PDF saved: " << path << "\n";
}
 
