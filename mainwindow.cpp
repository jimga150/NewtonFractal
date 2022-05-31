#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this->ui->coord_center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_real_spinbox_valueChanged);
    connect(this->ui->coord_center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_imag_spinbox_valueChanged);

    QSize gv_size = this->ui->graphicsView->size();
    this->fractal.setImageSize(gv_size);
    this->ui->image_height_spinbox->setValue(gv_size.height());
    this->ui->image_width_spinbox->setValue(gv_size.width());

    this->fractal.setNumRoots(5);

    QPixmap fractal_pixmap = QPixmap::fromImage(this->fractal.image);
//    fractal_pixmap.setDevicePixelRatio(2.0);
    this->pixmap_item = this->scene.addPixmap(fractal_pixmap);

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    connect(&this->scene, &CustomScene::pixelClicked, this, &MainWindow::clicked);
    connect(&this->scene, &CustomScene::mouseDraggedTo, this, &MainWindow::dragged);
    connect(&this->scene, &CustomScene::mouseReleased, this, &MainWindow::released);

//    connect(this->ui->graphicsView, &CustomGraphicsView::resized, this, &MainWindow::imageResized);

    this->updateImage();
    this->generateRootSpinBoxes();

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

    QPixmap fractal_pixmap = QPixmap::fromImage(this->fractal.image);
//    fractal_pixmap.setDevicePixelRatio(2.0);
    this->pixmap_item->setPixmap(fractal_pixmap);

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

        disconnect(this->ui->coord_center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_real_spinbox_valueChanged);
        this->ui->coord_center_real_spinbox->setValue(new_center.real());
        connect(this->ui->coord_center_real_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_real_spinbox_valueChanged);

        disconnect(this->ui->coord_center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_imag_spinbox_valueChanged);
        this->ui->coord_center_imag_spinbox->setValue(new_center.imag());
        connect(this->ui->coord_center_imag_spinbox, &QDoubleSpinBox::valueChanged, this, &MainWindow::coord_center_imag_spinbox_valueChanged);

        this->updateImage();
        this->pixel_dragging = p;
        return;
    }

    if (!root_is_selected) return;

    QPointF pf = p;
    QPointF coord_pt = this->fractal.ui_to_coord_tform.map(pf);
    complex coord_cpx = qpointfToComplex(coord_pt);
    int root_row = this->getRootEditRow(this->current_root_selected);

    QDoubleSpinBox* real_spinbox = static_cast<QDoubleSpinBox*>(this->ui->rootEditGridLayout->itemAtPosition(root_row, REAL_SPINBOX_COL)->widget());
    disconnect(this->root_real_edit_connections.at(this->current_root_selected));
    real_spinbox->setValue(coord_cpx.real());
    this->root_real_edit_connections[this->current_root_selected] = connect(real_spinbox, &QDoubleSpinBox::valueChanged, this,
            [=](double new_val){this->root_real_spinbox_changed(this->current_root_selected, new_val);});

    QDoubleSpinBox* imag_spinbox = static_cast<QDoubleSpinBox*>(this->ui->rootEditGridLayout->itemAtPosition(root_row, IMAG_SPINBOX_COL)->widget());
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
    if (update_img) this->updateImage();
    this->generateRootSpinBoxes();

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

    for (const QMetaObject::Connection &connection : this->root_red_edit_connections){
        disconnect(connection);
    }
    this->root_red_edit_connections.clear();

    for (const QMetaObject::Connection &connection : this->root_green_edit_connections){
        disconnect(connection);
    }
    this->root_green_edit_connections.clear();

    for (const QMetaObject::Connection &connection : this->root_blue_edit_connections){
        disconnect(connection);
    }
    this->root_blue_edit_connections.clear();

    for (QGridLayout* layout : this->root_color_edit_layouts){
        QLayoutItem *item;
        while((item = layout->takeAt(0))) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
//        printf("Deleted color layout %p\n", layout);
    }
    this->root_color_edit_layouts.clear();

    std::vector<complex>* roots = this->fractal.getRoots();
    for (uint i = 0; i < roots->size(); ++i){
        complex r = roots->at(i);

        int root_spinbox_row = this->getRootEditRow(i);
        int root_color_row = this->getRootColorEditRow(i);

        QLabel* real_label = new QLabel("Real:");
        this->ui->rootEditGridLayout->addWidget(real_label, root_spinbox_row, REAL_LABEL_COL);
        this->root_edit_items.push_back(real_label);

        QDoubleSpinBox* real_spinbox = new QDoubleSpinBox();
        real_spinbox->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
        real_spinbox->setDecimals(6);
        real_spinbox->setMaximum(DBL_MAX);
        real_spinbox->setMinimum(-DBL_MAX);
        real_spinbox->setValue(r.real());
        this->ui->rootEditGridLayout->addWidget(real_spinbox, root_spinbox_row, REAL_SPINBOX_COL);
        this->root_edit_items.push_back(real_spinbox);
        this->root_real_edit_connections.append(connect(real_spinbox, &QDoubleSpinBox::valueChanged, this,
                [=](double new_val){this->root_real_spinbox_changed(i, new_val);}));

        QLabel* imag_label = new QLabel("Imag:");
        this->ui->rootEditGridLayout->addWidget(imag_label, root_spinbox_row, IMAG_LABEL_COL);
        this->root_edit_items.push_back(imag_label);

        QDoubleSpinBox* imag_spinbox = new QDoubleSpinBox();
        imag_spinbox->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);
        imag_spinbox->setDecimals(6);
        imag_spinbox->setMaximum(DBL_MAX);
        imag_spinbox->setMinimum(-DBL_MAX);
        imag_spinbox->setValue(r.imag());
        this->ui->rootEditGridLayout->addWidget(imag_spinbox, root_spinbox_row, IMAG_SPINBOX_COL);
        this->root_edit_items.push_back(imag_spinbox);
        this->root_imag_edit_connections.append(connect(imag_spinbox, &QDoubleSpinBox::valueChanged, this,
                [=](double new_val){this->root_imag_spinbox_changed(i, new_val);}));

        QGridLayout* color_grid_layout = new QGridLayout();
