#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>
#include <QGridLayout>
#include <QFileDialog>
#include <QImageWriter>
#include <QApplication>

#include "customscene.h"
#include "fractalimage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum root_edit_row_idx_enum{
    CARTESIAN_ROW = 0,
    POLAR_ROW,
    COLOR_ROW,

    num_root_edit_rows
};

enum root_edit_cart_col_idx_enum{
    REAL_LABEL_COL = 0,
    REAL_SPINBOX_COL,
    IMAG_LABEL_COL,
    IMAG_SPINBOX_COL,

    num_root_edit_cart_cols
};

enum root_edit_polar_col_idx_enum{
    R_LABEL_COL = 0,
    R_SPINBOX_COL,
    THETA_LABEL_COL,
    THETA_SPINBOX_COL,

    num_root_edit_polar_cols
};

enum root_color_edit_col_idx_enum {
    COLOR_COL = 0,
    RED_LABEL_COL,
    RED_SPINBOX_COL,
    GREEN_LABEL_COL,
    GREEN_SPINBOX_COL,
    BLUE_LABEL_COL,
    BLUE_SPINBOX_COL,

    num_root_color_edit_cols
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateImage();

    void updateCartSpinBoxes(int root_index, complex new_val);

    void updateRealSpinBox(int root_index, double new_val);

    void updateImagSpinBox(int root_index, double new_val);

    void updatePolarSpinBoxes(int root_index, complex new_val);

    void updateRSpinBox(int root_index, double new_val);

    void updateThetaSpinBox(int root_index, double new_val);

    void numRootsChanged(int nr, bool update_img = true);

    void generateRootSpinBoxes();

    void numItersChanged(int ni, bool update_img = true);

    void printAvgUpdateTime();

    inline int getRootCartEditRow(int root_index){
        return num_root_edit_rows*root_index + CARTESIAN_ROW;
    }

    inline int getRootPolarEditRow(int root_index){
        return num_root_edit_rows*root_index + POLAR_ROW;
    }

    inline int getRootColorEditRow(int root_index){
        return num_root_edit_rows*root_index + COLOR_ROW;
    }

    void updateRootColorSample(int root_index);


    CustomScene scene;

    double image_scale = 1;
    int image_downscaling_factor = 1;

    QGraphicsPixmapItem* pixmap_item;

    bool dragging_view = false;
    QPointF pixel_dragging;

    QList<QWidget*> root_edit_items;
    QList<QMetaObject::Connection> root_real_edit_connections;
    QList<QMetaObject::Connection> root_imag_edit_connections;
    QList<QMetaObject::Connection> root_r_edit_connections;
    QList<QMetaObject::Connection> root_theta_edit_connections;

    QList<QGridLayout*> root_color_edit_layouts;
    QList<QMetaObject::Connection> root_red_edit_connections;
    QList<QMetaObject::Connection> root_green_edit_connections;
    QList<QMetaObject::Connection> root_blue_edit_connections;

    FractalImage fractal;

    bool render_roots = true;
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

    void on_num_iter_hslider_sliderMoved(int new_val);

    void on_num_iter_spinbox_editingFinished();

    void root_real_spinbox_changed(int root_index, double new_val);

    void root_imag_spinbox_changed(int root_index, double new_val);

    void root_r_spinbox_changed(int root_index, double new_val);

    void root_theta_spinbox_changed(int root_index, double new_val);

    void root_red_spinbox_changed(int root_index, int new_val);

    void root_green_spinbox_changed(int root_index, int new_val);

    void root_blue_spinbox_changed(int root_index, int new_val);

    void on_coord_scale_spinbox_valueChanged(double arg1);

    void coord_center_real_spinbox_valueChanged(double arg1);

    void coord_center_imag_spinbox_valueChanged(double arg1);

    void imageResized(QSize size);

    void on_set_bounding_view_button_clicked();

    void on_image_scale_spinbox_valueChanged(double arg1);

    void on_image_width_spinbox_editingFinished();

    void on_image_height_spinbox_editingFinished();

    void on_save_button_clicked();

    void on_render_roots_checkbox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
