#include "hcm.h"

#include <cstdlib>  // AI: setenv() для ESCDELAY

using namespace std;

// AI: Методы List/Menu определены в глобальной области видимости, а Key живёт
// AI: в namespace hcm. Это объявление вводит в область видимости ИМЯ типа,
// AI: но не его значения: писать всё равно нужно Key::Up, а не просто Up.
// AI: Так сохраняется типобезопасность enum class и не нужен hcm:: в каждой case-метке.
using hcm::Key;

namespace hcm {

namespace {
// AI: Защита от повторной инициализации: ncurses хранит состояние экрана
// AI: глобально, и два initscr() на один терминал — это ошибка.
bool g_terminal_active = false;
} // namespace

Terminal::Terminal() {
    if (g_terminal_active) {
        return;  // ok_ остаётся false
    }

    // AI: ESCDELAY — сколько ncurses ждёт после одиночного Esc, чтобы понять,
    // AI: это Esc или начало escape-последовательности (стрелка, F-клавиша).
    // AI: По умолчанию 1000 мс — Esc ощущается как «залипший». Ставим 25 мс.
    // AI: ВАЖНО: set_escdelay() есть только в ncurses 6.1+, а на macOS
    // AI: системный ncurses старше, поэтому используем переменную окружения.
    // AI: И задавать её нужно ДО initscr(), иначе ncurses прочитает старое значение.
    // AI: Третий аргумент 0 = не перезаписывать ESCDELAY, если пользователь
    // AI: выставил своё значение в окружении.
    setenv("ESCDELAY", "25", 0);

    // AI: initscr() возвращает nullptr при ошибке (и сам печатает причину).
    if (initscr() == nullptr) {
        return;
    }
    g_terminal_active = true;
    ok_ = true;

    // AI: cbreak() отключает построчную буферизацию ввода: getch() начинает
    // AI: возвращать каждую клавишу сразу, не дожидаясь Enter.
    // AI: На практике ncurses и сам выставляет этот режим при инициализации
    // AI: (проверено через tcgetattr: ICANON 1 -> 0), но man cbreak(3X)
    // AI: говорит, что это НЕ гарантировано, и советует вызывать явно.
    // AI: Строка ниже — страховка для других сборок ncurses и платформ.
    cbreak();

    // AI: noecho() выключает эхо. На уровне tty echo и так выключен самим
    // AI: ncurses, но getch() умеет «дорисовывать» введённые символы сам —
    // AI: именно этим управляет noecho().
    noecho();

    // AI: keypad() заставляет ncurses разбирать escape-последовательности
    // AI: (стрелки, F-клавиши) в константы KEY_UP/KEY_DOWN/... вместо того,
    // AI: чтобы отдавать их по байтам.
    keypad(stdscr, TRUE);

    // AI: Скрываем курсор: в меню он не нужен. Поле ввода будет временно
    // AI: показывать его само (шаг 4).
    curs_set(0);
}

Terminal::~Terminal() {
    if (ok_) {
        // AI: endwin() возвращает терминал в исходный режим: показывает
        // AI: курсор, выходит из alternate screen, восстанавливает termios.
        // AI: Без него после падения/выхода остаётся «сломанный» терминал.
        endwin();
        g_terminal_active = false;
    }
}

void InitTheme() {
    // AI: has_colors() == false на монохромном терминале или TERM=dumb.
    // AI: Тогда просто не трогаем цвета, а не падаем.
    if (!has_colors()) {
        return;
    }

    // AI: start_color() разрешает использование цветов, use_default_colors()
    // AI: позволяет паре использовать «цвет терминала по умолчанию» (-1).
    // AI: Без второго вызова фон всегда был бы одним из 8 базовых цветов.
    start_color();
    use_default_colors();

    // AI: Номера пар перенесены из main.cpp без изменений, чтобы картинка
    // AI: осталась прежней. В P2 они станут именованными (Style/Theme).
    init_pair(1, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_CYAN);
    init_pair(4, COLOR_WHITE, COLOR_GREEN);
    init_pair(5, COLOR_YELLOW, COLOR_WHITE);
    init_pair(6, COLOR_BLACK, COLOR_RED);
    init_pair(7, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(8, COLOR_BLACK, COLOR_MAGENTA);

    // AI: bkgd() задаёт фон всей области stdscr (пара 7 — та же, что была
    // AI: в main.cpp). Должен вызываться после init_pair(7, ...).
    bkgd(COLOR_PAIR(7));
}

} // namespace hcm

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

void List::Input(hcm::Key key) {
    if (opened) {
        switch (key) {
            case Key::Up:
                if (selectedIndex > 0) --selectedIndex;
                break;
            case Key::Down:
                // AI: static_cast<int> убирает предупреждение о сравнении
                // AI: signed/unsigned: size() возвращает size_t.
                // AI: Плюс запись selectedIndex + 1 не уходит в минус при пустом списке.
                if (selectedIndex + 1 < static_cast<int>(w.size())) ++selectedIndex;
                break;
            case Key::Select:
                w[selectedIndex]->OnSelect();
                break;
            case Key::Back:
                opened = false;
                break;
            default:
                break;
        }
    } else {
        if (key == Key::Select) {
            opened = true;
        }
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

void Menu::Input(hcm::Key key) {
    // AI: Quit обрабатываем ДО делегирования списку. Иначе при открытом
    // AI: списке 'q' уйдёт в List::Input и не сработает.
    // AI: endwin()+exit(0) здесь временно: шаг 3 заменит это на Result,
    // AI: чтобы библиотека не завершала процесс сама.
    if (key == Key::Quit) {
        endwin();
        exit(0);
    }

    // AI: Resize не требует действий: главный цикл после Input всё равно
    // AI: вызовет Show(), а тот перерисует экран уже с новыми LINES/COLS.
    if (key == Key::Resize) {
        return;
    }

    if (widgets[selectedIndex] != nullptr) {
        auto* list = dynamic_cast<List*>(widgets[selectedIndex]);
        if (list && list->IsOpened()) {
            list->Input(key);
            return;
        }
    }

    switch (key) {
        case Key::Up:
            if (selectedIndex > 0) --selectedIndex;
            break;
        case Key::Down:
            if (selectedIndex + 1 < static_cast<int>(widgets.size())) ++selectedIndex;
            break;
        case Key::Select:
            widgets[selectedIndex]->OnSelect();
            break;
        case Key::Back:
            // AI: Пока Back = выход, как было с 'x'. В шаге 3 он станет
            // AI: «закрыть меню», а решение о выходе останется в main.
            endwin();
            exit(0);
            break;
        default:
            break;
    }
}

Menu::~Menu() {
    for (auto widget : widgets) delete widget;
}

namespace hcm {

Key ReadKey() {
    // AI: getch() возвращает либо ASCII-код, либо одну из констант KEY_*,
    // AI: если последовательность была разобрана ncurses. Разбор включается
    // AI: вызовом keypad(stdscr, TRUE) в Terminal.
    switch (getch()) {
        // AI: vim-подобные hjkl продублированы стрелками: getch() отдаёт
        // AI: KEY_UP/KEY_DOWN/... только благодаря keypad().
        case 'k': case KEY_UP:    return Key::Up;
        case 'j': case KEY_DOWN:  return Key::Down;
        case 'h': case KEY_LEFT:  return Key::Left;
        case 'l': case KEY_RIGHT: return Key::Right;

        // AI: Enter в разных терминалах приходит как '\n', '\r' или KEY_ENTER,
        // AI: поэтому ловим все три варианта. Пробел — тоже «выбрать».
        case '\n': case '\r': case ' ': case KEY_ENTER:
            return Key::Select;

        // AI: 27 — это одиночный Esc. Он приходит только после ESCDELAY
        // AI: (25 мс, выставлено в Terminal), иначе его не отличить от
        // AI: начала escape-последовательности стрелки.
        // AI: 'x' оставлен как алиас для совместимости с прежним поведением.
        case 27: case 'x':        return Key::Back;

        case 'q':                 return Key::Quit;

        // AI: SIGWINCH от ncurses превращается в KEY_RESIZE.
        case KEY_RESIZE:          return Key::Resize;

        // AI: Ctrl-C (0x03) сюда НЕ доходит: под cbreak() включён ISIG,
        // AI: ядро превращает его в SIGINT, а ncurses сам восстанавливает
        // AI: терминал и завершает процесс (проверено: endwin-последовательность
        // AI: в выводе есть, код возврата 1). Чтобы ловить Ctrl-C как клавишу,
        // AI: нужен raw()-режим — это отдельное решение, не для этого шага.
        default:                  return Key::Unknown;
    }
}

} // namespace hcm