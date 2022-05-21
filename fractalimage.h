#ifndef FRACTALIMAGE_H
#define FRACTALIMAGE_H

#include <cfloat>

#include <QObject>
#include <QImage>
#include <QtConcurrent>

#include "common.h"
#include "fractalpixel.h"
#include "polynomial.h"

#define USE_MAP

#ifndef USE_MAP

#define USE_RUN

#endif

class FractalImage : public QObject
{
    Q_OBJECT
public:
    explicit FractalImage(QObject *parent = nullptr);

    void updateImage();

    void updateImageLine(int y);


    QImage image;

    const double coord_to_ui_scale = 50.0;
    const double ui_to_coord_scale = 1.0/coord_to_ui_scale;

    QTransform coord_to_ui_tform;
    QTransform ui_to_coord_tform;

    std::vector<QColor> colors;

    std::vector<FractalPixel> fractal_pixels;

    Polynomial poly_fxn;

signals:

};

#endif // FRACTALIMAGE_H
