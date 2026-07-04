#ifndef EXPORT_FILETYPE_H
#define EXPORT_FILETYPE_H

#include <string>

class ExportFiletype {
public:
    ExportFiletype()  = default;
    ~ExportFiletype() = default;

    void set_content(const std::string &content);
    void dispatch(const std::string &path, const std::string &format);

protected:
    void export_as_txt(const std::string &path);
    void export_as_pdf(const std::string &path);
    void export_as_csv(const std::string &path);

private:
    std::string m_content;
};

#endif