//        printf("Made color layout %p\n", color_grid_layout);
        this->root_color_edit_layouts.append(color_grid_layout);

        QLabel* color_indicator_label = new QLabel();
        QColor col = this->fractal.colors.at(i);
        color_indicator_label->setStyleSheet(QString("QLabel { background-color : rgb(%1, %2, %3)}").arg(col.red()).arg(col.green()).arg(col.blue()));
//        printf("setting root %d label to %d, %d, %d\n", i, col.red(), col.green(), col.blue());
        color_grid_layout->addWidget(color_indicator_label, 0, COLOR_COL);

        QLabel* color_red_label = new QLabel("Red:");
        color_grid_layout->addWidget(color_red_label, 0, RED_LABEL_COL);

        QSpinBox* color_red_spinbox = new QSpinBox();
        color_red_spinbox->setMinimum(0);
        color_red_spinbox->setMaximum(255);
        color_red_spinbox->setValue(col.red());
        color_grid_layout->addWidget(color_red_spinbox, 0, RED_SPINBOX_COL);
        this->root_red_edit_connections.append(connect(color_red_spinbox, &QSpinBox::valueChanged, this, [=](int new_val){
            this->root_red_spinbox_changed(i, new_val);
        }));

        QLabel* color_green_label = new QLabel("Green:");
        color_grid_layout->addWidget(color_green_label, 0, GREEN_LABEL_COL);

        QSpinBox* color_green_spinbox = new QSpinBox();
        color_green_spinbox->setMinimum(0);
        color_green_spinbox->setMaximum(255);
        color_green_spinbox->setValue(col.green());
        color_grid_layout->addWidget(color_green_spinbox, 0, GREEN_SPINBOX_COL);
        this->root_green_edit_connections.append(connect(color_green_spinbox, &QSpinBox::valueChanged, this, [=](int new_val){
            this->root_green_spinbox_changed(i, new_val);
        }));

        QLabel* color_blue_label = new QLabel("Blue:");
        color_grid_layout->addWidget(color_blue_label, 0, BLUE_LABEL_COL);

        QSpinBox* color_blue_spinbox = new QSpinBox();
        color_blue_spinbox->setMinimum(0);
        color_blue_spinbox->setMaximum(255);
        color_blue_spinbox->setValue(col.blue());
        color_grid_layout->addWidget(color_blue_spinbox, 0, BLUE_SPINBOX_COL);
        this->root_blue_edit_connections.append(connect(color_blue_spinbox, &QSpinBox::valueChanged, this, [=](int new_val){
            this->root_blue_spinbox_changed(i, new_val);
        }));

        this->ui->rootEditGridLayout->addLayout(color_grid_layout, root_color_row, 0, 1, num_root_edit_cols);

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

