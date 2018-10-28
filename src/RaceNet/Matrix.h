//
// https://github.com/OmarAflak/Neural-Network
//

#ifndef OPENNFS_MATRIX_H
#define OPENNFS_MATRIX_H

#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>

template<class T>
class Matrix {
public:
    Matrix<T>(int height, int width);

    Matrix<T>(std::vector<std::vector<T> > const &array);

    Matrix<T>();

    int getHeight();

    int getWidth();

    void fill(T const &value);

    void put(int h, int w, T const &value);

    T get(int h, int w) const;

    Matrix<T> add(T const &value);

    Matrix<T> subtract(T const &value);

    Matrix<T> multiply(T const &value);

    Matrix<T> add(Matrix<T> const &m) const;

    Matrix<T> subtract(Matrix<T> const &m) const;

    Matrix<T> multiply(Matrix<T> const &m) const;

    Matrix<T> dot(Matrix<T> const &m) const;

    Matrix<T> transpose() const;

    Matrix<T> applyFunction(T (*function)(T)) const;

    Matrix<T> subMatrix(int startH, int startW, int h, int w) const;

    void print(std::ostream &flux) const;

    bool operator==(Matrix<T> const &m);

    bool operator!=(Matrix<T> const &m);

    void operator+=(Matrix<T> const &m);

    void operator-=(Matrix<T> const &m);

    void operator*=(Matrix<T> const &m);

    void operator+=(T const &m);

    void operator-=(T const &m);

    void operator*=(T const &m);

    T &operator()(int y, int x);

private:
    std::vector<std::vector<T> > array;
    int height;
    int width;
};

template<class T>
Matrix<T> operator+(Matrix<T> const &a, Matrix<T> const &b);

template<class T>
Matrix<T> operator-(Matrix<T> const &a, Matrix<T> const &b);

template<class T>
Matrix<T> operator*(Matrix<T> const &a, Matrix<T> const &b);

template<class T>
Matrix<T> operator+(Matrix<T> const &a, T const &b);

template<class T>
Matrix<T> operator-(Matrix<T> const &a, T const &b);

template<class T>
Matrix<T> operator*(Matrix<T> const &a, T const &b);

template<class T>
std::ostream &operator<<(std::ostream &flux, Matrix<T> const &m);

#endif


template<class T>
Matrix<T>::Matrix(int height, int width) {
    this->height = height;
    this->width = width;
    this->array = std::vector<std::vector<T> >(height, std::vector<T>(width));
}

template<class T>
Matrix<T>::Matrix(std::vector<std::vector<T> > const &array) {
    assert(array.size() != 0);
    this->height = array.size();
    this->width = array[0].size();
    this->array = array;
}

template<class T>
Matrix<T>::Matrix() {}

template<class T>
int Matrix<T>::getHeight() {
    return height;
}

template<class T>
int Matrix<T>::getWidth() {
    return width;
}

template<class T>
void Matrix<T>::fill(T const &value) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            array[i][j] = value;
        }
    }
}

template<class T>
void Matrix<T>::put(int h, int w, T const &value) {
    array[h][w] = value;
}

template<class T>
T Matrix<T>::get(int h, int w) const {
    return array[h][w];
}

template<class T>
Matrix<T> Matrix<T>::add(T const &value) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            array[i][j] += value;
        }
    }

    return *this;
}

template<class T>
Matrix<T> Matrix<T>::subtract(T const &value) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            array[i][j] -= value;
        }
    }

    return *this;
}

template<class T>
Matrix<T> Matrix<T>::multiply(T const &value) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            array[i][j] *= value;
        }
    }

    return *this;
}

template<class T>
Matrix<T> Matrix<T>::add(Matrix const &m) const {
    assert(height == m.height && width == m.width);

    Matrix result(height, width);
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            result.array[i][j] = array[i][j] + m.array[i][j];
        }
    }

    return result;
}

template<class T>
Matrix<T> Matrix<T>::subtract(Matrix const &m) const {
    assert(height == m.height && width == m.width);

    Matrix result(height, width);
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            result.array[i][j] = array[i][j] - m.array[i][j];
        }
    }
    return result;
}

