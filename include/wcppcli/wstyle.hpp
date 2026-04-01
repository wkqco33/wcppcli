#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace wcppcli {

    enum class Color : int {
        None = -1,
        Black = 0, Red = 1, Green = 2, Yellow = 3, Blue = 4, Magenta = 5, Cyan = 6, White = 7, Default = 9
    };

    struct Style {
        Color fg = Color::None;
        Color bg = Color::None;
        bool bold = false, dim = false, italic = false, underline = false, blink = false, reverse = false, hidden = false, strikethrough = false;
    };

    std::string format(std::string_view text, const Style& style);
    void print(std::string_view text, const Style& style = {});

    // --- Advanced UI Components ---

    struct Column {
        std::string header;
        Style header_style, cell_style;
        size_t width = 0;
    };

    class Table {
        public:
            void add_column(const std::string& header, const Style& header_style = {}, const Style& cell_style = {});
            void add_row(const std::vector<std::string>& row);
            void render() const;
        private:
            std::vector<Column> columns_;
            std::vector<std::vector<std::string>> rows_;
            void update_widths();
    };

    struct ProgressBar {
        std::string label;
        float progress = 0.0f;
        int width = 40;
        Style bar_style = {.fg = Color::Green}, bg_style = {.fg = Color::Black, .bold = true};
        void render() const;
    };

    // 다음 단계: Panel 추가 (고급 레이아웃)
    struct Panel {
        std::string title;
        std::string content;
        Style border_style = {.fg = Color::Blue};
        void render() const;
    };

} // namespace wcppcli
