#pragma once
#ifndef HCM_H
#define HCM_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <string>

enum Key { UP, DOWN, ESC, SELECT, UNKNOWN };

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
    void Input(Key key);
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
    void Input(Key key);
    ~Menu();
};

Key getch_key();

#endif // HCM_H
