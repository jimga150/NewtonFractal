#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this->ui->center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_real_spinbox_valueChanged);
    connect(this->ui->center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_imag_spinbox_valueChanged);

    this->fractal.setImageSize(this->ui->graphicsView->size());

    this->pixmap_item = this->scene.addPixmap(QPixmap::fromImage(this->fractal.image));

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    connect(&this->scene, &CustomScene::pixelClicked, this, &MainWindow::clicked);
    connect(&this->scene, &CustomScene::mouseDraggedTo, this, &MainWindow::dragged);
    connect(&this->scene, &CustomScene::mouseReleased, this, &MainWindow::released);

    connect(this->ui->graphicsView, &CustomGraphicsView::resized, this, &MainWindow::imageResized);

    this->generateRootSpinBoxes();
    this->updateImage();

//    for(int nr = 3; nr <= 10; ++nr){
//        this->numRootsChanged(nr, false);
//        for (int iters = 1; iters < 20; ++iters){
//            this->numItersChanged(iters);
//            for (int i = 0; i < 20; ++i){
//                this->updateImage();
//            }
//        }
//    }
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

    for (complex c : *this->fractal.getRoots()){
        QPointF center = complexToQPointF(c);
        //adjust to always be the same size on screen
        painter.drawEllipse(center, 10*this->fractal.ui_to_coord_scale, 10*this->fractal.ui_to_coord_scale);
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
        printf("Average update was %f ms long (roots: %lu, iterations: %d)\n", avg_time_ns/(1000*1000), this->fractal.getRoots()->size(), this->fractal.getNumIters());
    }
    this->update_times_ns.clear();
    fflush(stdout);
}

void MainWindow::clicked(QPointF p){
//    printf("click gotten at pixel (%f, %f)\n", p.x(), p.y());
//    fflush(stdout);

    QPointF pf = p;
    QPointF coord = this->fractal.ui_to_coord_tform.map(pf);

    this->root_is_selected = false;

    for (uint i = 0; i < this->fractal.getRoots()->size(); ++i){
        complex r = this->fractal.getRoots()->at(i);
        QPointF root_pt = complexToQPointF(r);
        double x_diff = root_pt.x() - coord.x();
        double y_diff = root_pt.y() - coord.y();
        double dist = sqrt(x_diff*x_diff + y_diff*y_diff);
        if (dist < 10*this->fractal.ui_to_coord_scale){
            this->current_root_selected = i;
            this->root_is_selected = true;
//            printf("picked root %d\n", i);
            break;
        }
    }

    if (!this->root_is_selected){
        this->dragging_view = true;
        this->pixel_dragging = p;
    }

}

void MainWindow::dragged(QPointF p){
//    printf("drag gotten at pixel (%f, %f)\n", p.x(), p.y());
//    fflush(stdout);

    if (this->dragging_view){
        QPointF diff = this->pixel_dragging - p;
        if (diff.isNull()) return;
        QPointF diff_scaled = diff*this->fractal.ui_to_coord_scale;
//        this->ui->graphicsView->setSceneRect(this->ui->graphicsView->sceneRect().translated(diff));
        complex diff_cpx = qpointfToComplex(diff_scaled);
        complex old_center = this->fractal.getCenter();
        complex new_center = old_center - diff_cpx;
        this->fractal.setCenter(new_center);

        disconnect(this->ui->center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_real_spinbox_valueChanged);
        this->ui->center_real_spinbox->setValue(new_center.real());
        connect(this->ui->center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_real_spinbox_valueChanged);

        disconnect(this->ui->center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_imag_spinbox_valueChanged);
        this->ui->center_imag_spinbox->setValue(new_center.imag());
        connect(this->ui->center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::center_imag_spinbox_valueChanged);

        this->updateImage();
        this->pixel_dragging = p;
        return;
    }

    if (!root_is_selected) return;

    QPointF pf = p;
    QPointF coord_pt = this->fractal.ui_to_coord_tform.map(pf);
    complex coord_cpx = qpointfToComplex(coord_pt);

    QDoubleSpinBox* real_spinbox = static_cast<QDoubleSpinBox*>(this->ui->rootEditGridLayout->itemAtPosition(this->current_root_selected, 1)->widget());
    disconnect(this->root_real_edit_connections.at(this->current_root_selected));
    real_spinbox->setValue(coord_cpx.real());
    this->root_real_edit_connections[this->current_root_selected] = connect(real_spinbox, &QDoubleSpinBox::valueChanged, this,
            [=](double new_val){this->root_real_spinbox_changed(this->current_root_selected, new_val);});

    QDoubleSpinBox* imag_spinbox = static_cast<QDoubleSpinBox*>(this->ui->rootEditGridLayout->itemAtPosition(this->current_root_selected, 3)->widget());
    disconnect(this->root_imag_edit_connections.at(this->current_root_selected));
    imag_spinbox->setValue(coord_cpx.imag());
//    printf("setting imaginary spinbox %d to %f\n",this->current_root_selected, coord_cpx.imag());
    this->root_imag_edit_connections[this->current_root_selected] = connect(imag_spinbox, &QDoubleSpinBox::valueChanged, this,
            [=](double new_val){this->root_imag_spinbox_changed(this->current_root_selected, new_val);});

    this->fractal.getRoots()->at(this->current_root_selected) = coord_cpx;

    this->updateImage();
}

