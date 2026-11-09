#include <string>
#include "hcm.h"

int sum(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

int mul(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    return a / b;
}


int main(){
    // AI: Единая инициализация вместо копипасты. Обрати внимание: палитра
    // AI: здесь теперь та же, что у demo (InitTheme общая), поэтому пары 1
    // AI: и 5 визуально немного отличаются от прежней версии этого примера.
    hcm::Terminal term;
    if (!term.Valid()) {
        return 1;
    }
    hcm::InitTheme();

    // AI: Флаг вместо exit(0) внутри библиотеки: цикл завершится сам,
    // AI: и деструкторы Menu/Widget/Terminal выполнятся в обычном порядке.
    bool running = true;

    Menu calc(" CALCULATOR ");

    // AI: Держим указатели на поля и Label отдельно, чтобы не доставать их
    // AI: из calc.widgets по индексу и не кастовать вручную в каждой лямбде.
    TextField* first = new TextField("Enter 1st number", "Please enter number: ", 3);
    TextField* second = new TextField("Enter 2nd number", "Please enter number: ", 3);
    Label* result = new Label("Result: ", 5);

    calc.widgets.push_back(first);
    calc.widgets.push_back(second);
    calc.widgets.push_back(result);

    calc.widgets.push_back(new Button("Sum", [&]() {
        // AI: ParseInt() вместо stoi(): пустое поле даёт 0, а не исключение.
        result->SetText("Result: " + std::to_string(sum(first->ParseInt(), second->ParseInt())));
    }, 2));
    calc.widgets.push_back(new Button("Sub", [&]() {
        result->SetText("Result: " + std::to_string(sub(first->ParseInt(), second->ParseInt())));
    }, 2));
    calc.widgets.push_back(new Button("Mul", [&]() {
        result->SetText("Result: " + std::to_string(mul(first->ParseInt(), second->ParseInt())));
    }, 2));
    calc.widgets.push_back(new Button("Div", [&]() {
        const int a = first->ParseInt();
        const int b = second->ParseInt();
        // AI: Деление на ноль — это SIGFPE, а не C++-исключение. Проверяем
        // AI: явно, иначе процесс упадёт прямо из обработчика кнопки.
        if (b == 0) {
            result->SetText("Result: division by zero");
            return;
        }
        result->SetText("Result: " + std::to_string(divide(a, b)));
    }, 2));
    calc.widgets.push_back(new Button("EXIT", [&]() { running = false; }, 6));

    // AI: Здесь нет вложенных меню, поэтому Quit и Close одинаково означают
    // AI: «выйти»: всё, что не Continue, завершает цикл.
    while (running) {
        calc.Show();
        if (calc.HandleKey(hcm::ReadKey()) != hcm::Result::Continue) {
            break;
        }
    }

    // AI: endwin() убран: его вызывает ~Terminal().
    return 0;
}
