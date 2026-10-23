#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>
#include <complex>
#include <algorithm>

#define NOMINMAX
#include <windows.h>

// --- Защита от переполнения ---
short saturate_short(long long v) {
    if (v > static_cast<long long>(std::numeric_limits<short>::max())) return std::numeric_limits<short>::max();
    if (v < static_cast<long long>(std::numeric_limits<short>::min())) return std::numeric_limits<short>::min();
    return static_cast<short>(v);
}

char saturate_char(long long v) {
    if (v > static_cast<long long>(std::numeric_limits<char>::max())) return std::numeric_limits<char>::max();
    if (v < static_cast<long long>(std::numeric_limits<char>::min())) return std::numeric_limits<char>::min();
    return static_cast<char>(v);
}

float saturate_float(double v) {
    if (v > static_cast<double>(std::numeric_limits<float>::max())) return std::numeric_limits<float>::max();
    if (v < static_cast<double>(std::numeric_limits<float>::lowest())) return std::numeric_limits<float>::lowest();
    return static_cast<float>(v);
}

std::complex<float> saturate_complex(std::complex<double> v) {
    return std::complex<float>(saturate_float(v.real()), saturate_float(v.imag()));
}

// --- Перегрузки операций ---
bool op_add(bool a, bool b) { return a || b; }
char op_add(char a, char b) { return saturate_char(static_cast<long long>(a) + static_cast<long long>(b)); }
short op_add(short a, short b) { return saturate_short(static_cast<long long>(a) + static_cast<long long>(b)); }
float op_add(float a, float b) { return saturate_float(static_cast<double>(a) + static_cast<double>(b)); }
std::complex<float> op_add(std::complex<float> a, std::complex<float> b) { return a + b; }

bool op_sub(bool a, bool b) { return a && !b; }
char op_sub(char a, char b) { return saturate_char(static_cast<long long>(a) - static_cast<long long>(b)); }
short op_sub(short a, short b) { return saturate_short(static_cast<long long>(a) - static_cast<long long>(b)); }
float op_sub(float a, float b) { return saturate_float(static_cast<double>(a) - static_cast<double>(b)); }
std::complex<float> op_sub(std::complex<float> a, std::complex<float> b) { return a - b; }

bool op_mul(bool a, bool b) { return a && b; }
char op_mul(char a, char b) { return saturate_char(static_cast<long long>(a) * static_cast<long long>(b)); }
short op_mul(short a, short b) { return saturate_short(static_cast<long long>(a) * static_cast<long long>(b)); }
float op_mul(float a, float b) { return saturate_float(static_cast<double>(a) * static_cast<double>(b)); }
std::complex<float> op_mul(std::complex<float> a, std::complex<float> b) { return a * b; }

bool op_invert(bool a) { return !a; }
char op_invert(char a) { return (a == std::numeric_limits<char>::min()) ? std::numeric_limits<char>::max() : static_cast<char>(-a); }
short op_invert(short a) { return (a == std::numeric_limits<short>::min()) ? std::numeric_limits<short>::max() : static_cast<short>(-a); }
float op_invert(float a) { return -a; }
std::complex<float> op_invert(std::complex<float> a) { return -a; }

// --- random_value: перегрузки по типу ---
bool random_value(bool, std::mt19937& engine) {
    std::bernoulli_distribution dist(0.5);
    return dist(engine);
}
char random_value(char, std::mt19937& engine) {
    std::uniform_int_distribution<int> dist(
        static_cast<int>(std::numeric_limits<char>::min()),
        static_cast<int>(std::numeric_limits<char>::max()));
    return static_cast<char>(dist(engine));
}
short random_value(short, std::mt19937& engine) {
    std::uniform_int_distribution<int> dist(
        static_cast<int>(std::numeric_limits<short>::min()),
        static_cast<int>(std::numeric_limits<short>::max()));
    return static_cast<short>(dist(engine));
}
float random_value(float, std::mt19937& engine) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return static_cast<float>(dist(engine));
}
std::complex<float> random_value(std::complex<float>, std::mt19937& engine) {
    std::uniform_real_distribution<float> real_dist(0.0f, 1.0f);
    std::uniform_real_distribution<float> imag_dist(0.0f, 1.0f);
    return std::complex<float>(real_dist(engine), imag_dist(engine));
}

// --- Класс ---
template<typename T>
class Image {
private:
    int width_;
    int height_;
    T* data_;

public:
    static const double epsilon;

