#include "hcm.h"

using namespace std;

Button::Button(std::string text, std::function<void()> f, int colorPair) : text(text), f(f), colorPair(colorPair) {}

void Button::Show(bool selected, int y) {
    if (selected) attron(COLOR_PAIR(colorPair));
    mvprintw(y, 5, text.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));
}

void Button::OnSelect() {
    f();
}


Label::Label(std::string text, int colorPair) : text(text), colorPair(colorPair) {}

void Label::SetText(std::string newText) {
    text = newText;
}

void Label::Show(bool selected, int y) {
    if (selected) attron(COLOR_PAIR(colorPair));
    mvprintw(y, 5, text.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));
}

CheckBox::CheckBox(std::string text, int colorPair) : text(text), checked(false), colorPair(colorPair) {}

void CheckBox::Show(bool selected, int y) { 
    if (selected) attron(COLOR_PAIR(colorPair)); // Включаем цвет
    mvprintw(y, 5, checked ? "(x) " : "( ) "); // Выводим чекбокс в зависимости от состояния
    mvprintw(y, 9, text.c_str()); // Выводим текст
    if (selected) attroff(COLOR_PAIR(colorPair)); // Выключаем цвет
}

void CheckBox::OnSelect() {
    checked = !checked; // Инвертируем состояние
}

bool CheckBox::IsChecked() const {
    return checked;
}

List::List(std::string text, int colorPair) : text(text), opened(false), colorPair(colorPair) {
    w.push_back(new Button("(X) Close list", [&]() { opened = false; }, 6));
}

void List::AddItem(Widget* item) {
    w.push_back(item);
}

int List::GetSize() const {
    return w.size();
}

void List::Show(bool selected, int y) {
    mvprintw(y, 3, opened ? "v " : "> ");
    if (selected) attron(COLOR_PAIR(colorPair));
    mvprintw(y, 5, text.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));

    if (opened) {
        for (size_t i = 0; i < w.size(); i++) {
            w[i]->Show(i == selectedIndex, y + i + 1);
        }
    }
}

bool List::IsOpened() const {
    return opened;
}

void List::OnSelect() {
    opened = !opened;
}

void List::Input(Key key) {
    if (opened) {
        switch (key) {
            case UP: if (selectedIndex > 0) selectedIndex--; break;
            case DOWN: if (selectedIndex < w.size() - 1) selectedIndex++; break;
            case SELECT: w[selectedIndex]->OnSelect(); break;
            case ESC: opened = false; break;
            default: break;
        }
    } else {
        if (key == SELECT) opened = true;
    }
}

List::~List() {
    for (auto item : w) delete item;
}

Input::Input(std::string placeholder, std::string comment, int colorPair) : text(placeholder), comment(comment), colorPair(colorPair) {}

void Input::Show(bool selected, int y) {
    if (selected) attron(COLOR_PAIR(colorPair));
    mvprintw(y, 5, text.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));
}

void Input::OnSelect() {
    printw("\n%s", comment.c_str());
    echo();
    char buffer[256];
    getstr(buffer);
    text = std::string(buffer);
    noecho();
}

Menu::Menu(std::string header) : header(header) {}

void Menu::Show() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(1, 5, header.c_str());
    attroff(COLOR_PAIR(1));

    int y = 3;
    for (size_t i = 0; i < widgets.size(); i++) {
        widgets[i]->Show(i == selectedIndex, y);
        y++;
        if (dynamic_cast<List*>(widgets[i]) && dynamic_cast<List*>(widgets[i])->IsOpened()) {
            y += dynamic_cast<List*>(widgets[i])->GetSize();
        }
    }
    refresh();
}

void Menu::Input(Key key) {
    if (widgets[selectedIndex] != nullptr) {
        auto* list = dynamic_cast<List*>(widgets[selectedIndex]);
        if (list && list->IsOpened()) {
            list->Input(key);
            return;
        }
    }

    switch (key) {
        case UP: if (selectedIndex > 0) selectedIndex--; break;
        case DOWN: if (selectedIndex < widgets.size() - 1) selectedIndex++; break;
        case SELECT: widgets[selectedIndex]->OnSelect(); break;
        case ESC: endwin(); exit(0);
    }
}

Menu::~Menu() {
    for (auto widget : widgets) delete widget;
}

Key getch_key() { // Функция для получения клавиши
    int ch = getch();
    switch (ch) {
        case 'k': return Key::UP;
        case 'j': return Key::DOWN;
        case '\n': return Key::SELECT;
        case 'x': return Key::ESC;
        default: return Key::UNKNOWN;
    }
}