template<class T>
Matrix<T> Matrix<T>::multiply(Matrix const &m) const {
    assert(height == m.height && width == m.width);

    Matrix result(height, width);
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            result.array[i][j] = array[i][j] * m.array[i][j];
        }
    }
    return result;
}

template<class T>
Matrix<T> Matrix<T>::dot(Matrix const &m) const {
    assert(width == m.height);

    int i, j, h, mwidth = m.width;
    T w = 0;

    Matrix<T> result(height, mwidth);

    for (i = 0; i < height; i++) {
        for (j = 0; j < mwidth; j++) {
            for (h = 0; h < width; h++) {
                w += array[i][h] * m.array[h][j];
            }
            result.array[i][j] = w;
            w = 0;
        }
    }

    return result;
}

template<class T>
Matrix<T> Matrix<T>::transpose() const {
    Matrix<T> result(width, height);
    int i, j;

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            result.array[i][j] = array[j][i];
        }
    }
    return result;
}

template<class T>
Matrix<T> Matrix<T>::applyFunction(T (*function)(T)) const {
    Matrix<T> result(height, width);
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            result.array[i][j] = (*function)(array[i][j]);
        }
    }

    return result;
}

template<class T>
Matrix<T> Matrix<T>::subMatrix(int startH, int startW, int h, int w) const {
    assert(startH + h <= height && startW + w <= width);

    Matrix<T> result(h, w);
    int i, j;

    for (i = startH; i < startH + h; i++) {
        for (j = startW; j < startW + w; j++) {
            result.array[i - startH][j - startW] = array[i][j];
        }
    }
    return result;
}

template<class T>
void Matrix<T>::print(std::ostream &flux) const {
    int i, j;
    int maxLength[width] = {};
    std::stringstream ss;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            ss << array[i][j];
            if (maxLength[j] < ss.str().size()) {
                maxLength[j] = ss.str().size();
            }
            ss.str(std::string());
        }
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            flux << array[i][j];
            ss << array[i][j];
            for (int k = 0; k < maxLength[j] - ss.str().size() + 1; k++) {
                flux << " ";
            }
            ss.str(std::string());
        }
        flux << std::endl;
    }
}

template<class T>
bool Matrix<T>::operator==(Matrix const &m) {
    if (height == m.height && width == m.width) {
        int i, j;
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                if (array[i][j] != m.array[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

template<class T>
bool Matrix<T>::operator!=(Matrix const &m) {
    return !operator==(m);
}

template<class T>
void Matrix<T>::operator+=(Matrix const &m) {
    this->array = add(m).array;
}

template<class T>
void Matrix<T>::operator-=(Matrix const &m) {
    this->array = subtract(m).array;
}

template<class T>
void Matrix<T>::operator*=(Matrix const &m) {
    this->array = multiply(m).array;
}

template<class T>
void Matrix<T>::operator+=(T const &m) {
    add(m);
}

template<class T>
void Matrix<T>::operator-=(T const &m) {
    subtract(m);
}

template<class T>
void Matrix<T>::operator*=(T const &m) {
    multiply(m);
}

template<class T>
T &Matrix<T>::operator()(int y, int x) {
    assert(y < height && x < width);
    return array[y][x];
}

template<class T>
Matrix<T> operator+(Matrix<T> const &a, Matrix<T> const &b) {
    return a.add(b);
}

template<class T>
Matrix<T> operator-(Matrix<T> const &a, Matrix<T> const &b) {
    return a.subtract(b);
}

template<class T>
Matrix<T> operator*(Matrix<T> const &a, Matrix<T> const &b) {
    return a.multiply(b);
}

template<class T>
Matrix<T> operator+(Matrix<T> const &a, T const &b) {
    Matrix<T> result = a;
    return result.add(b);
}

template<class T>
Matrix<T> operator-(Matrix<T> const &a, T const &b) {
    Matrix<T> result = a;
    return result.subtract(b);
}

template<class T>
Matrix<T> operator*(Matrix<T> const &a, T const &b) {
    Matrix<T> result = a;
    return result.multiply(b);
}

template<class T>
std::ostream &operator<<(std::ostream &flux, Matrix<T> const &m) {
    m.print(flux);
    return flux;
}
