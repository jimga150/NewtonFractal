#include "fractalpixel.h"

FractalPixel::FractalPixel(QRgb *pixel, complex coord, Polynomial *fxn, std::vector<QColor> *colors)
    : pixel(pixel), coord(coord), fxn(fxn), colors(colors)
{

}

void FractalPixel::update(){
    uint closest_root = this->fxn->findRoot(this->coord);
    *this->pixel = this->colors->at(closest_root).rgb();
}
