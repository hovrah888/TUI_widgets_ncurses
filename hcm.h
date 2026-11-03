#pragma once
#ifndef HCM_H
#define HCM_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <string>

// AI: enum class вместо обычного enum:
// AI:  - значения не протекают в глобальную область видимости (Key::Up, а не UP);
// AI:  - компилятор не позволит случайно сравнить Key с числом или с другим enum.
namespace hcm {

enum class Key {
    None,       // AI: getch() вернул ERR. Пока не используется (нет timeout/nodelay).
    Up,
    Down,
    Left,
    Right,
    Select,     // AI: Enter или пробел
    Back,       // AI: Esc или 'x' — «назад» на уровень выше
    Quit,       // AI: 'q' — выйти из приложения
    Resize,     // AI: KEY_RESIZE — терминал изменил размер
    Unknown     // AI: всё, что не назначено
};

} // namespace hcm

struct Widget {
    virtual void Show(bool selected = false, int y = 0) = 0;
    virtual void OnSelect() {};
    virtual ~Widget() = default;
};

class Button : public Widget {
private:
    std::string text;
    std::function<void()> f;
    int colorPair;
public:
    Button(std::string text, std::function<void()> f, int colorPair);
    void Show(bool selected = false, int y = 0) override;
    void OnSelect() override;
};

class Label : public Widget {
private:
    std::string text;
    int colorPair;
public:
    Label(std::string text, int colorPair);
    void SetText(std::string newText);
    void Show(bool selected = false, int y = 0) override;
};

class CheckBox : public Widget {
private:
    std::string text;
    bool checked;
    int colorPair;
public:
    CheckBox(std::string text, int colorPair);
    void Show(bool selected = false, int y = 0) override;
    void OnSelect() override;
    bool IsChecked() const;
};

class List : public Widget {
private:
    std::string text;
    bool opened;
    std::vector<Widget*> w;
    int selectedIndex = 0;
    int colorPair;
public:
    List(std::string text, int colorPair);
    void AddItem(Widget* item);
    int GetSize() const;
    void Show(bool selected = false, int y = 0) override;
    bool IsOpened() const;
    void OnSelect() override;
    void Input(hcm::Key key);
    ~List();
};

class Input : public Widget {
private:
    std::string comment;
    int colorPair;
public:
    std::string text;
    Input(std::string placeholder, std::string comment, int colorPair);
    void Show(bool selected = false, int y = 0) override;
    void OnSelect() override;
};

class Menu {
private:
    std::string header;
public:
    std::vector<Widget*> widgets;
    int selectedIndex = 0;
    Menu(std::string header);
    void Show();
    void Input(hcm::Key key);
    ~Menu();
};

namespace hcm {

// AI: RAII-обёртка над жизненным циклом ncurses.
// AI: Создаётся в начале main, разрушается при выходе из области видимости.
// AI: Деструктор вызывает endwin(), поэтому терминал восстанавливается сам,
// AI: и в main больше не нужен ручной endwin() в конце.
class Terminal {
public:
    Terminal();
    ~Terminal();

    // AI: Копировать/перемещать нельзя: два объекта Terminal означали бы
    // AI: два initscr()/endwin() на один и тот же терминал.
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    Terminal(Terminal&&) = delete;
    Terminal& operator=(Terminal&&) = delete;

    // AI: false, если initscr() не смог инициализировать экран
    // AI: (например, TERM не задан или stdin не терминал).
    bool Valid() const { return ok_; }

private:
    bool ok_ = false;
};

// AI: Единая точка инициализации цветов. Раньше init_pair(1..8) и bkgd()
// AI: копировались в каждый пример, и библиотека неявно требовала, чтобы
// AI: хост инициализировал пары 1..8 в определённом порядке.
void InitTheme();

// AI: Читает одно нажатие и переводит его в Key. Заменяет старую getch_key().
// AI: Внутри используется getch(), поэтому функция должна вызываться только
// AI: после успешной инициализации Terminal.
Key ReadKey();

} // namespace hcm

#endif // HCM_H