    Image(int width, int height, bool randomize)
        : width_(width), height_(height), data_(nullptr) {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("Размеры изображения должны быть положительными");
        data_ = new T[static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)];
        if (randomize) {
            std::random_device rd;
            std::mt19937 engine(rd());
            for (int i = 0; i < size(); ++i)
                data_[i] = random_value(T{}, engine);
        }
        else {
            for (int i = 0; i < size(); ++i) data_[i] = T{};
        }
    }

    Image(const Image& other)
        : width_(other.width_), height_(other.height_),
        data_(new T[static_cast<std::size_t>(other.width_) * static_cast<std::size_t>(other.height_)]) {
        for (int i = 0; i < size(); ++i) data_[i] = other.data_[i];
    }

    Image& operator=(const Image& other) {
        if (this == &other) return *this;
        delete[] data_;
        width_ = other.width_;
        height_ = other.height_;
        data_ = new T[static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)];
        for (int i = 0; i < size(); ++i) data_[i] = other.data_[i];
        return *this;
    }

    ~Image() { delete[] data_; }

    T& operator()(int row, int col) {
        if (row < 0 || row >= height_ || col < 0 || col >= width_)
            throw std::out_of_range("Индекс выходит за пределы изображения");
        return data_[static_cast<std::size_t>(row) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(col)];
    }

    const T& operator()(int row, int col) const {
        if (row < 0 || row >= height_ || col < 0 || col >= width_)
            throw std::out_of_range("Индекс выходит за пределы изображения");
        return data_[static_cast<std::size_t>(row) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(col)];
    }

    int width() const { return width_; }
    int height() const { return height_; }
    int size() const { return width_ * height_; }

    Image operator+(const Image& other) const {
        int w = std::max(width_, other.width_);
        int h = std::max(height_, other.height_);
        Image result(w, h, false);
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c) {
                T a = (r < height_ && c < width_) ? data_[r * width_ + c] : T{};
                T b = (r < other.height_ && c < other.width_) ? other.data_[r * other.width_ + c] : T{};
                result.data_[r * w + c] = op_add(a, b);
            }
        return result;
    }

    Image operator-(const Image& other) const {
        int w = std::max(width_, other.width_);
        int h = std::max(height_, other.height_);
        Image result(w, h, false);
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c) {
                T a = (r < height_ && c < width_) ? data_[r * width_ + c] : T{};
                T b = (r < other.height_ && c < other.width_) ? other.data_[r * other.width_ + c] : T{};
                result.data_[r * w + c] = op_sub(a, b);
            }
        return result;
    }

    Image operator*(const Image& other) const {
        int w = std::max(width_, other.width_);
        int h = std::max(height_, other.height_);
        Image result(w, h, false);
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c) {
                T a = (r < height_ && c < width_) ? data_[r * width_ + c] : T{};
                T b = (r < other.height_ && c < other.width_) ? other.data_[r * other.width_ + c] : T{};
                result.data_[r * w + c] = op_mul(a, b);
            }
        return result;
    }

    Image operator*(const T& scalar) const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i)
            result.data_[i] = op_mul(data_[i], scalar);
        return result;
    }

    Image operator+(const T& scalar) const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i)
            result.data_[i] = op_add(data_[i], scalar);
        return result;
    }

    Image operator!() const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i)
            result.data_[i] = op_invert(data_[i]);
        return result;
    }

    bool operator==(const Image& other) const {
        if (width_ != other.width_ || height_ != other.height_) return false;
        for (int i = 0; i < size(); ++i) {
            if (std::abs(data_[i] - other.data_[i]) > epsilon) return false;
        }
        return true;
    }

    bool operator!=(const Image& other) const { return !(*this == other); }

    double fill_ratio() const {
        double sum = 0.0;
        for (int i = 0; i < size(); ++i)
            sum += static_cast<double>(std::abs(data_[i]));
        double max_val = static_cast<double>(std::numeric_limits<T>::max());
        if (max_val == 0.0) max_val = 1.0;
        return sum / (static_cast<double>(size()) * max_val);
    }

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const Image<U>& img);
};

template<typename T>
const double Image<T>::epsilon = 1e-6;

template<typename T>
std::ostream& operator<<(std::ostream& os, const Image<T>& img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col)
            os << img(row, col) << ' ';
        os << '\n';
    }
    return os;
}

template<typename T>
Image<T> operator*(const T& scalar, const Image<T>& img) { return img * scalar; }

template<typename T>
Image<T> operator+(const T& scalar, const Image<T>& img) { return img + scalar; }

// --- Задача 3 ---
template<typename T>
Image<T> draw_filled_circle(int width, int height, double cx, double cy, double r, const T& fill_value) {
    if (width <= 0 || height <= 0) throw std::invalid_argument("Размеры должны быть положительными");
    if (r < 0) throw std::invalid_argument("Радиус должен быть неотрицательным");
    Image<T> img(width, height, false);
    for (int row = 0; row < height; ++row)
        for (int col = 0; col < width; ++col) {
            double dx = col - cx;
            double dy = row - cy;
            if (dx * dx + dy * dy <= r * r) img(row, col) = fill_value;
        }
    return img;
}

