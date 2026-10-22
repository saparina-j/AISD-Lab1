#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <complex>
#include <algorithm>

#define NOMINMAX 
#include <windows.h> 


template <typename T>
struct is_complex : std::false_type {};

template <typename U>
struct is_complex<std::complex<U>> : std::true_type {};


template<typename T>
T saturate(long long value) {
    if (value > static_cast<long long>(std::numeric_limits<T>::max()))
        return std::numeric_limits<T>::max();
    if (value < static_cast<long long>(std::numeric_limits<T>::min()))
        return std::numeric_limits<T>::min();
    return static_cast<T>(value);
}

template<typename T>
T saturate(double value) {
    if (value > static_cast<double>(std::numeric_limits<T>::max()))
        return std::numeric_limits<T>::max();
    if (value < static_cast<double>(std::numeric_limits<T>::lowest()))
        return std::numeric_limits<T>::lowest();
    return static_cast<T>(value);
}


template<typename T>
class Image {
private:
    int width_;
    int height_;
    T* data_;

    template<typename E>
    T random_value(E& engine) {
        if constexpr (is_complex<T>::value) {
            using R = typename T::value_type;
            std::uniform_real_distribution<R> real_dist(R(0), R(1));
            std::uniform_real_distribution<R> imag_dist(R(0), R(1));
            return T(real_dist(engine), imag_dist(engine));
        }
        else if constexpr (std::is_same_v<T, bool>) {
            std::bernoulli_distribution dist(0.5);
            return dist(engine);
        }
        else if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<int> dist(
                static_cast<int>(std::numeric_limits<T>::min()),
                static_cast<int>(std::numeric_limits<T>::max()));
            return static_cast<T>(dist(engine));
        }
        else {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            return static_cast<T>(dist(engine));
        }
    }

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
                data_[i] = random_value(engine);
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
                if constexpr (std::is_same_v<T, bool>)
                    result.data_[r * w + c] = a || b;
                else if constexpr (std::is_integral_v<T>)
                    result.data_[r * w + c] = saturate<T>(static_cast<long long>(a) + static_cast<long long>(b));
                else
                    result.data_[r * w + c] = saturate<T>(static_cast<double>(a) + static_cast<double>(b));
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
                if constexpr (std::is_same_v<T, bool>)
                    result.data_[r * w + c] = a && !b;
                else if constexpr (std::is_integral_v<T>)
                    result.data_[r * w + c] = saturate<T>(static_cast<long long>(a) - static_cast<long long>(b));
                else
                    result.data_[r * w + c] = saturate<T>(static_cast<double>(a) - static_cast<double>(b));
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
                if constexpr (std::is_same_v<T, bool>)
                    result.data_[r * w + c] = a && b;
                else if constexpr (std::is_integral_v<T>)
                    result.data_[r * w + c] = saturate<T>(static_cast<long long>(a) * static_cast<long long>(b));
                else
                    result.data_[r * w + c] = saturate<T>(static_cast<double>(a) * static_cast<double>(b));
            }
        return result;
    }

    Image operator*(const T& scalar) const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i) {
            if constexpr (std::is_same_v<T, bool>)
                result.data_[i] = data_[i] && scalar;
            else if constexpr (std::is_integral_v<T>)
                result.data_[i] = saturate<T>(static_cast<long long>(data_[i]) * static_cast<long long>(scalar));
            else
                result.data_[i] = saturate<T>(static_cast<double>(data_[i]) * static_cast<double>(scalar));
        }
        return result;
    }

    Image operator+(const T& scalar) const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i) {
            if constexpr (std::is_same_v<T, bool>)
                result.data_[i] = data_[i] || scalar;
            else if constexpr (std::is_integral_v<T>)
                result.data_[i] = saturate<T>(static_cast<long long>(data_[i]) + static_cast<long long>(scalar));
            else
                result.data_[i] = saturate<T>(static_cast<double>(data_[i]) + static_cast<double>(scalar));
        }
        return result;
    }

    Image operator!() const {
        Image result(width_, height_, false);
        for (int i = 0; i < size(); ++i) {
            if constexpr (std::is_same_v<T, bool>)
                result.data_[i] = !data_[i];
            else if constexpr (std::is_signed_v<T>)
                result.data_[i] = (data_[i] == std::numeric_limits<T>::min()) ? std::numeric_limits<T>::max() : static_cast<T>(-data_[i]);
            else if constexpr (std::is_integral_v<T>)
                result.data_[i] = static_cast<T>(std::numeric_limits<T>::max() - data_[i]);
            else
                result.data_[i] = -data_[i];
        }
        return result;
    }

    bool operator==(const Image& other) const {
        if (width_ != other.width_ || height_ != other.height_) return false;
        for (int i = 0; i < size(); ++i) {
            if constexpr (is_complex<T>::value) {
                if (std::abs(data_[i].real() - other.data_[i].real()) > epsilon) return false;
                if (std::abs(data_[i].imag() - other.data_[i].imag()) > epsilon) return false;
            }
            else if constexpr (std::is_floating_point_v<T>) {
                if (std::abs(data_[i] - other.data_[i]) > epsilon) return false;
            }
            else {
                if (data_[i] != other.data_[i]) return false;
            }
        }
        return true;
    }

    bool operator!=(const Image& other) const { return !(*this == other); }

    double fill_ratio() const {
        double sum = 0.0;
        for (int i = 0; i < size(); ++i) {
            if constexpr (is_complex<T>::value)
                sum += std::abs(data_[i]);
            else
                sum += static_cast<double>(data_[i]);
        }
        double max_val = is_complex<T>::value ? 1.0 : static_cast<double>(std::numeric_limits<T>::max());
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

// --- Задача 3: закрашенный круг 
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