void MainWindow::root_red_spinbox_changed(int root_index, int new_val){
    this->fractal.colors.at(root_index).setRed(new_val);
    this->updateImage();
    QColor col = this->fractal.colors.at(root_index);
    QGridLayout* layout = this->root_color_edit_layouts.at(root_index);
    QLabel* label = static_cast<QLabel*>(layout->itemAtPosition(0, COLOR_COL)->widget());
    label->setStyleSheet(QString("QLabel { background-color : rgb(%1, %2, %3)}").arg(col.red()).arg(col.green()).arg(col.blue()));
}

void MainWindow::root_green_spinbox_changed(int root_index, int new_val){
    this->fractal.colors.at(root_index).setGreen(new_val);
    this->updateImage();
}

void MainWindow::root_blue_spinbox_changed(int root_index, int new_val){
    this->fractal.colors.at(root_index).setBlue(new_val);
    this->updateImage();
}

void MainWindow::numItersChanged(int ni, bool update_img){
    this->printAvgUpdateTime();
    this->fractal.setNumIters(ni);
    if (update_img) this->updateImage();
}


void MainWindow::on_coord_scale_spinbox_valueChanged(double arg1){
    this->ui->coord_scale_spinbox->setSingleStep(0.1*arg1);
    this->fractal.setScale(arg1);
    this->updateImage();
}


void MainWindow::coord_center_real_spinbox_valueChanged(double arg1){
    this->fractal.setCenterReal(arg1);
    this->updateImage();
}

void MainWindow::coord_center_imag_spinbox_valueChanged(double arg1){
    this->fractal.setCenterImag(arg1);
    this->updateImage();
}

void MainWindow::imageResized(QSize size){
    QSizeF sizef(size);
    sizef.rwidth() /= this->image_scale;
    sizef.rheight() /= this->image_scale;
    size = sizef.toSize();

    this->fractal.setImageSize(size);

    this->ui->image_width_spinbox->setValue(size.width());
    this->ui->image_height_spinbox->setValue(size.height());

    this->updateImage();
}

void MainWindow::on_pushButton_clicked(){
    QRectF rect = this->fractal.getRootBoundingBox();
    this->ui->coord_scale_spinbox->setValue(this->fractal.getScaleOf(rect, this->fractal.image.size()));
    complex center_cpx = this->fractal.getCenterOF(rect);
    this->ui->coord_center_real_spinbox->setValue(center_cpx.real());
    this->ui->coord_center_imag_spinbox->setValue(center_cpx.imag());
}

void MainWindow::on_image_scale_spinbox_valueChanged(double arg1){
    QTransform transform;
    transform.scale(arg1, arg1);

    this->ui->graphicsView->setTransform(transform);

    this->image_scale = arg1;
//    this->imageResized(this->ui->graphicsView->size());

    this->ui->image_scale_spinbox->setSingleStep(0.1*arg1);
}


void MainWindow::on_image_width_spinbox_editingFinished(){
    this->imageResized(QSize(this->ui->image_width_spinbox->value(), this->ui->image_height_spinbox->value()));
}


void MainWindow::on_image_height_spinbox_editingFinished(){
    this->imageResized(QSize(this->ui->image_width_spinbox->value(), this->ui->image_height_spinbox->value()));
}


void MainWindow::on_save_button_clicked(){
    QString filter_str = "Images (";
    for (const QByteArray &ba : QImageWriter::supportedImageFormats()){
        filter_str += QString("*.%1 ").arg(ba.constData());
    }
    filter_str.remove(filter_str.length()-1, 1);
    filter_str += ")";

    QString filepath_str =  QFileDialog::getSaveFileName(this, tr("Save File as"),
                                                      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                                      filter_str);
    printf("Save location: %s\n", filepath_str.toUtf8().constData());
    if (!this->fractal.image.save(filepath_str)){
        fprintf(stderr, "Could not save image\n");
    }

}