// --- Меню ---
void print_menu() {
    std::cout << "\n[1] Создать изображение (размеры и значение заполнения)\n";
    std::cout << "[2] Создать изображение (размеры, случайные значения)\n";
    std::cout << "[3] Показать изображение\n";
    std::cout << "[4] Сложить два изображения\n";
    std::cout << "[5] Вычесть два изображения\n";
    std::cout << "[6] Поэлементно умножить два изображения\n";
    std::cout << "[7] Умножить изображение на константу\n";
    std::cout << "[8] Инвертировать изображение\n";
    std::cout << "[9] Вычислить коэффициент заполнения\n";
    std::cout << "[10] Нарисовать круг (задача 3)\n";
    std::cout << "[11] Выход\n";
}

template<typename T>
T read_value(const std::string& prompt) {
    T value{};
    while (true) {
        std::cout << prompt << ": ";
        if (std::cin >> value) return value;
        std::cout << "Некорректный ввод, попробуйте снова.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int read_int(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) return value;
        std::cout << "Некорректный ввод, попробуйте снова.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int read_slot() {
    while (true) {
        int slot = read_int("Какое изображение? (1 или 2): ");
        if (slot == 1 || slot == 2) return slot - 1;
        std::cout << "Введите 1 или 2.\n";
    }
}

template<typename T>
void run_menu() {
    Image<T>* imgs[2] = { nullptr, nullptr };
    while (true) {
        print_menu();
        int choice = read_int("Выберите пункт меню: ");
        try {
            switch (choice) {
            case 1: {
                int slot = read_slot();
                int w = read_int("Ширина: ");
                int h = read_int("Высота: ");
                T value = read_value<T>("Значение заполнения");
                delete imgs[slot];
                imgs[slot] = new Image<T>(w, h, false);
                for (int r = 0; r < h; ++r)
                    for (int c = 0; c < w; ++c)
                        (*imgs[slot])(r, c) = value;
                std::cout << "Изображение " << slot + 1 << ":\n" << *imgs[slot];
                break;
            }
            case 2: {
                int slot = read_slot();
                int w = read_int("Ширина: ");
                int h = read_int("Высота: ");
                delete imgs[slot];
                imgs[slot] = new Image<T>(w, h, true);
                std::cout << "Изображение " << slot + 1 << ":\n" << *imgs[slot];
                break;
            }
            case 3: {
                int slot = read_slot();
                if (!imgs[slot]) { std::cout << "Изображение ещё не создано.\n"; break; }
                std::cout << *imgs[slot];
                break;
            }
            case 4: {
                if (!imgs[0] || !imgs[1]) { std::cout << "Сначала создайте оба изображения.\n"; break; }
                std::cout << (*imgs[0] + *imgs[1]);
                break;
            }
            case 5: {
                if (!imgs[0] || !imgs[1]) { std::cout << "Сначала создайте оба изображения.\n"; break; }
                std::cout << (*imgs[0] - *imgs[1]);
                break;
            }
            case 6: {
                if (!imgs[0] || !imgs[1]) { std::cout << "Сначала создайте оба изображения.\n"; break; }
                std::cout << (*imgs[0] * *imgs[1]);
                break;
            }
            case 7: {
                int slot = read_slot();
                if (!imgs[slot]) { std::cout << "Изображение ещё не создано.\n"; break; }
                T scalar = read_value<T>("Константа");
                std::cout << (*imgs[slot] * scalar);
                break;
            }
            case 8: {
                int slot = read_slot();
                if (!imgs[slot]) { std::cout << "Изображение ещё не создано.\n"; break; }
                std::cout << (!*imgs[slot]);
                break;
            }
            case 9: {
                int slot = read_slot();
                if (!imgs[slot]) { std::cout << "Изображение ещё не создано.\n"; break; }
                std::cout << "Коэффициент заполнения = " << imgs[slot]->fill_ratio() << '\n';
                break;
            }
            case 10: {
                int w = read_int("Ширина: ");
                int h = read_int("Высота: ");
                double cx = read_value<double>("Координата центра cx");
                double cy = read_value<double>("Координата центра cy");
                double r = read_value<double>("Радиус r");
                T fill = read_value<T>("Значение заполнения");
                std::cout << draw_filled_circle<T>(w, h, cx, cy, r, fill);
                break;
            }
            case 11:
                delete imgs[0];
                delete imgs[1];
                std::cout << "До свидания!\n";
                return;
            default:
                std::cout << "Неизвестный пункт меню.\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << '\n';
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "Выберите тип элемента изображения:\n";
    std::cout << "  [1] bool\n  [2] char\n  [3] float\n  [4] short\n";
    int type = read_int("Ваш выбор: ");
    switch (type) {
    case 1: run_menu<bool>(); break;
    case 2: run_menu<char>(); break;
    case 3: run_menu<float>(); break;
    case 4: run_menu<short>(); break;
    default: std::cout << "Неверный выбор.\n"; break;
    }
    return 0;
}