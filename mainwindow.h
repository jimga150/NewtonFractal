#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <complex>
#include <cfloat>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>
#include <QtConcurrent>

#include "CustomScene.h"

struct line_item_struct {
    int row;
    QRgb* line_ptr;
    line_item_struct(int row, QRgb* line_ptr){
        this->row = row;
        this->line_ptr = line_ptr;
    }
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef std::complex<double> complex;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateImage();

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


    CustomScene scene;

    QImage current_img;

    const double coord_to_ui_scale = 50.0;
    const double ui_to_coord_scale = 1.0/coord_to_ui_scale;

    QTransform coord_to_ui_tform;
    QTransform ui_to_coord_tform;

    QGraphicsPixmapItem* pixmap_item;

    std::vector<complex> roots;
    bool root_is_selected = false;
    uint current_root_selected = 0;

    std::vector<complex> derivative_coefs;
    std::vector<complex> fxn_coefs;

    int num_iterations = 3;

    std::vector<QColor> colors;

public slots:
    void clicked(QPoint p);

    void dragged(QPoint p);

    void released();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
