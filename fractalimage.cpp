#include "fractalimage.h"

FractalImage::FractalImage(QObject *parent)
    : QObject{parent}
{

    this->image = QImage(500, 500, QImage::Format_ARGB32);
    this->image.fill(Qt::GlobalColor::black);

    this->coord_to_ui_tform.translate(this->image.width()/2, this->image.height()/2);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);
    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

    this->colors.push_back(QColor(207, 73, 20));
    this->colors.push_back(QColor(249, 172, 59));
    this->colors.push_back(QColor(116, 139, 50));
    this->colors.push_back(QColor(152, 89, 20));
    this->colors.push_back(QColor(207, 178, 132));
    this->colors.push_back(QColor(44, 116, 139));

    for (int y = 0; y < this->image.height(); ++y){

        QRgb* image_line = reinterpret_cast<QRgb*>(this->image.scanLine(y));

        for (int x = 0; x < this->image.width(); ++x){

            QPointF pixel_pt(x, y);
            QPointF coord_pt = this->ui_to_coord_tform.map(pixel_pt);
            complex coord = qpointfToComplex(coord_pt);

            this->fractal_pixels.push_back(FractalPixel(&image_line[x], coord, &this->poly_fxn, &this->colors));

        }
    }

    this->updateImage();
}

void FractalImage::updateImage(){

//    QElapsedTimer timer;
//    timer.start();

    this->poly_fxn.prepFunctionDerivative();

//    printf("prep function derivative took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();

//    this->image.fill(Qt::GlobalColor::black);

//    QList<QFuture<void>> threads;

//    for (int y = 0; y < this->image.height(); y = ++y){
//        QFuture<void> thread = QtConcurrent::run(&FractalImage::updateImageLine, this, y);
//        threads.append(thread);
//    }

//    for (QFuture<void> thread : threads){
//        thread.waitForFinished();
//    }

    QtConcurrent::blockingMap(this->fractal_pixels, &FractalPixel::update);

//    printf("fractal fill took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();
}

void FractalImage::updateImageLine(int y){

    int first = y*this->image.width();
    int end = (y+1)*this->image.width();

    for (int i = first; i < end; ++i){
        this->fractal_pixels.at(i).update();
    }
}