void MainWindow::released(){
//    printf("mouse released\n");
//    fflush(stdout);
    this->root_is_selected = false;
    this->dragging_view = false;
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

void MainWindow::numRootsChanged(int nr, bool update_img){

    this->printAvgUpdateTime();
    this->fractal.setNumRoots(nr);
    this->generateRootSpinBoxes();
    if (update_img) this->updateImage();
}

void MainWindow::generateRootSpinBoxes(){

    for (const QMetaObject::Connection &connection : this->root_real_edit_connections){
        disconnect(connection);
    }
    this->root_real_edit_connections.clear();

    for (const QMetaObject::Connection &connection : this->root_imag_edit_connections){
        disconnect(connection);
    }
    this->root_imag_edit_connections.clear();

    for (QWidget* widget_item : this->root_edit_items){
        this->ui->rootEditGridLayout->removeWidget(widget_item);
        widget_item->deleteLater();
    }
    this->root_edit_items.clear();

    std::vector<complex>* roots = this->fractal.getRoots();
    for (uint i = 0; i < roots->size(); ++i){
        complex r = roots->at(i);

        QLabel* real_label = new QLabel("Real:");
        this->ui->rootEditGridLayout->addWidget(real_label, i, 0);
        this->root_edit_items.push_back(real_label);

        QDoubleSpinBox* real_spinbox = new QDoubleSpinBox();
        real_spinbox->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
        real_spinbox->setDecimals(6);
        real_spinbox->setMaximum(DBL_MAX);
        real_spinbox->setMinimum(-DBL_MAX);
        real_spinbox->setValue(r.real());
        this->ui->rootEditGridLayout->addWidget(real_spinbox, i, 1);
        this->root_edit_items.push_back(real_spinbox);
        this->root_real_edit_connections.append(connect(real_spinbox, &QDoubleSpinBox::valueChanged, this,
                [=](double new_val){this->root_real_spinbox_changed(i, new_val);}));

        QLabel* imag_label = new QLabel("Imag:");
        this->ui->rootEditGridLayout->addWidget(imag_label, i, 2);
        this->root_edit_items.push_back(imag_label);

        QDoubleSpinBox* imag_spinbox = new QDoubleSpinBox();
        imag_spinbox->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
        imag_spinbox->setDecimals(6);
        imag_spinbox->setMaximum(DBL_MAX);
        imag_spinbox->setMinimum(-DBL_MAX);
        imag_spinbox->setValue(r.imag());
        this->ui->rootEditGridLayout->addWidget(imag_spinbox, i, 3);
        this->root_edit_items.push_back(imag_spinbox);
        this->root_imag_edit_connections.append(connect(imag_spinbox, &QDoubleSpinBox::valueChanged, this,
                [=](double new_val){this->root_imag_spinbox_changed(i, new_val);}));
    }
}

void MainWindow::on_num_iter_hslider_sliderMoved(int newval)
{
    this->ui->num_iter_spinbox->setValue(newval);
    this->numItersChanged(newval);
}


void MainWindow::on_num_iter_spinbox_editingFinished()
{
    int new_val = this->ui->num_iter_spinbox->value();
    this->ui->num_iter_hslider->setValue(new_val);
    this->numItersChanged(new_val);
}

void MainWindow::root_real_spinbox_changed(int root_index, double new_val){
    this->fractal.getRoots()->at(root_index).real(new_val);
    this->updateImage();
}

void MainWindow::root_imag_spinbox_changed(int root_index, double new_val){
    this->fractal.getRoots()->at(root_index).imag(new_val);
    this->updateImage();
}

void MainWindow::numItersChanged(int ni, bool update_img){
    this->printAvgUpdateTime();
    this->fractal.setNumIters(ni);
    if (update_img) this->updateImage();
}


void MainWindow::on_scale_spinbox_valueChanged(double arg1){
    this->ui->scale_spinbox->setSingleStep(0.1*arg1);
    this->fractal.setScale(arg1);
    this->updateImage();
}


void MainWindow::center_real_spinbox_valueChanged(double arg1){
    this->fractal.setCenterReal(arg1);
    this->updateImage();
}

void MainWindow::center_imag_spinbox_valueChanged(double arg1){
    this->fractal.setCenterImag(arg1);
    this->updateImage();
}

void MainWindow::imageResized(QSize size){
    this->fractal.setImageSize(size);
    this->updateImage();
}
