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

    int h_step = 360/10;
    for (int i = 0; i < 10; ++i){ //TODO: make max roots a constant
        this->colors.push_back(QColor::fromHsv(i*h_step, 150, 200));
    }

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

#ifdef USE_RUN

    QList<QFuture<void>> threads;

    for (int y = 0; y < this->image.height(); y = ++y){
        QFuture<void> thread = QtConcurrent::run(&FractalImage::updateImageLine, this, y);
        threads.append(thread);
    }

    for (QFuture<void> thread : threads){
        thread.waitForFinished();
    }

#elif defined (USE_MAP)

    QtConcurrent::blockingMap(this->fractal_pixels, &FractalPixel::update);

#endif

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

