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
    initscr(); // Инициализация ncurses
    start_color(); // Инициализация цветов
    init_pair(1, COLOR_MAGENTA, COLOR_BLUE); 
    init_pair(2, COLOR_BLACK, COLOR_YELLOW); 
    init_pair(3, COLOR_WHITE, COLOR_CYAN); 
    init_pair(4, COLOR_WHITE, COLOR_GREEN); 
    init_pair(5, COLOR_YELLOW, COLOR_BLUE); 
    init_pair(6, COLOR_BLACK, COLOR_RED);
    init_pair(7, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(8, COLOR_BLACK, COLOR_MAGENTA);

    bkgd(COLOR_PAIR(7)); 

    noecho(); // Не показывать вводимые символы
    curs_set(0); // Не показывать курсор
    keypad(stdscr, TRUE); // Включаем режим работы с функциональными клавишами

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
    calc.widgets.push_back(new Button("EXIT", []() { endwin(); exit(0); }, 6));

    while (true) {
        calc.Show();
        calc.Input(getch_key());
    }

    endwin(); // Завершение работы с ncurses
    return 0;
}
