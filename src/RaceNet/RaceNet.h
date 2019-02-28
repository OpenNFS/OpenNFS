//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include <time.h>
#include <stdlib.h>
#include <unordered_map>
#include <cmath>
#include <array>
#include <stack>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "RaceNEAT.h"
#include "../Util/Logger.h"

static const float learningRate = 0.7f;
static const std::vector<int> layerParams{3, 10, 5, 3};
// 3 input neurons
// 10 hidden neurons (experimental) : you can specify as many hidden layers as you want (you need to add the number of neurons in each)
// 5 hidden neurons (experimental)
// 3 output neurons (3 outputs)
// 0.7 learning rate (experimental)

enum type {
    RECURRENT,
    NON_RECURRENT
};

class Neuron {
public:
    int type = 0; // 0 = ordinal, 1 = input, 2 = output, 3 = bias
    double value = 0.0;
    bool visited = false;
    std::vector<std::pair<size_t, double>> in_nodes;

    Neuron() = default;

    ~Neuron() { in_nodes.clear(); }
};

class RaceNet {
private:
    std::vector<Neuron> nodes;
    bool recurrent = false;

    std::vector<size_t> input_nodes;
    std::vector<size_t> bias_nodes;
    std::vector<size_t> output_nodes;

    double sigmoid(double x) {
        return 2.0 / (1.0 + std::exp(-4.9 * x)) - 1;
    }

    void evaluate_nonrecurrent(const std::vector<double> &input, std::vector<double> &output);

    void evaluate_recurrent(const std::vector<double> &input, std::vector<double> &output);
public:
    explicit RaceNet() = default;

    void from_genome(const genome &a);

    void evaluate(const std::vector<double> &input, std::vector<double> &output);

    void import_fromfile(std::string filename);

    void export_tofile(std::string filename);
};



