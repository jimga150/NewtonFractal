#ifndef FRACTALIMAGE_H
#define FRACTALIMAGE_H

#include <complex>
#include <cfloat>

#include <QObject>
#include <QImage>
#include <QtConcurrent>

typedef std::complex<double> complex;

struct line_item_struct {
    int row;
    QRgb* line_ptr;
    line_item_struct(int row, QRgb* line_ptr){
        this->row = row;
        this->line_ptr = line_ptr;
    }
};

class FractalImage : public QObject
{
    Q_OBJECT
public:
    explicit FractalImage(QObject *parent = nullptr);

    void updateImage();

    void updateImageLine_wstruct(line_item_struct line_item);

    void updateImageLine(int y, QRgb* lines);

    uint findRoot(complex x);

    complex qpointfToComplex(QPointF p);

    QPointF complexToQPointF(complex c);

    void prepFunctionDerivative();

    complex doFunction(complex x);

    complex doFunctionDerivative(complex x);

    std::vector<std::vector<complex>> getSets(std::vector<complex> list, uint set_size);

    uint nCr(uint n, uint r);

    uint factorial(uint n);

    void printList(std::vector<complex> list);

    void print2DList(std::vector<std::vector<complex>> list2d);


    QImage image;

    const double coord_to_ui_scale = 50.0;
    const double ui_to_coord_scale = 1.0/coord_to_ui_scale;

    QTransform coord_to_ui_tform;
    QTransform ui_to_coord_tform;

    std::vector<complex> roots;

    std::vector<complex> derivative_coefs;
    std::vector<complex> fxn_coefs;

    int num_iterations = 3;

    std::vector<QColor> colors;

signals:

};

#endif // FRACTALIMAGE_H
