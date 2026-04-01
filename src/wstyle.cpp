#include "wcppcli/wstyle.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace wcppcli {

    std::string format(std::string_view text, const Style& style) {
        std::vector<int> codes;
        if (style.bold) codes.push_back(1);
        if (style.dim) codes.push_back(2);
        if (style.italic) codes.push_back(3);
        if (style.underline) codes.push_back(4);
        if (style.blink) codes.push_back(5);
        if (style.reverse) codes.push_back(7);
        if (style.hidden) codes.push_back(8);
        if (style.strikethrough) codes.push_back(9);

        if (style.fg != Color::None) codes.push_back(30 + static_cast<int>(style.fg));
        if (style.bg != Color::None) codes.push_back(40 + static_cast<int>(style.bg));

        if (codes.empty()) return std::string(text);
        std::ostringstream oss;
        oss << "\033[";
        for (size_t i = 0; i < codes.size(); ++i) oss << codes[i] << (i == codes.size() - 1 ? "" : ";");
        oss << "m" << text << "\033[0m";
        return oss.str();
    }

    void print(std::string_view text, const Style& style) { std::cout << format(text, style) << std::endl; }

    // --- Table ---
    void Table::add_column(const std::string& h, const Style& hs, const Style& cs) { columns_.push_back({h, hs, cs, h.size()}); }
    void Table::add_row(const std::vector<std::string>& row) { rows_.push_back(row); }
    void Table::update_widths() {
        for (size_t i = 0; i < columns_.size(); ++i) {
            columns_[i].width = std::max(columns_[i].width, columns_[i].header.size());
            for (const auto& row : rows_) if (i < row.size()) columns_[i].width = std::max(columns_[i].width, row[i].size());
        }
    }
    void Table::render() const {
        const_cast<Table*>(this)->update_widths();
        auto border = [&]() {
            std::cout << "+";
            for (const auto& col : columns_) std::cout << std::string(col.width + 2, '-') << "+";
            std::cout << std::endl;
        };
        border();
        std::cout << "|";
        for (const auto& col : columns_) std::cout << " " << format(col.header, col.header_style) << std::string(col.width - col.header.size(), ' ') << " |";
        std::cout << std::endl;
        border();
        for (const auto& row : rows_) {
            std::cout << "|";
            for (size_t i = 0; i < columns_.size(); ++i) {
                std::string text = (i < row.size()) ? row[i] : "";
                std::cout << " " << format(text, columns_[i].cell_style) << std::string(columns_[i].width - text.size(), ' ') << " |";
            }
            std::cout << std::endl;
        }
        border();
    }

    // --- ProgressBar ---
    void ProgressBar::render() const {
        int filled = static_cast<int>(progress * width);
        std::cout << "\r" << label << " [" << format(std::string(filled, '='), bar_style) << format(std::string(width - filled, ' '), bg_style) << "] " << std::fixed << std::setprecision(1) << (progress * 100.0f) << "%" << std::flush;
    }

    // --- Panel ---
    void Panel::render() const {
        size_t width = std::max(title.size(), content.size()) + 4;
        std::string top = " " + title + " " + std::string(width - title.size() - 2, '-');
        std::cout << format("+" + top + "+", border_style) << std::endl;
        std::cout << format("| ", border_style) << content << std::string(width - content.size() - 1, ' ') << format("|", border_style) << std::endl;
        std::cout << format("+" + std::string(width, '-') + "+", border_style) << std::endl;
    }

} // namespace wcppcli
