#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <complex>

template<typename T>
class Image {
private:
    int width_;
    int height_;
    T* data_;

public:
    static constexpr double epsilon = 1e-6;
    Image(int width, int height, bool randomize);
    Image(const Image& other);
    Image& operator=(const Image& other);
    ~Image();

    T& operator()(int row, int col);
    const T& operator()(int row, int col) const;

    int width() const;
    int height() const;
    int size() const;


    bool operator==(const Image& other) const;
    bool operator!=(const Image& other) const;



    Image operator+(const Image& other) const;
    Image operator*(const Image& other) const;
    Image operator*(T scalar) const;
    Image operator+(T scalar) const;
    Image operator!() const;

    double fill_ratio() const;

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const Image<U>& img);




};








template<typename T>
Image<T>::Image(int width, int height, bool randomize)
    : width_(width), height_(height), data_(nullptr) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Image dimensions must be positive");
    }
    data_ = new T[static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)];

    if (randomize) {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = 0; i < size(); ++i) {
            if constexpr (std::is_same_v<T, bool>) {
                std::bernoulli_distribution dist(0.5);
                data_[i] = dist(gen);
            }
            else if constexpr (std::is_integral_v<T>) {
                std::uniform_int_distribution<int> dist(
                    static_cast<int>(std::numeric_limits<T>::min()),
                    static_cast<int>(std::numeric_limits<T>::max()));
                data_[i] = static_cast<T>(dist(gen));
            }
            else {
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                data_[i] = static_cast<T>(dist(gen));
            }
        }
    }
    else {
        for (int i = 0; i < size(); ++i) {
            data_[i] = T{};
        }
    }
}

template<typename T>
Image<T>::Image(const Image& other)
    : width_(other.width_), height_(other.height_),
    data_(new T[static_cast<std::size_t>(other.width_) * static_cast<std::size_t>(other.height_)]) {
    for (int i = 0; i < size(); ++i) {
        data_[i] = other.data_[i];
    }
}

template<typename T>
Image<T>& Image<T>::operator=(const Image& other) {
    if (this != &other) {
        T* new_data = new T[static_cast<std::size_t>(other.width_) * static_cast<std::size_t>(other.height_)];
        for (int i = 0; i < other.size(); ++i) {
            new_data[i] = other.data_[i];
        }
        delete[] data_;
        data_ = new_data;
        width_ = other.width_;
        height_ = other.height_;
    }
    return *this;
}

template<typename T>
Image<T>::~Image() {
    delete[] data_;
}

template<typename T>
T& Image<T>::operator()(int row, int col) {
    if (row < 0 || row >= height_ || col < 0 || col >= width_) {
        throw std::out_of_range("Image index out of range");
    }
    return data_[static_cast<std::size_t>(row) * static_cast<std::size_t>(width_)
        + static_cast<std::size_t>(col)];
}

template<typename T>
const T& Image<T>::operator()(int row, int col) const {
    if (row < 0 || row >= height_ || col < 0 || col >= width_) {
        throw std::out_of_range("Image index out of range");
    }
    return data_[static_cast<std::size_t>(row) * static_cast<std::size_t>(width_)
        + static_cast<std::size_t>(col)];
}

template<typename T>
int Image<T>::width() const { return width_; }

template<typename T>
int Image<T>::height() const { return height_; }

template<typename T>
int Image<T>::size() const { return width_ * height_; }





template<typename T>
bool Image<T>::operator==(const Image& other) const {
    if (width_ != other.width_ || height_ != other.height_) {
        return false;
    }
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, std::complex<float>>
            || std::is_same_v<T, std::complex<double>>) {
            if (std::abs(data_[i].real() - other.data_[i].real()) > epsilon) {
                return false;
            }
            if (std::abs(data_[i].imag() - other.data_[i].imag()) > epsilon) {
                return false;
            }
        }
        else if constexpr (std::is_floating_point_v<T>) {
            if (std::abs(data_[i] - other.data_[i]) > epsilon) {
                return false;
            }
        }
        else {
            if (data_[i] != other.data_[i]) {
                return false;
            }
        }
    }
    return true;
}

template<typename T>
bool Image<T>::operator!=(const Image& other) const {
    return !(*this == other);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Image<T>& img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            os << img(row, col) << ' ';
        }
        os << '\n';
    }
    return os;
}

template<typename T>
Image<T> Image<T>::operator+(const Image& other) const {
    if (width_ != other.width_ || height_ != other.height_) {
        throw std::invalid_argument("Image sizes must match for addition");
    }
    Image<T> result(width_, height_, false);
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            result.data_[i] = data_[i] || other.data_[i];
        }
        else {
            result.data_[i] = static_cast<T>(data_[i] + other.data_[i]);
        }
    }
    return result;
}

template<typename T>
Image<T> Image<T>::operator*(const Image& other) const {
    if (width_ != other.width_ || height_ != other.height_) {
        throw std::invalid_argument("Image sizes must match for multiplication");
    }
    Image<T> result(width_, height_, false);
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            result.data_[i] = data_[i] && other.data_[i];
        }
        else {
            result.data_[i] = static_cast<T>(data_[i] * other.data_[i]);
        }
    }
    return result;
}

