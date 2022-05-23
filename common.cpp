#include "common.h"

complex qpointfToComplex(QPointF p){
    //x is the real axis, y is the imaginary axis
    return complex(p.x(), -p.y());
}

QPointF complexToQPointF(complex c){
    //x is the real axis, y is the imaginary axis
    return QPointF(c.real(), -c.imag());
}
