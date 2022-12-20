#ifndef FRACTALPIXEL_H
#define FRACTALPIXEL_H

#include <QRgb>
#include <QColor>

#include "polynomial.h"

class FractalPixel
{
public:
    FractalPixel(QRgb* pixel, complex coord, Polynomial* fxn, std::vector<QColor>* colors);

    void update();

    QRgb* pixel;

    complex coord;

    Polynomial* fxn;

    std::vector<QColor>* colors;
};

#endif // FRACTALPIXEL_H
