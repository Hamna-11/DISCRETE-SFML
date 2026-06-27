#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <algorithm>

// ─────────────────────────────────────────────
//  Lightweight GUI primitives for SFML
// ─────────────────────────────────────────────

// Colour palette
namespace Colors {
    const sf::Color BG{ 15,  20,  35 };
    const sf::Color PANEL{ 25,  32,  55 };
    const sf::Color ACCENT{ 70,  130, 220 };
    const sf::Color ACCENT2{ 100, 200, 130 };
    const sf::Color WARN{ 220, 90,  70 };
    const sf::Color TEXT{ 220, 225, 240 };
    const sf::Color TEXT_DIM{ 130, 140, 160 };
    const sf::Color BTN{ 40,  60,  110 };
    const sf::Color BTN_HOV{ 60,  90,  160 };
    const sf::Color BTN_SEL{ 70,  130, 220 };
    const sf::Color OK{ 80,  190, 120 };
    const sf::Color FAIL{ 220, 80,  80 };
    const sf::Color HEADER_BG{ 20,  28,  50 };
    const sf::Color SIDEBAR{ 18,  24,  44 };
}

// ── Button ────────────────────────────────────
struct Button {
    sf::RectangleShape rect;
    sf::Text           label;
    bool               hovered = false;
    bool               selected = false;

    void draw(sf::RenderWindow& win) const {
        win.draw(rect);
        win.draw(label);
    }
    bool contains(sf::Vector2f p) const { return rect.getGlobalBounds().contains(p); }
};

// ── ScrollArea ────────────────────────────────
class ScrollArea {
public:
    sf::View    view;
    float       totalHeight = 0;
    float       scroll = 0;
    sf::FloatRect bounds;

    void setViewport(sf::RenderWindow& win, sf::FloatRect rect) {
        bounds = rect;
        auto ws = win.getSize();
        view.setViewport({ rect.left / ws.x, rect.top / ws.y,
                          rect.width / ws.x, rect.height / ws.y });
        view.setSize(rect.width, rect.height);
        view.setCenter(rect.width / 2.f, rect.height / 2.f + scroll);
    }
    void scrollBy(float delta) {
        scroll = std::max(0.f, std::min(scroll + delta, std::max(0.f, totalHeight - bounds.height)));
        view.setCenter(bounds.width / 2.f, bounds.height / 2.f + scroll);
    }
    bool containsMouse(sf::Vector2i mp) const {
        return bounds.contains((float)mp.x, (float)mp.y);
    }
};

// ── InputBox ─────────────────────────────────
struct InputBox {
    sf::RectangleShape rect;
    sf::Text           text;
    sf::Text           placeholder;
    std::string        value;
    bool               focused = false;
    std::string        ph;

    void handleEvent(const sf::Event& ev) {
        if (!focused) return;
        if (ev.type == sf::Event::TextEntered) {
            auto ch = ev.text.unicode;
            if (ch == 8 && !value.empty()) value.pop_back();
            else if (ch >= 32 && ch < 127) value += (char)ch;
            text.setString(value);
        }
    }
    void draw(sf::RenderWindow& win) {
        rect.setOutlineColor(focused ? Colors::ACCENT : Colors::TEXT_DIM);
        win.draw(rect);
        if (value.empty() && !focused) win.draw(placeholder);
        else win.draw(text);
    }
    bool contains(sf::Vector2f p) const { return rect.getGlobalBounds().contains(p); }
};

// ── Helper functions ──────────────────────────
inline sf::Text makeText(const std::string& str, const sf::Font& font,
    unsigned size, sf::Color col, sf::Vector2f pos) {
    sf::Text t(str, font, size);
    t.setFillColor(col);
    t.setPosition(pos);
    return t;
}

inline Button makeButton(const std::string& label, const sf::Font& font,
    sf::FloatRect bounds, unsigned charSize = 14) {
    Button b;
    b.rect.setPosition(bounds.left, bounds.top);
    b.rect.setSize({ bounds.width, bounds.height });
    b.rect.setFillColor(Colors::BTN);
    b.rect.setOutlineColor(Colors::ACCENT);
    b.rect.setOutlineThickness(1);

    b.label = sf::Text(label, font, charSize);
    b.label.setFillColor(Colors::TEXT);
    // Centre text inside button
    auto tb = b.label.getLocalBounds();
    b.label.setPosition(bounds.left + (bounds.width - tb.width) / 2.f - tb.left,
        bounds.top + (bounds.height - tb.height) / 2.f - tb.top);
    return b;
}

inline void updateButton(Button& b, sf::Vector2f mouse) {
    b.hovered = b.contains(mouse);
    b.rect.setFillColor(b.selected ? Colors::BTN_SEL :
        b.hovered ? Colors::BTN_HOV : Colors::BTN);
}

inline InputBox makeInputBox(const std::string& ph, const sf::Font& font,
    sf::FloatRect bounds, unsigned charSize = 14) {
    InputBox ib;
    ib.ph = ph;
    ib.rect.setPosition(bounds.left, bounds.top);
    ib.rect.setSize({ bounds.width, bounds.height });
    ib.rect.setFillColor(Colors::PANEL);
    ib.rect.setOutlineThickness(1.5f);

    ib.text = sf::Text("", font, charSize);
    ib.text.setFillColor(Colors::TEXT);
    ib.text.setPosition(bounds.left + 6, bounds.top + (bounds.height - charSize) / 2.f);

    ib.placeholder = sf::Text(ph, font, charSize);
    ib.placeholder.setFillColor(Colors::TEXT_DIM);
    ib.placeholder.setPosition(bounds.left + 6, bounds.top + (bounds.height - charSize) / 2.f);
    return ib;
}

// Draw a rounded-ish panel (rectangle with subtle outline)
inline void drawPanel(sf::RenderWindow& win, sf::FloatRect r,
    sf::Color fill = Colors::PANEL,
    sf::Color outline = Colors::ACCENT, float ot = 1.f) {
    sf::RectangleShape rect({ r.width, r.height });
    rect.setPosition(r.left, r.top);
    rect.setFillColor(fill);
    rect.setOutlineColor(outline);
    rect.setOutlineThickness(ot);
    win.draw(rect);
}

// Wrap text into lines of maxWidth pixels
inline std::vector<std::string> wrapText(const std::string& s, const sf::Font& font,
    unsigned charSize, float maxWidth)
{
    std::vector<std::string> lines;
    std::istringstream stream(s);
    std::string word, line;
    sf::Text t("", font, charSize);
    while (stream >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        t.setString(test);
        if (!line.empty() && t.getLocalBounds().width > maxWidth) {
            lines.push_back(line);
            line = word;
        }
        else {
            line = test;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}#pragma once
