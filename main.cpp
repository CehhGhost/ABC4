#include <deque>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <set>
#include <stdlib.h>
#include <synchapi.h>

pthread_mutex_t mutex;

// класс проверяющего числа(был создан для более удобного обращения с потоками)
class Checker {
public:
    // метод для передачи числа в поток(плюс сопутствующие параметры)
    int createThread(size_t num, size_t n, bool intoFile, const std::string& output = "output.txt") {
        _num = num;
        _n = n;
        _intoFile = intoFile;
        _output = output;
        return pthread_create(&_thread, nullptr, Checker::_func, this);
    }
    // метод ожидания окончания работы потока
    int waitThread() const {
        return pthread_join(_thread, nullptr);
    }

private:
    // сам поток
    pthread_t _thread;
    // число, которое нужно проверить
    size_t _num;
    // n из условия задачи
    size_t _n;
    // путь к файлу вывода
    std::string _output;
    // флаг для проверки записи в файл(если true)
    bool _intoFile;
    void _check() {
        // лочим данные, для раздельного доступа потоков к данным
        pthread_mutex_lock(&mutex);
        // создаем сет из цифр числа
        auto nums = Checker::_getNums(_num);
        bool flag = true;
        size_t x = _num * _n;
        // открываем данные
        pthread_mutex_unlock(&mutex);
        // проверяем есть ли в новом числе (изначальное число перемноженное на n)
        while (x > 0) {
            if (nums.find(x % 10) == nums.end()) {
                flag = false;
                break;
            }
            x /= 10;
        }
        // если не нашлось отличных цифр, то записываем данные в консоль и/или в файл(предварительно залочив данные для раздельного доступа потоков к данным)
        if (flag) {
            pthread_mutex_lock(&mutex);
            std::cout << std::to_string(_num) + "\r\n";
            std::ofstream fout(_output, std::ios::app);
            if (_intoFile) {
                fout << std::to_string(_num) + "\r\n";
            }
            fout.close();
            pthread_mutex_unlock(&mutex);
        }
    }
    // метод для создания сета из цифр переданного числа
    static std::set<size_t> _getNums(size_t num) {
        size_t x = num;
        std::set<size_t> res;
        while (x > 0) {
            res.insert(x % 10);
            x /= 10;
        }
        return res;
    }
    // функция обочка для передачи проверяющей функции в отдельный поток
    static void *_func(void *param) {
        (static_cast<Checker *>(param))->_check();
        return nullptr;
    }
};

int main(int argc, char *argv[]) {
    // генерируем мутекс
    pthread_mutex_init(&mutex, nullptr);
    // число потоков
    size_t threads;
    // n из условия задачи
    size_t n;
    // флаг для ввода из файла(если true)
    bool intoFile = false;
    // сид для рандомной генерации
    int seed;
    // путь до файла вывода
    std::string output;
    // если никаких параметров из командной строки не передавалось
    if (argc <= 1) {
        std::cout << "Choose your input:\r\n1) manual\r\n2) file\r\nother - random\r\n";
        int choice;
        std::cin >> choice;
        // если выбран ручной ввод
        if (choice == 1) {
            std::cout << "Set your threads amount(>0 && <= 16)\r\n";
            std::cin >> threads;
            if (threads <= 0 || threads > 16) {
                std::cout << "Amount of threads is out of range\r\n";
                return 1;
            }
            std::cout << "Set your n(>1 && <= 9)\r\n";
            std::cin >> n;
            if (n <= 1 || n > 9) {
                std::cout << "Your n is out of range\r\n";
                return 1;
            }
        }
        // если выбран ввод через файл
        else if (choice == 2) {
            std::ifstream fin("input.txt");
            fin >> threads;
            fin >> n;
            fin.close();
            output = "output.txt";
            // помечаем флаг
            intoFile = true;
        }
        // если хотим сгенерировать случайно
        else {
            std::cout << "Choose seed generation(>0 && <= 100):\r\n";
            std::cin >> seed;
            if (seed <= 0 || seed > 100) {
                std::cout << "Seed is out of range\r\n";
                return 1;
            }
            srand(seed);
            threads = (rand() % 16) + 1;
            n = (rand() % 9) + 1;
            std::cout << "your threads amount = " << threads << "\r\nyour n = " << n << "\r\n";
            // чтобы было видно какие данные мы сгенерировали останавливаемся на секунду
            Sleep(1000);
        }
    }
    // если передаем через командную строчку вместе с файлами ввода и вывода
    else if (argc == 3) {
        // считываем данные с помощью первого параметра
        std::ifstream fin(argv[1]);
        fin >> threads;
        fin >> n;
        fin.close();
        // вводим данные с помощью второго параметра
        output = argv[2];
        // помечаем флаг
        intoFile = true;
    }
    // создаем массив потоков, которые будут выполнять проверку чисел
    Checker checkers[threads];
    // ставим флаг, отвечающий за конец программы
    bool flag = true;
    // проходим по заданному диапазону чисел
    for (size_t i = 1000; flag;) {
        // цикл для передачи чисел на проверку в потоки
        for (size_t j = 0; j < threads; ++j) {
            // отправляем число на проверку в отдельный поток
            checkers[j].createThread(i, n, intoFile, output);
            ++i;
            if (i > 999999999) {
                flag = false;
            }
        }
        // цикл для ожидании освобождения всех потоков, для дальнейшей работы
        for (size_t j = 0; j < threads; ++j) {
            checkers[j].waitThread();
        }
    }
    return 0;
}