template<typename T>
Image<T> Image<T>::operator*(T scalar) const {
    Image<T> result(width_, height_, false);
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            result.data_[i] = data_[i] && scalar;
        }
        else {
            result.data_[i] = static_cast<T>(data_[i] * scalar);
        }
    }
    return result;
}

template<typename T>
Image<T> Image<T>::operator+(T scalar) const {
    Image<T> result(width_, height_, false);
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            result.data_[i] = data_[i] || scalar;
        }
        else {
            result.data_[i] = static_cast<T>(data_[i] + scalar);
        }
    }
    return result;
}

template<typename T>
Image<T> operator*(T scalar, const Image<T>& img) {
    return img * scalar;
}

template<typename T>
Image<T> operator+(T scalar, const Image<T>& img) {
    return img + scalar;
}

template<typename T>
Image<T> Image<T>::operator!() const {
    Image<T> result(width_, height_, false);
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            result.data_[i] = !data_[i];
        }
        else if constexpr (std::is_signed_v<T>) {
            if (data_[i] == std::numeric_limits<T>::min()) {
                result.data_[i] = std::numeric_limits<T>::max();
            }
            else {
                result.data_[i] = static_cast<T>(-data_[i]);
            }
        }
        else if constexpr (std::is_integral_v<T>) {
            result.data_[i] = static_cast<T>(std::numeric_limits<T>::max() - data_[i]);
        }
        else {
            result.data_[i] = -data_[i];
        }
    }
    return result;
}


template<typename T>
double Image<T>::fill_ratio() const {
    double sum = 0.0;
    for (int i = 0; i < size(); ++i) {
        if constexpr (std::is_same_v<T, bool>) {
            sum += data_[i] ? 1.0 : 0.0;
        }
        else {
            sum += static_cast<double>(data_[i]);
        }
    }

    double max_value = 0.0;
    if constexpr (std::is_same_v<T, bool>) {
        max_value = 1.0;
    }
    else {
        max_value = static_cast<double>(std::numeric_limits<T>::max());
    }

    return sum / (static_cast<double>(size()) * max_value);
}



//--------------------------------------------------------------------------------------------------------------------
template<typename T>
Image<T> draw_filled_circle(int width, int height, double cx, double cy, double r, T fill_value) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Dimensions must be positive");
    }
    if (r < 0) {
        throw std::invalid_argument("Radius must be non-negative");
    }
    Image<T> img(width, height, false);
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            double dx = col - cx;
            double dy = row - cy;
            if (dx * dx + dy * dy <= r * r) {
                img(row, col) = fill_value;
            }
        }
    }
    return img;
}




template<typename T>
void demo(const char* type_name) {
    std::cout << "=== Demo for type: " << type_name << " ===\n";

    Image<T> a(3, 3, true);
    Image<T> b(3, 3, true);

    std::cout << "a:\n" << a;
    std::cout << "b:\n" << b;

    std::cout << "a == b: " << (a == b) << "\n";
    std::cout << "a != b: " << (a != b) << "\n";

    std::cout << "a + b:\n" << (a + b);
    std::cout << "a * b:\n" << (a * b);

    std::cout << "a * scalar(2):\n" << (a * static_cast<T>(2));
    std::cout << "scalar(2) * a:\n" << (static_cast<T>(2) * a);
    std::cout << "a + scalar(1):\n" << (a + static_cast<T>(1));
    std::cout << "scalar(1) + a:\n" << (static_cast<T>(1) + a);

    std::cout << "!a:\n" << (!a);

    std::cout << "fill_ratio: " << a.fill_ratio() << "\n";


    std::cout << "circle r=2 (fill=1):\n"
        << draw_filled_circle<T>(5, 5, 2.0, 2.0, 2.0, static_cast<T>(1));

    std::cout << "\n";
}



int main() {
    try {
        demo<bool>("bool");
        demo<char>("char");
        demo<float>("float");
        demo<short>("short");

        std::cout << "=== Exception checks ===\n";
        Image<int> img(2, 2, false);
        try {
            img(5, 5);
        }
        catch (const std::out_of_range& e) {
            std::cout << "Caught out_of_range: " << e.what() << "\n";
        }
        try {
            Image<int> bad(0, 0, false);
        }
        catch (const std::invalid_argument& e) {
            std::cout << "Caught invalid_argument: " << e.what() << "\n";
        }

        std::cout << "=== Interactive circle ===\n";
        int w, h;
        double cx, cy, r;
        std::cout << "Enter width height: ";
        std::cin >> w >> h;
        std::cout << "Enter cx cy r: ";
        std::cin >> cx >> cy >> r;
        std::cout << "Circle (int, fill=1):\n"
            << draw_filled_circle<int>(w, h, cx, cy, r, 1);

    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled error: " << e.what() << "\n";
    }
    return 0;
}