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

struct Settings
{
    int function_choice = 0; // Тип функции
    int step_type = 0; // Тип шага
    int max_iter = 0; // Максимум итераций
    double epsilon = 1e-8; // Допустимая точность
    double min_step_size = 1e-8; // Минимальное значение шага
    double start_step = 0.0; // Начальный шаг
    std::vector<double> start = { 2.0, 2.0 }; // Начальная точка

    // Функции для целевой функции и её субградиента
    std::function<double(const std::vector<double>&)> objective;
    std::function<std::vector<double>(const std::vector<double>&)> subgradient;

    // Метод для установки начального шага в зависимости от функции и типа шага
    void setStartStep()
    {
        if (this->step_type == 1) // Фиксированный шаг
            switch (this->function_choice)
            {
            case 1: this->start_step = 0.1; break; // Оптимальный шаг 0.1
            case 2: this->start_step = 0.001; break; // Оптимальный шаг 0.001 или 0.0001 (долго)
            case 3: this->start_step = 1.0; break; // Оптимальный шаг 1.0
            default: this->start_step = 1.0; break;
            }
        else if (this->step_type == 2) // Убывающий шаг
            switch (this->function_choice)
            {
            case 1: this->start_step = 0.1; break; // Оптимальный шаг 0.1
            case 2: this->start_step = 0.1; break; // Оптимальный шаг 0.1 или 0.01 (долго)
            case 3: this->start_step = 0.1; break; // Оптимальный шаг 0.1
            default: this->start_step = 0.1; break;
            }
    }

    // Метод для выбора функции и её субградиента
    void setFunctions()
    {
        switch (this->function_choice)
        {
        case 1:
            // Квадратичная функция: f(x) = x1^2 + x2^2 - 2x1 - 2x2
            this->objective = [](const std::vector<double>& x) {
                return x[0] * x[0] + x[1] * x[1] - 2 * x[0] - 2 * x[1];
                };
            this->subgradient = [](const std::vector<double>& x) {
                return std::vector<double>{
                    2 * x[0] - 2, // Субградиент для x1
                    2 * x[1] - 2  // Субградиент для x2
                };
                };
            break;
        case 2:
            // Функция с несколькими локальными минимумами: f(x) = sin(x1) + sin(x2)
            this->objective = [](const std::vector<double>& x) {
                return sin(x[0]) + sin(x[1]);
                };
            this->subgradient = [](const std::vector<double>& x) {
                return std::vector<double>{
                    cos(x[0]),
                    cos(x[1])
                };
                };
            break;
        case 3:
            // Не дифференцируемая функция: f(x) = |x1| + |x2|
            this->objective = [](const std::vector<double>& x) {
                return std::abs(x[0]) + std::abs(x[1]);
                };
            this->subgradient = [](const std::vector<double>& x) {
                return std::vector<double>{
                    x[0] < 0 ? -1.0 : (x[0] > 0 ? 1.0 : 0.0),
                    x[1] < 0 ? -1.0 : (x[1] > 0 ? 1.0 : 0.0)
                };
                };
            break;
        default:
            this->objective = [](const std::vector<double>&) { return 0.0; };
            this->subgradient = [](const std::vector<double>&) { return std::vector<double>{0.0, 0.0}; };
        }
    }
};

// Основной метод наискорейшего субградиентного спуска
static std::vector<double> subgradient_descent(
    const Settings &settings
)
{
    std::vector<double> x = settings.start;
    double prev_value = settings.objective(x); // Предыдущее значение функции

    Timer timer;
    for (int i = 0; i < settings.max_iter; ++i) {
        // Вычисляем субградиент в текущей точке
        std::vector<double> g = settings.subgradient(x);

        // Проверяем условие остановки (меньше эпсилон для компоненты субградиента)
        bool stop_condition = true;
        for (size_t i = 0; i < g.size(); ++i) {
            if (std::abs(g[i]) >= settings.epsilon) {
                stop_condition = false;
                break;
            }
        }

        if (stop_condition) {
            std::cout << "Достигнуто оптимальное решение на итерации " << i << "\n";
            break;
        }

        // Вычисляем шаг по выбранной стратегии
        double step_size;
        if (settings.step_type == 1) step_size = settings.start_step; // Фиксированный шаг
        else if (settings.step_type == 2) step_size = settings.start_step / std::sqrt(i + 1); // Убывающий шаг
        else throw std::invalid_argument("Некорректный тип шага");

        // Проверяем минимальное значение шага
        if (step_size < settings.min_step_size) {
            std::cout << "Шаг стал слишком маленьким на итерации " << i << ". Завершение.\n";
            break;
        }

        // Обновляем точку
        for (size_t i = 0; i < x.size(); ++i) x[i] -= step_size * g[i];

        // Вычисляем текущее значение функции
        double current_value = settings.objective(x);

        // Проверяем изменение значения функции
        if (std::abs(current_value - prev_value) < settings.epsilon) {
            std::cout << "Изменение значения функции меньше " << settings.epsilon << " на итерации " << i << ". Завершение.\n";
            break;
        }

        prev_value = current_value; // Обновляем предыдущее значение

        // Вывод текущего значения функции и точки
        std::cout << "Итерация " << i << ": значение функции = " << current_value << ", шаг = " << step_size << "\n";
    }
    return x;
}

int main() {
    setlocale(LC_ALL, "ru");

    Settings settings;

    // Выбор типа функции
    std::cout << "Выберите функцию для тестирования:\n";
    std::cout << "1. Квадратичная функция: f(x) = x1^2 + x2^2 - 2x1 - 2x2\n";
    std::cout << "2. Функция с несколькими локальными минимумами: f(x) = sin(x1) + sin(x2)\n";
    std::cout << "3. Не дифференцируемая функция: f(x) = |x1| + |x2|\n";
    std::cout << "Введите номер функции (1-3): ";
    std::cin >> settings.function_choice;

    if (settings.function_choice < 1 || settings.function_choice > 3) {
        std::cerr << "Некорректный выбор типа функции. Завершение программы.\n";
        return EXIT_FAILURE;
    }

    // Ввод типа шага
    std::cout << "Выберите тип шага:\n";
    std::cout << "1 - Фиксированный\n";
    std::cout << "2 - Убывающий\n";
    std::cin >> settings.step_type;

    if (settings.step_type < 1 || settings.step_type > 2) {
        std::cerr << "Некорректный выбор типа шага. Завершение программы.\n";
        return EXIT_FAILURE;
    }

    // Ввод максимальной итерации
    std::cout << "Введите максимальное количество шагов (введите 0 чтобы выполнять без ограничений)\n";
    std::cin >> settings.max_iter;

    if (settings.max_iter < 0) {
        std::cerr << "Некорректное количество итераций. Введите положительное число.\n";
        return EXIT_FAILURE;
    }
    else if (settings.max_iter == 0) settings.max_iter = 1000000;

    settings.setFunctions();
    settings.setStartStep();

    // Запуск метода
    std::vector<double> solution = subgradient_descent(settings);

    // Вывод результата
    std::cout << "Оптимальное решение: (" << solution[0] << ", " << solution[1] << ")\n";

    return EXIT_SUCCESS;
}