//
// https://github.com/OmarAflak/Neural-Network
//

#include "Network.h"

Network::Network(std::vector<int> neurons, double learningRate) {
    srand(time(NULL));

    this->learningRate = learningRate;
    this->hiddenLayersCount = neurons.size() - 2;

    H = std::vector<Matrix<double>>(hiddenLayersCount + 2);
    W = std::vector<Matrix<double>>(hiddenLayersCount + 1);
    B = std::vector<Matrix<double>>(hiddenLayersCount + 1);
    dEdW = std::vector<Matrix<double>>(hiddenLayersCount + 1);
    dEdB = std::vector<Matrix<double>>(hiddenLayersCount + 1);

    for (int i = 0; i < neurons.size() - 1; i++) {
        W[i] = Matrix<double>(neurons[i], neurons[i + 1]);
        B[i] = Matrix<double>(1, neurons[i + 1]);

        W[i] = W[i].applyFunction(random);
        B[i] = B[i].applyFunction(random);
    }
}

Network::Network(const char *filepath) { loadNetworkParams(filepath); }

Matrix<double> Network::computeOutput(std::vector<double> input) {
    H[0] = Matrix<double>({input}); // row matrix

    for (int i = 1; i < hiddenLayersCount + 2; i++) {
        H[i] = H[i - 1].dot(W[i - 1]).add(B[i - 1]).applyFunction(sigmoid);
    }

    return H[hiddenLayersCount + 1];
}

void Network::learn(std::vector<double> expectedOutput) {
    Y = Matrix<double>({expectedOutput}); // row matrix

    // Error E = 1/2 (expectedOutput - computedOutput)^2
    // Then, we need to calculate the partial derivative of E with respect to W and B

    // compute gradients
    dEdB[hiddenLayersCount] = H[hiddenLayersCount + 1].subtract(Y).multiply(
        H[hiddenLayersCount].dot(W[hiddenLayersCount]).add(B[hiddenLayersCount]).applyFunction(sigmoidePrime));
    for (int i = hiddenLayersCount - 1; i >= 0; i--) {
        dEdB[i] = dEdB[i + 1].dot(W[i + 1].transpose()).multiply(H[i].dot(W[i]).add(B[i]).applyFunction(sigmoidePrime));
    }

    for (int i = 0; i < hiddenLayersCount + 1; i++) {
        dEdW[i] = H[i].transpose().dot(dEdB[i]);
    }

    // update weights
    for (int i = 0; i < hiddenLayersCount + 1; i++) {
        W[i] = W[i].subtract(dEdW[i].multiply(learningRate));
        B[i] = B[i].subtract(dEdB[i].multiply(learningRate));
    }
}

void Network::printToFile(Matrix<double> &m, std::ofstream &file) {
    int h = m.getHeight();
    int w = m.getWidth();

    file << h << std::endl;
    file << w << std::endl;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            file << m.get(i, j) << (j != w - 1 ? " " : "");
        }
        file << std::endl;
    }
}

void Network::saveNetworkParams(const char *filepath) {
    std::ofstream out(filepath);

    out << hiddenLayersCount << std::endl;
    out << learningRate << std::endl;

    for (Matrix<double> m : W) {
        printToFile(m, out);
    }

    for (Matrix<double> m : B) {
        printToFile(m, out);
    }

    out.close();
}

void Network::loadNetworkParams(const char *filepath) {
    std::ifstream in(filepath);
    std::vector<Matrix<double>> params;
    double val;
    int h, w;

    if (in) {
        in >> hiddenLayersCount;
        in >> learningRate;

        H = std::vector<Matrix<double>>(hiddenLayersCount + 2);
        W = std::vector<Matrix<double>>(hiddenLayersCount + 1);
        B = std::vector<Matrix<double>>(hiddenLayersCount + 1);
        dEdW = std::vector<Matrix<double>>(hiddenLayersCount + 1);
        dEdB = std::vector<Matrix<double>>(hiddenLayersCount + 1);

        for (int i = 0; i < 2 * hiddenLayersCount + 2; i++) {
            in >> h;
            in >> w;
            Matrix<double> m(h, w);
            for (int hh = 0; hh < h; hh++) {
                for (int ww = 0; ww < w; ww++) {
                    in >> val;
                    m.put(hh, ww, val);
                }
            }

            params.push_back(m);
        }
    }
    in.close();

    // assign values
    for (int i = 0; i < hiddenLayersCount + 1; i++) {
        W[i] = params[i];
    }

    for (int i = hiddenLayersCount + 1; i < params.size(); i++) {
        B[i - hiddenLayersCount - 1] = params[i];
    }
}

double Network::random(double x) { return (double)(rand() % 10000 + 1) / 10000 - 0.5; }

double Network::sigmoid(double x) { return 1 / (1 + exp(-x)); }

double Network::sigmoidePrime(double x) { return exp(-x) / (pow(1 + exp(-x), 2)); }