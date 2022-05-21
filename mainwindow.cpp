#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->pixmap_item = this->scene.addPixmap(QPixmap::fromImage(this->fractal.image));

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    connect(&this->scene, &CustomScene::pixelClicked, this, &MainWindow::clicked);
    connect(&this->scene, &CustomScene::mouseDraggedTo, this, &MainWindow::dragged);
    connect(&this->scene, &CustomScene::mouseReleased, this, &MainWindow::released);

    this->updateImage();
}

MainWindow::~MainWindow()
{
    this->printAvgUpdateTime();
    delete ui;
}

void MainWindow::updateImage(){
    QElapsedTimer timer;
    timer.start();

    this->fractal.updateImage();

    QPainter painter(&this->fractal.image);

    QPen pen(QBrush(Qt::GlobalColor::white), this->fractal.ui_to_coord_scale);
    pen.setColor(Qt::GlobalColor::white);
    painter.setPen(pen);

    painter.setTransform(this->fractal.coord_to_ui_tform);

    for (complex r : this->fractal.poly_fxn.roots){
        QPointF center = complexToQPointF(r);
        painter.drawEllipse(center, 0.1, 0.1);
//        printf("Drawing circle at (%f, %f)\n", center.x(), center.y());
    }

    painter.end();

    this->pixmap_item->setPixmap(QPixmap::fromImage(this->fractal.image));

    unsigned long long nanos = timer.nsecsElapsed();
    this->update_times_ns.push_back(nanos);
//    printf("update image took %lld ns\n", nanos);
}

void MainWindow::printAvgUpdateTime(){
    if (this->update_times_ns.size() > 0){
        double avg_time_ns = 0;
        for (unsigned long long time_ns : this->update_times_ns){
            avg_time_ns += time_ns*1.0/this->update_times_ns.size();
        }
        printf("Average update was %f ns long\n", avg_time_ns);
    }
    this->update_times_ns.clear();
    fflush(stdout);
}

void MainWindow::clicked(QPoint p){
//    printf("click gotten at pixel (%d, %d)\n", p.x(), p.y());
//    fflush(stdout);

    QPointF pf = p;
    QPointF coord = this->fractal.ui_to_coord_tform.map(pf);

    this->root_is_selected = false;

    for (uint i = 0; i < this->fractal.poly_fxn.roots.size(); ++i){
        complex r = this->fractal.poly_fxn.roots.at(i);
        QPointF root_pt = complexToQPointF(r);
        double x_diff = root_pt.x() - coord.x();
        double y_diff = root_pt.y() - coord.y();
        double dist = sqrt(x_diff*x_diff + y_diff*y_diff);
        if (dist < 0.1){
            this->current_root_selected = i;
            this->root_is_selected = true;
//            printf("picked root %d\n", i);
            break;
        }
    }

}

void MainWindow::dragged(QPoint p){
//    printf("drag gotten at pixel (%d, %d)\n", p.x(), p.y());
//    fflush(stdout);

    if (!root_is_selected) return;

    QPointF pf = p;
    QPointF coord = this->fractal.ui_to_coord_tform.map(pf);

    this->fractal.poly_fxn.roots.at(this->current_root_selected) = qpointfToComplex(coord);

    this->updateImage();
}

void MainWindow::released(){
//    printf("mouse released\n");
//    fflush(stdout);
}

void MainWindow::on_num_roots_hslider_sliderReleased()
{
    int new_val = this->ui->num_roots_hslider->value();
    this->ui->num_roots_spinbox->setValue(new_val);
    this->numRootsChanged(new_val);
}


void MainWindow::on_num_roots_spinbox_editingFinished()
{
    int new_val = this->ui->num_roots_spinbox->value();
    this->ui->num_roots_hslider->setValue(new_val);
    this->numRootsChanged(new_val);
}

void MainWindow::numRootsChanged(int nr){
    this->printAvgUpdateTime();
    this->fractal.poly_fxn.changeNumRoots(nr); //TODO: dont expose member instances, use getters and setters
    this->updateImage();
}


void MainWindow::on_num_iter_hslider_sliderReleased()
{
    int new_val = this->ui->num_iter_hslider->value();
    this->ui->num_iter_spinbox->setValue(new_val);
    this->numItersChanged(new_val);
}


void MainWindow::on_num_iter_spinbox_editingFinished()
{
    int new_val = this->ui->num_iter_spinbox->value();
    this->ui->num_iter_hslider->setValue(new_val);
    this->numItersChanged(new_val);
}

void MainWindow::numItersChanged(int ni){
    this->printAvgUpdateTime();
    this->fractal.poly_fxn.changeNumIters(ni); //TODO: dont expose member instances, use getters and setters
    this->updateImage();
}

