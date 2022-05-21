#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include <cfloat>

#include <QPointF>

#include "common.h"

class Polynomial
{
public:
    Polynomial();

    void changeNumRoots(int num_roots);

    void changeNumIters(int num_iters);

    uint findRoot(complex x);

    void prepFunctionDerivative();

    complex doFunction(complex x);

    complex doFunctionDerivative(complex x);

    std::vector<std::vector<complex>> getSets(std::vector<complex> list, uint set_size);

    uint nCr(uint n, uint r);

    uint factorial(uint n);

    void printList(std::vector<complex> list);

    void print2DList(std::vector<std::vector<complex>> list2d);


    std::vector<complex> roots;

    std::vector<complex> derivative_coefs;
    std::vector<complex> fxn_coefs;

    int num_iterations = 3;

};

#endif // POLYNOMIAL_H
