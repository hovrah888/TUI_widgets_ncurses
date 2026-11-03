#include <iostream>
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
    calc.widgets.push_back(new Input("Enter 1st number", "Please enter number: ", 3));
    calc.widgets.push_back(new Input("Enter 2nd number", "Please enter number: ", 3));
    calc.widgets.push_back(new Label("Result: ", 5));
    calc.widgets.push_back(new Button("Sum", [&]() {
        int a = stoi(((Input*)calc.widgets[0])->text);
        int b = stoi(((Input*)calc.widgets[1])->text); 
       //((Label*)calc.widgets[2])->SetText("Result: " + to_string(sum(a, b)));
    }, 2));
    calc.widgets.push_back(new Button("Sub", [&]() {
        int a = stoi(((Input*)calc.widgets[0])->text); 
        int b = stoi(((Input*)calc.widgets[1])->text); 
        //((Label*)calc.widgets[2])->SetText("Result: " + to_string(sub(a, b)));
    }, 2));
    calc.widgets.push_back(new Button("Mul", [&]() {
        int a = stoi(((Input*)calc.widgets[0])->text); 
        int b = stoi(((Input*)calc.widgets[1])->text); 
        //((Label*)calc.widgets[2])->SetText("Result: " + to_string(mul(a, b)));
    }, 2));
    calc.widgets.push_back(new Button("Div", [&]() {
        int a = stoi(((Input*)calc.widgets[0])->text); 
        int b = stoi(((Input*)calc.widgets[1])->text); 
        //((Label*)calc.widgets[2])->SetText("Result: " + to_string(divide(a, b)));
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
