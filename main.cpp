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
};


// --- Реализация ---

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



int main() {
    Image<float> a(3, 3, true);
    Image<float> b = a;
    std::cout << "a == b: " << (a == b) << "\n";
    std::cout << "a != b: " << (a != b) << "\n";

    b(0, 0) = b(0, 0) + 1.0f;
    std::cout << "after change, a == b: " << (a == b) << "\n";




    return 0;
}