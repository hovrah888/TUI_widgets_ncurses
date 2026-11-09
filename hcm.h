#pragma once

#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>

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

// AI: Результат обработки клавиши меню. Библиотека сообщает, ЧТО случилось,
// AI: а решение (выйти из приложения или вернуться на предыдущий экран)
// AI: принимает вызывающий код. Раньше меню само вызывало exit(0),
// AI: из-за чего библиотеку нельзя было встроить в чужое приложение.
enum class Result {
    Continue,   // AI: меню продолжает работу
    Close,      // AI: пользователь просит закрыть это меню (Esc)
    Quit        // AI: пользователь просит выйти из приложения (q)
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
    void HandleKey(hcm::Key key);
    ~List();

    // AI: List владеет Widget* и удаляет их в деструкторе. Копирование
    // AI: привело бы к double free (два деструктора удаляют одни и те же
    // AI: указатели), поэтому запрещаем и copy, и move.
    List(const List&) = delete;
    List& operator=(const List&) = delete;
    List(List&&) = delete;
    List& operator=(List&&) = delete;
};

// AI: Input переименован в TextField: «Input» слишком общее имя и путается
// AI: с ключевым словом в других API. Класс — это именно текстовое поле.
class TextField : public Widget {
private:
    // AI: placeholder — подсказка, видимая, пока поле пустое. Раньше она
    // AI: хранилась в том же поле text, что и ввод, поэтому stoi() пытался
    // AI: распарсить "Enter 1st number" и бросал исключение.
    std::string placeholder;
    // AI: value — только то, что реально ввёл пользователь (может быть пустым).
    std::string value;
    std::string comment;
    int colorPair;
    int maxLength;
    // AI: Строка, на которой поле нарисовано в последний раз. Нужна, чтобы
    // AI: OnSelect() знал, куда печатать приглашение к вводу.
    int lastY = 0;
public:
    TextField(std::string placeholder, std::string comment, int colorPair, int maxLength = 64);
    void Show(bool selected = false, int y = 0) override;
    void OnSelect() override;
    const std::string& GetValue() const;
    bool Empty() const;
    // AI: Безопасное преобразование в int: при пустом или некорректном вводе
    // AI: возвращает fallback, а не бросает исключение, как std::stoi.
    int ParseInt(int fallback = 0) const;
};

class Menu {
private:
    std::string header;
public:
    std::vector<Widget*> widgets;
    int selectedIndex = 0;
    Menu(std::string header);
    void Show();
    // AI: Возвращает Result вместо того, чтобы завершать процесс самому.
    hcm::Result HandleKey(hcm::Key key);
    ~Menu();

    // AI: Menu тоже владеет виджетами и удаляет их в деструкторе.
    // AI: Копия означала бы два delete на одни и те же Widget*.
    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;
    Menu(Menu&&) = delete;
    Menu& operator=(Menu&&) = delete;
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
