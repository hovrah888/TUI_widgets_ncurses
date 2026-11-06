// clang++ -std=c++17 main.cpp hcm.cpp -o main -lncurses                компиляция
// AI: -std=c++17 обязателен: TextField::ParseInt использует std::from_chars
// AI: (появился в C++17). При -std=c++11/14 компилятор пишет, что
// AI: std::from_chars и std::from_chars_result не существуют.
// AI: -ltinfo на macOS не нужен: отдельной библиотеки tinfo там нет,
// AI: достаточно -lncurses.
#include <iostream>
#include "hcm.h"

using namespace std;

void Execute() {
    mvprintw(15, 5, "Some button pressed...");
    refresh();
    napms(1000);
}

int func(int a, int b) {
    return a + b;
}

int main() {
    // AI: Вся инициализация терминала теперь в одном месте.
    // AI: term создаётся раньше виджетов, поэтому разрушится последним:
    // AI: endwin() произойдёт уже после удаления всех Menu/Widget.
    hcm::Terminal term;
    if (!term.Valid()) {
        return 1;
    }
    hcm::InitTheme();

    Menu root(" MENU ");
    Menu help("HELP MENU");

    Menu* currentMenu = &root;

    // AI: Флаг жизни приложения. Раньше его роль играл exit(0) внутри
    // AI: библиотеки; теперь цикл завершается естественно, и все деструкторы
    // AI: (Menu, Widget, Terminal) успевают выполниться.
    bool running = true;

    TextField* input1 = new TextField("Enter 1st number", "Please enter number: ", 3);
    TextField* input2 = new TextField("Enter 2nd number", "Please enter number: ", 3);
    Label* resultLabel = new Label("Result: ", 5);

    Button* calculateButton = new Button("Calculate", [&]() {
        // AI: ParseInt() возвращает 0 для пустого или некорректного ввода.
        // AI: Раньше здесь был stoi(text), который на placeholder-строке
        // AI: "Enter 1st number" бросал std::invalid_argument.
        int a = input1->ParseInt();
        int b = input2->ParseInt();
        resultLabel->SetText("Result: " + to_string(func(a, b)));
    }, 2);


    root.widgets.push_back(new Button("Press for help", [&]() { currentMenu = &help; }, 8));
    root.widgets.push_back(new Button("Start", Execute, 2));
    root.widgets.push_back(new TextField("Enter something", "Please enter something: ", 3));
    root.widgets.push_back(new CheckBox("Check me", 4));

    List* lst = new List("List", 4);
    lst->AddItem(new Label("1 item", 5));
    lst->AddItem(new Button("2 item", Execute, 4));
    lst->AddItem(new Label("3 item", 5));

    root.widgets.push_back(lst);
    // AI: Кнопка больше не завершает процесс сама — она просто опускает флаг,
    // AI: а выйти из цикла и вызвать деструкторы будет main.
    root.widgets.push_back(new Button("Exit", [&]() { running = false; }, 6));

    help.widgets.push_back(new Label("Use UP/DOWN to navigate", 5));
    help.widgets.push_back(new Label("Enter to select", 5));
    // AI: Было "X to exit", но теперь Back на этом экране возвращает в корень.
    help.widgets.push_back(new Label("X to go back", 5));
    help.widgets.push_back(new Button("Back", [&]() { currentMenu = &root; }, 6));


    // AI: ReadKey() читает ровно одно нажатие, HandleKey() его обрабатывает
    // AI: и возвращает Result. Дальше решает приложение:
    // AI:  - Quit (q)   — выйти из приложения из любого меню;
    // AI:  - Close (Esc) — закрыть текущее меню: help -> root, root -> выход;
    // AI:  - Continue     — просто перерисовать кадр.
    // AI: Порядок важен: сначала рисуем кадр, потом ждём клавишу — поэтому
    // AI: после каждого ввода экран перерисовывается с новым выделением.
    while (running) {
        currentMenu->Show();
        switch (currentMenu->HandleKey(hcm::ReadKey())) {
            case hcm::Result::Quit:
                running = false;
                break;
            case hcm::Result::Close:
                // AI: Политика «Esc на корне = выход» живёт здесь, в приложении,
                // AI: а не в библиотеке. Другой проект сможет решить иначе.
                if (currentMenu == &help) {
                    currentMenu = &root;
                } else {
                    running = false;
                }
                break;
            case hcm::Result::Continue:
                break;
        }
    }

    // AI: endwin() здесь больше не нужен: его вызывает ~Terminal().
    // AI: Деструктор сработает и при обычном выходе из main, и при
    // AI: размотке стека из-за исключения.
    return 0;
}
