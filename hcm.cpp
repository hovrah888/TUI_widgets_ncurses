#include "hcm.h"

#include <algorithm> // AI: std::remove_if для фильтрации управляющих символов
#include <charconv>  // AI: std::from_chars в TextField::ParseInt
#include <cstddef>   // AI: std::size_t
#include <cstdlib>   // AI: setenv() для ESCDELAY

// AI: Методы List/Menu определены в глобальной области видимости, а Key живёт
// AI: в namespace hcm. Это объявление вводит в область видимости ИМЯ типа,
// AI: но не его значения: писать всё равно нужно Key::Up, а не просто Up.
// AI: Так сохраняется типобезопасность enum class и не нужен hcm:: в каждой case-метке.
using hcm::Key;
using hcm::Result;

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
    // AI: size() возвращает std::size_t; приводим явно, чтобы не было
    // AI: неявного сужения до int.
    return static_cast<int>(w.size());
}

void List::Show(bool selected, int y) {
    mvprintw(y, 3, opened ? "v " : "> ");
    if (selected) attron(COLOR_PAIR(colorPair));
    mvprintw(y, 5, text.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));

    if (opened) {
        // AI: i имеет тип std::size_t, а selectedIndex и y — int. Приводим
        // AI: индекс к int один раз, иначе -Wsign-compare ругается на
        // AI: сравнение беззнакового со знаковым.
        for (std::size_t i = 0; i < w.size(); ++i) {
            const int row = y + static_cast<int>(i) + 1;
            w[i]->Show(static_cast<int>(i) == selectedIndex, row);
        }
    }
}

bool List::IsOpened() const {
    return opened;
}

void List::OnSelect() {
    opened = !opened;
}

void List::HandleKey(hcm::Key key) {
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

TextField::TextField(std::string placeholder, std::string comment, int colorPair, int maxLength)
    : placeholder(placeholder), comment(comment), colorPair(colorPair), maxLength(maxLength) {}

void TextField::Show(bool selected, int y) {
    // AI: запоминаем позицию: приглашение при вводе выводится строкой ниже.
    lastY = y;

    if (selected) attron(COLOR_PAIR(colorPair));
    // AI: пустое поле показывает placeholder, заполненное — само значение.
    // AI: Формат "%s" обязателен: в тексте пользователя может встретиться '%'.
    mvprintw(y, 5, "%s", Empty() ? placeholder.c_str() : value.c_str());
    if (selected) attroff(COLOR_PAIR(colorPair));
}

void TextField::OnSelect() {
    // AI: Приглашение печатаем под полем и предварительно чистим строку,
    // AI: чтобы не осталось хвостов от предыдущего ввода.
    move(lastY + 1, 5);
    clrtoeol();
    printw("%s", comment.c_str());
    refresh();

    // AI: На время ввода возвращаем эхо и курсор: иначе не видно, что печатаешь.
    echo();
    curs_set(1);

    // AI: getnstr() читает не больше maxLength символов. Старая getstr()
    // AI: границы не знала и могла переполнить буфер.
    // AI: vector, а не char[256]: длина задаётся в рантайме, +1 под '\0'.
    std::vector<char> buffer(maxLength + 1, '\0');
    // AI: getnstr() возвращает ERR, если ввод не удался (например, EOF).
    // AI: В этом случае оставляем прежнее значение, а не затираем его пустым.
    if (getnstr(buffer.data(), maxLength) != ERR) {
        // AI: buffer.data() — обычная C-строка: getnstr сам ставит '\0' в конце.
        value.assign(buffer.data());

        // AI: getnstr не трактует Esc как отмену: он кладёт в строку байт 27.
        // AI: Вырезаем управляющие символы, чтобы в значении не оставался мусор,
        // AI: который невозможно увидеть и который ломает ParseInt.
        value.erase(std::remove_if(value.begin(), value.end(),
                                   [](unsigned char c) { return c < 32 || c == 127; }),
                    value.end());
    }

    // AI: Возвращаем режим меню: без эха и без курсора.
    curs_set(0);
    noecho();
}

const std::string& TextField::GetValue() const {
    return value;
}

bool TextField::Empty() const {
    return value.empty();
}

int TextField::ParseInt(int fallback) const {
    // AI: std::from_chars (C++17) не бросает исключений и не зависит от локали,
    // AI: в отличие от std::stoi. Пустая строка и "12abc" дают fallback,
    // AI: а не падение процесса.
    if (value.empty()) {
        return fallback;
    }

    int result = 0;
    const char* first = value.data();
    const char* last = first + value.size();
    const std::from_chars_result parsed = std::from_chars(first, last, result);

    // AI: ptr должен дойти до конца: иначе были лишние символы.
    if (parsed.ec != std::errc() || parsed.ptr != last) {
        return fallback;
    }
    return result;
}

Menu::Menu(std::string header) : header(header) {}

void Menu::Show() {
    // AI: erase() вместо clear(): clear() дополнительно заставляет терминал
    // AI: стереть весь экран (escape-последовательность), из-за чего каждый
    // AI: кадр мог мигать. erase() просто заполняет буфер окна пробелами, а
    // AI: refresh() перерисовывает только изменившиеся места.
    erase();
    attron(COLOR_PAIR(1));
    mvprintw(1, 5, header.c_str());
    attroff(COLOR_PAIR(1));

    int y = 3;
    // AI: Тот же приём, что в List::Show: std::size_t приводим к int,
    // AI: чтобы сравнение с selectedIndex не было знаковым/беззнаковым.
    for (std::size_t i = 0; i < widgets.size(); ++i) {
        widgets[i]->Show(static_cast<int>(i) == selectedIndex, y);
        y++;
        if (dynamic_cast<List*>(widgets[i]) && dynamic_cast<List*>(widgets[i])->IsOpened()) {
            y += dynamic_cast<List*>(widgets[i])->GetSize();
        }
    }
    refresh();
}

Result Menu::HandleKey(hcm::Key key) {
    // AI: Quit обрабатываем ДО делегирования списку: иначе при открытом
    // AI: списке 'q' уйдёт в List::HandleKey и не сработает.
    if (key == Key::Quit) {
        return Result::Quit;
    }

    // AI: Resize не требует действий: главный цикл после HandleKey всё равно
    // AI: вызовет Show(), а тот перерисует экран уже с новыми LINES/COLS.
    if (key == Key::Resize) {
        return Result::Continue;
    }

    // AI: Пустое меню — не повод для UB: widgets[selectedIndex] читать нельзя.
    // AI: Esc на пустом меню означает «закрыть его».
    if (widgets.empty()) {
        return key == Key::Back ? Result::Close : Result::Continue;
    }

    // AI: widgets и selectedIndex публичные, поэтому их могли изменить снаружи.
    // AI: Подрезаем индекс, чтобы widgets[selectedIndex] был корректен.
    if (selectedIndex < 0) {
        selectedIndex = 0;
    } else if (selectedIndex >= static_cast<int>(widgets.size())) {
        selectedIndex = static_cast<int>(widgets.size()) - 1;
    }

    if (widgets[selectedIndex] != nullptr) {
        auto* list = dynamic_cast<List*>(widgets[selectedIndex]);
        if (list && list->IsOpened()) {
            list->HandleKey(key);
            return Result::Continue;
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
            // AI: Здесь больше нет endwin()/exit(0): библиотека не решает,
            // AI: что значит «назад» для конкретного приложения.
            return Result::Close;
        default:
            break;
    }
    return Result::Continue;
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
