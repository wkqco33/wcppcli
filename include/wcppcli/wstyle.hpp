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

        Style(Color f = Color::None, Color b = Color::None, bool bld = false)
            : fg(f), bg(b), bold(bld) {}
    };

    std::string format(std::string_view text, const Style& style);
    void print(std::string_view text, const Style& style = Style());

    // UTF-8 문자열의 터미널 표시 폭 계산 (한글/CJK 등 넓은 문자는 2칸으로 계산).
    // 정렬/패딩 계산 시 std::string::size() 대신 사용해야 함 (바이트 수 != 표시 폭).
    size_t display_width(std::string_view text);

    // text 를 display_width 기준 width 까지 공백으로 채운 문자열 반환
    std::string pad_display(std::string_view text, size_t width);

    // --- Advanced UI Components ---

    struct Column {
        std::string header;
        Style header_style, cell_style;
        size_t width = 0;
    };

    class Table {
        public:
            void add_column(const std::string& header, const Style& header_style = Style(), const Style& cell_style = Style());
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
        Style bar_style = Style(Color::Green), bg_style = Style(Color::Black, Color::None, true);
        void render() const;
    };

    // 다음 단계: Panel 추가 (고급 레이아웃)
    struct Panel {
        std::string title;
        std::string content;
        Style border_style = Style(Color::Blue);
        void render() const;
    };

} // namespace wcppcli
