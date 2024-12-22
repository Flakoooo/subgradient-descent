#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include <chrono>

// Для засекания времени
class Timer
{
    std::chrono::steady_clock::time_point start;
    std::chrono::time_point<std::chrono::steady_clock> end;
public:
    Timer() { start = std::chrono::high_resolution_clock::now(); }
    ~Timer()
    {
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;
        std::cout << "Минимизация была выполнена за: " << duration.count() << " секунд\n";
    }
};

// Основной метод наискорейшего субградиентного спуска
static std::vector<double> subgradient_descent(
    const std::function<double(const std::vector<double>&)>& objective, // Целевая функция
    const std::function<std::vector<double>(const std::vector<double>&)>& subgradient, // Субградиент
    const std::vector<double>& start, // Начальная точка
    int step_type, // Тип шага: 1 - фиксированный, 2 - убывающий
    int max_iter, // Максимальное количество итераций
    double epsilon = 1e-6 // Допустимая точность
)
{
    std::vector<double> x = start;
    double alpha_0 = 0.01; // Начальный шаг
    double min_step_size = 1e-8; // Минимальное значение шага
    double prev_value = objective(x); // Предыдущее значение функции

    Timer timer;
    for (int iter = 0; iter < max_iter; ++iter) {
        // Вычисляем субградиент в текущей точке
        std::vector<double> g = subgradient(x);

        // Проверяем условие остановки (меньше эпсилон для компоненты субградиента)
        bool stop_condition = true;
        for (size_t i = 0; i < g.size(); ++i) {
            if (std::abs(g[i]) >= epsilon) {
                stop_condition = false;
                break;
            }
        }

        if (stop_condition) {
            std::cout << "Достигнуто оптимальное решение на итерации " << iter << "\n";
            break;
        }

        // Вычисляем шаг по выбранной стратегии
        double step_size;
        if (step_type == 1)
            step_size = alpha_0; // Фиксированный шаг
        else if (step_type == 2)
            step_size = alpha_0 / (iter + 1); // Убывающий шаг
        else
            throw std::invalid_argument("Некорректный тип шага");

        // Проверяем минимальное значение шага
        if (step_size < min_step_size) {
            std::cout << "Шаг стал слишком маленьким на итерации " << iter << ". Завершение.\n";
            break;
        }

        // Обновляем точку
        for (size_t i = 0; i < x.size(); ++i)
            x[i] -= step_size * g[i];

        // Вычисляем текущее значение функции
        double current_value = objective(x);

        // Проверяем изменение значения функции
        if (std::abs(current_value - prev_value) < epsilon) {
            std::cout << "Изменение значения функции меньше " << epsilon << " на итерации " << iter << ". Завершение.\n";
            break;
        }

        prev_value = current_value; // Обновляем предыдущее значение

        // Вывод текущего значения функции и точки
        std::cout << "Итерация " << iter << ": значение функции = " << current_value << ", шаг = " << step_size << "\n";
    }

    return x;
}

int main() {
    setlocale(LC_ALL, "ru");

    // Выбор типа функции
    int function_choice;
    std::cout << "Выберите функцию для тестирования:\n";
    std::cout << "1. Квадратичная функция: f(x) = x1^2 + x2^2 - 2x1 - 2x2\n";
    std::cout << "2. Функция с несколькими локальными минимумами: f(x) = sin(x1) + sin(x2)\n";
    std::cout << "3. Не дифференцируемая функция: f(x) = |x1| + |x2|\n";
    std::cout << "Введите номер функции (1-3): ";
    std::cin >> function_choice;

    if (function_choice < 1 || function_choice > 3) {
        std::cerr << "Некорректный выбор типа шага. Завершение программы.\n";
        return EXIT_FAILURE;
    }

    // Функции
    auto objective = [function_choice](const std::vector<double>& x) {
        switch (function_choice)
        {
        case 1:
            // Функция f(x) = x1^2 + x2^2 - 2x1 - 2x2
            return x[0] * x[0] + x[1] * x[1] - 2 * x[0] - 2 * x[1];
        case 2:
            // Функция f(x) = sin(x1) + sin(x2)
            return sin(x[0]) + sin(x[1]);
        case 3:
            // Функция f(x) = |x1| + |x2|
            return std::abs(x[0]) + std::abs(x[1]);
        default:
            return 0.0; // Возвращаем 0, если функция не выбрана
        }
        };

    // Субградиент функции
    auto subgradient = [function_choice](const std::vector<double>& x) {
        switch (function_choice)
        {
        case 1:
            // Субградиент квадратичной функции
            return std::vector<double>{
                2 * x[0] - 2, // Субградиент для x1
                2 * x[1] - 2  // Субградиент для x2
            };
        case 2:
            // Субградиент функции с несколькими локальными минимумами
            return std::vector<double>{
                cos(x[0]),
                cos(x[1])
            };
        case 3:
            // Субградиент не дифференцируемой функции
            return std::vector<double>{
                x[0] < 0 ? -1.0 : (x[0] > 0 ? 1.0 : 0.0),
                x[1] < 0 ? -1.0 : (x[1] > 0 ? 1.0 : 0.0)
            };
        default:
            return std::vector<double>{0.0, 0.0}; // Возвращаем пустой субградиент по умолчанию
        }
        };

    // Ввод типа шага
    int step_type;
    std::cout << "Выберите тип шага:\n";
    std::cout << "1 - Фиксированный\n";
    std::cout << "2 - Убывающий\n";
    std::cin >> step_type;

    if (step_type < 1 || step_type > 2) {
        std::cerr << "Некорректный выбор типа шага. Завершение программы.\n";
        return EXIT_FAILURE;
    }

    // Ввод максимальной итерации
    int max_iter;
    std::cout << "Введите максимальное количество шагов\n";
    std::cin >> max_iter;

    if (max_iter <= 0) {
        std::cerr << "Некорректное количество итераций. Введите положительное число.\n";
        return EXIT_FAILURE;
    }

    // Начальная точка
    std::vector<double> start = { 2.0, -3.0 };

    // Запуск метода
    std::vector<double> solution = subgradient_descent(objective, subgradient, start, step_type, max_iter);

    // Вывод результата
    std::cout << "Оптимальное решение: (" << solution[0] << ", " << solution[1] << ")\n";

    return EXIT_SUCCESS;
}