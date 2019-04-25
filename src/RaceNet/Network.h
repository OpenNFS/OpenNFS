//
// https://github.com/OmarAflak/Neural-Network
//

#pragma once

#include <vector>
#include <fstream>
#include <cmath>
#include <time.h>
#include <stdlib.h>

#include "Matrix.h"

class Network {
public:
    Network(std::vector<int> neurons, double learningRate);

    Network(const char *filepath);

    Matrix<double> computeOutput(std::vector<double> input);

    void saveNetworkParams(const char *filepath);

    void loadNetworkParams(const char *filepath);

    std::vector<Matrix<double> > W;
    std::vector<Matrix<double> > B;

private:
    std::vector<Matrix<double> > H;
    std::vector<Matrix<double> > dEdW;
    std::vector<Matrix<double> > dEdB;

    Matrix<double> Y;

    int hiddenLayersCount;
    double learningRate;

    static double random(double x);

    static double sigmoid(double x);

    static double sigmoidePrime(double x);

    void printToFile(Matrix<double> &m, std::ofstream &file);
};