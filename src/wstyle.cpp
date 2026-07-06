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

    namespace {
        // UTF-8 한 글자를 디코딩하고 바이트 길이를 반환
        size_t utf8_decode(std::string_view s, size_t i, char32_t& cp) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if (c < 0x80) { cp = c; return 1; }
            if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
                cp = static_cast<char32_t>(c & 0x1F);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
                return 2;
            }
            if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
                cp = static_cast<char32_t>(c & 0x0F);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
                return 3;
            }
            if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
                cp = static_cast<char32_t>(c & 0x07);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
                return 4;
            }
            cp = c; // 잘못된 시퀀스: 1바이트로 취급
            return 1;
        }

        // 동아시아 넓은 문자 범위(한글/CJK/가나 등) 대략적 판별
        bool is_wide(char32_t cp) {
            return (cp >= 0x1100 && cp <= 0x115F) ||
                   (cp >= 0x2E80 && cp <= 0xA4CF && cp != 0x303F) ||
                   (cp >= 0xAC00 && cp <= 0xD7A3) ||
                   (cp >= 0xF900 && cp <= 0xFAFF) ||
                   (cp >= 0xFF00 && cp <= 0xFF60) ||
                   (cp >= 0xFFE0 && cp <= 0xFFE6) ||
                   (cp >= 0x20000 && cp <= 0x3FFFD);
        }
    } // namespace

    size_t display_width(std::string_view text) {
        size_t width = 0;
        size_t i = 0;
        while (i < text.size()) {
            char32_t cp;
            i += utf8_decode(text, i, cp);
            width += is_wide(cp) ? 2 : 1;
        }
        return width;
    }

    std::string pad_display(std::string_view text, size_t width) {
        std::string result(text);
        size_t w = display_width(text);
        if (w < width) result += std::string(width - w, ' ');
        return result;
    }

    // --- Table ---
    void Table::add_column(const std::string& h, const Style& hs, const Style& cs) { columns_.push_back({h, hs, cs, h.size()}); }
    void Table::add_row(const std::vector<std::string>& row) { rows_.push_back(row); }
    void Table::update_widths() {
        for (size_t i = 0; i < columns_.size(); ++i) {
            columns_[i].width = std::max(columns_[i].width, display_width(columns_[i].header));
            for (const auto& row : rows_) if (i < row.size()) columns_[i].width = std::max(columns_[i].width, display_width(row[i]));
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
        for (const auto& col : columns_) std::cout << " " << format(col.header, col.header_style) << std::string(col.width - display_width(col.header), ' ') << " |";
        std::cout << std::endl;
        border();
        for (const auto& row : rows_) {
            std::cout << "|";
            for (size_t i = 0; i < columns_.size(); ++i) {
                std::string text = (i < row.size()) ? row[i] : "";
                std::cout << " " << format(text, columns_[i].cell_style) << std::string(columns_[i].width - display_width(text), ' ') << " |";
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
        std::vector<std::string> lines;
        std::istringstream iss(content);
        std::string line;
        size_t max_content_width = 0;
        while (std::getline(iss, line)) {
            lines.push_back(line);
            max_content_width = std::max(max_content_width, display_width(line));
        }

        if (lines.empty()) {
            lines.push_back("");
        }

        size_t width = std::max(display_width(title) + 2, max_content_width) + 2;

        // 상단 테두리
        std::string top_border = " " + title + " " + std::string(width - display_width(title) - 2, '-');
        std::cout << format("+" + top_border + "+", border_style) << std::endl;

        // 내용 출력
        for (const auto& l : lines) {
            std::cout << format("| ", border_style) << l << std::string(width - display_width(l) - 1, ' ') << format("|", border_style) << std::endl;
        }

        // 하단 테두리
        std::cout << format("+" + std::string(width, '-') + "+", border_style) << std::endl;
    }

} // namespace wcppcli
