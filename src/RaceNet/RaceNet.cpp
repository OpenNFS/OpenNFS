//
// Created by Amrik on 28/10/2018.
//

#include "RaceNet.h"

using namespace std;

double stepFunction(double x)
{
    if(x>0.9){
        return 1.0;
    }
    if(x<0.1){
        return 0.0;
    }
    return x;
}

RaceNet::RaceNet() : net(layerParams, learningRate) {}

vector<double> RaceNet::Infer(vector<double> raycastInputs) {
        // as the sigmoid function never reaches 0.0 nor 1.0
        // it can be a good idea to consider values greater than 0.9 as 1.0 and values smaller than 0.1 as 0.0
        // hence the step function.
        Matrix<double> resultMatrix = net.computeOutput(raycastInputs).applyFunction(stepFunction);
        std::vector<double> resultVector;

        resultVector.emplace_back(resultMatrix.get(0, 0));
        resultVector.emplace_back(resultMatrix.get(0, 1));
        resultVector.emplace_back(resultMatrix.get(0, 2));
        //resultVector.emplace_back(resultMatrix.get(0, 3));

        return resultVector;
}
