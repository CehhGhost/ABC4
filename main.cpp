#include <deque>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <set>
#include <stdlib.h>
#include <synchapi.h>

pthread_mutex_t mutex;

class Checker {
public:
    int createThread(size_t num, size_t n, bool intoFile, const std::string& output = "output.txt") {
        _num = num;
        _n = n;
        _intoFile = intoFile;
        _output = output;
        return pthread_create(&_thread, nullptr, Checker::_func, this);
    }
    int waitThread() const {
        return pthread_join(_thread, nullptr);
    }

private:
    pthread_t _thread;
    size_t _num;
    size_t _n;
    std::string _output;
    bool _intoFile;
    void _check() {
        pthread_mutex_lock(&mutex);
        auto nums = Checker::_getNums(_num);
        bool flag = true;
        size_t x = _num * _n;
        pthread_mutex_unlock(&mutex);
        while (x > 0) {
            if (nums.find(x % 10) == nums.end()) {
                flag = false;
                break;
            }
            x /= 10;
        }
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
    static std::set<size_t> _getNums(size_t num) {
        size_t x = num;
        std::set<size_t> res;
        while (x > 0) {
            res.insert(x % 10);
            x /= 10;
        }
        return res;
    }
    static void *_func(void *param) {
        (static_cast<Checker *>(param))->_check();
        return nullptr;
    }
};

int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, nullptr);
    size_t threads;
    size_t n;
    bool intoFile = false;
    int seed;
    if (argc <= 1) {
        std::cout << "Choose your input:\r\n1) manual\r\n2) file\r\nother - random\r\n";
        int choice;
        std::cin >> choice;
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
        } else if (choice == 2) {
            std::ifstream fin("input.txt");
            fin >> threads;
            fin >> n;
            fin.close();
            intoFile = true;
        } else {
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
            Sleep(1000);
        }
    }
    Checker checkers[threads];
    bool flag = true;
    for (size_t i = 1000; flag;) {
        for (size_t j = 0; j < threads; ++j) {
            checkers[j].createThread(i, n, intoFile);
            ++i;
            if (i > 999999999) {
                flag = false;
            }
        }
        for (size_t j = 0; j < threads; ++j) {
            checkers[j].waitThread();
        }
    }
    return 0;
}
