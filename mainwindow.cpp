#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    int num_roots = 5;
    double angle_step_rad = 2*M_PI/num_roots;
    for (int i = 0; i < num_roots; ++i){
        double angle_rad = i*angle_step_rad;
        double x = cos(angle_rad);
        double y = sin(angle_rad);
        this->roots.push_back(complex(x, y));
    }

    ui->setupUi(this);

    this->current_img = QImage(500, 500, QImage::Format_ARGB32);
    this->current_img.fill(Qt::GlobalColor::black);

    this->pixmap_item = this->scene.addPixmap(QPixmap::fromImage(this->current_img));

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    this->coord_to_ui_tform.translate(250, 250);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);
    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

    this->updateImage();

    connect(&this->scene, &CustomScene::pixelClicked, this, &MainWindow::clicked);
    connect(&this->scene, &CustomScene::mouseDraggedTo, this, &MainWindow::dragged);
    connect(&this->scene, &CustomScene::mouseReleased, this, &MainWindow::released);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateImage(){
    this->current_img.fill(Qt::GlobalColor::black);
    QPainter painter(&this->current_img);

    QPen pen(QBrush(Qt::GlobalColor::white), this->ui_to_coord_scale);
    pen.setColor(Qt::GlobalColor::white);
    painter.setPen(pen);

    painter.setTransform(this->coord_to_ui_tform);

    for (complex r : this->roots){
        QPointF center = complexToQPointF(r);
        painter.drawEllipse(center, 0.1, 0.1);
//        printf("Drawing circle at (%f, %f)\n", center.x(), center.y());
    }

    painter.end();

    this->pixmap_item->setPixmap(QPixmap::fromImage(this->current_img));
}

void MainWindow::clicked(QPoint p){
//    printf("click gotten at pixel (%d, %d)\n", p.x(), p.y());
//    fflush(stdout);

    QPointF pf = p;
    QPointF coord = this->ui_to_coord_tform.map(pf);

    root_is_selected = false;

    for (uint i = 0; i < this->roots.size(); ++i){
        complex r = this->roots.at(i);
        QPointF root_pt = complexToQPointF(r);
        double x_diff = root_pt.x() - coord.x();
        double y_diff = root_pt.y() - coord.y();
        double dist = sqrt(x_diff*x_diff + y_diff*y_diff);
        if (dist < 0.1){
            this->current_root_selected = i;
            root_is_selected = true;
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
    QPointF coord = this->ui_to_coord_tform.map(pf);

    this->roots.at(this->current_root_selected) = qpointfToComplex(coord);

    this->updateImage();
}

void MainWindow::released(){
//    printf("mouse released\n");
//    fflush(stdout);
}

complex MainWindow::qpointfToComplex(QPointF p){
    //x is the real axis, y is the imaginary axis
    return complex(p.x(), p.y());
}

QPointF MainWindow::complexToQPointF(complex c){
    //x is the real axis, y is the imaginary axis
    return QPointF(c.real(), c.imag());
}
