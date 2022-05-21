#ifndef COMMON_H
#define COMMON_H

#include <complex>

#include <QPointF>

typedef std::complex<double> complex;

complex qpointfToComplex(QPointF p);
QPointF complexToQPointF(complex c);

#endif // COMMON_H
