#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPainter>
#include <QtConcurrent>

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


    CustomScene scene;

    QGraphicsPixmapItem* pixmap_item;

    FractalImage fractal;

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
