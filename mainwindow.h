#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>
#include <QLayoutItem>

#include "customscene.h"
#include "fractalimage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateImage();

    void numRootsChanged(int nr, bool update_img = true);

    void generateRootSpinBoxes();

    void numItersChanged(int ni, bool update_img = true);

    void printAvgUpdateTime();


    CustomScene scene;

    QGraphicsPixmapItem* pixmap_item;

    bool dragging_view = false;
    QPointF pixel_dragging;

    QList<QWidget*> root_edit_items;
    QList<QMetaObject::Connection> root_real_edit_connections;
    QList<QMetaObject::Connection> root_imag_edit_connections;

    FractalImage fractal;

    bool root_is_selected = false;
    uint current_root_selected = 0;

    std::vector<unsigned long long> update_times_ns;


public slots:
    void clicked(QPointF p);

    void dragged(QPointF p);

    void released();

private slots:
    void on_num_roots_hslider_sliderReleased();

    void on_num_roots_spinbox_editingFinished();

    void on_num_iter_hslider_sliderMoved(int newval);

    void on_num_iter_spinbox_editingFinished();

    void root_real_spinbox_changed(int root_index, double new_val);

    void root_imag_spinbox_changed(int root_index, double new_val);

    void on_scale_spinbox_valueChanged(double arg1);

    void center_real_spinbox_valueChanged(double arg1);

    void center_imag_spinbox_valueChanged(double arg1);

    void imageResized(QSize size);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
