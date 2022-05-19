#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <complex>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>

#include "CustomScene.h"

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

    complex qpointfToComplex(QPointF p);

    QPointF complexToQPointF(complex c);


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

public slots:
    void clicked(QPoint p);

    void dragged(QPoint p);

    void released();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
