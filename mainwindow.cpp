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
        QPointF pt(cos(angle_rad), sin(angle_rad));
        this->roots.push_back(this->qpointfToComplex(pt));
    }

    ui->setupUi(this);

    this->current_img = QImage(500, 500, QImage::Format_ARGB32);
    this->current_img.fill(Qt::GlobalColor::black);

    this->pixmap_item = this->scene.addPixmap(QPixmap::fromImage(this->current_img));

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    this->coord_to_ui_tform.translate(this->current_img.width()/2, this->current_img.height()/2);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);
    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

//    std::vector<complex> test_list;
//    test_list.push_back(1);
//    test_list.push_back(2);
//    test_list.push_back(3);
//    test_list.push_back(4);
//    test_list.push_back(5);
//    test_list.push_back(6);
//    test_list.push_back(7);

//    uint set_size = 5;

//    std::vector<std::vector<complex>> test_sets = this->getSets(test_list, set_size);
//    printf("Got:\n");
//    this->print2DList(test_sets);

//    Q_ASSERT(this->nCr(test_list.size(), set_size) == test_sets.size());
//    for (const std::vector<complex> &l : test_sets){
//        Q_ASSERT(l.size() == set_size);
//    }

    this->colors.push_back(QColor(207, 73, 20));
    this->colors.push_back(QColor(249, 172, 59));
    this->colors.push_back(QColor(116, 139, 50));
    this->colors.push_back(QColor(152, 89, 20));
    this->colors.push_back(QColor(207, 178, 132));
    this->colors.push_back(QColor(44, 116, 139));

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

    this->prepFunctionDerivative();

    this->current_img.fill(Qt::GlobalColor::black);

    for (int y = 0; y < this->current_img.height(); y = y + 4){
        for (int x = 0; x < this->current_img.width(); x = x + 4){
            QPointF pt(x, y);
            pt = this->ui_to_coord_tform.map(pt);
            complex start = this->qpointfToComplex(pt);
            uint closest_root = this->findRoot(start);
//            printf("closest root: %u\n", closest_root);
            this->current_img.setPixel(x, y, this->colors.at(closest_root).rgb());
        }
    }

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

uint MainWindow::findRoot(complex x){
    for (int i = 0; i < this->num_iterations; ++i){
        x = x - this->doFunction(x)/this->doFunctionDerivative(x);
    }

    QPointF root_guess_pt = this->complexToQPointF(x);
    double min_dist = DBL_MAX;
    uint closest_root = this->roots.size() + 1;

    for (uint i = 0; i < this->roots.size(); ++i){
        QPointF root_pt = this->complexToQPointF(this->roots.at(i));
        double x_diff = root_guess_pt.x() - root_pt.x();
        double y_diff = root_guess_pt.y() - root_pt.y();
        double dist = sqrt(x_diff*x_diff + y_diff*y_diff);
        if (dist < min_dist){
            min_dist = dist;
            closest_root = i;
        }
    }

    return closest_root;
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

void MainWindow::prepFunctionDerivative(){
    uint order = this->roots.size();
    complex reg_coefs[order+1];
    this->derivative_coefs.clear();

    for (uint i = 0; i < order; ++i){
        complex ans = 0;

        std::vector<std::vector<complex>> root_sets = this->getSets(this->roots, order - i);
        for (const std::vector<complex> &root_set : root_sets){
            complex to_add = 1;
            for (complex root : root_set){
                to_add *= -1.0*root;
            }
            ans += to_add;
        }

        reg_coefs[i] = ans;

        if (i > 0){
            this->derivative_coefs.push_back(ans*static_cast<double>(i));
        }
    }

    reg_coefs[order] = 1.0;
    this->derivative_coefs.push_back(order);

//    for (uint i = 0; i < order; ++i){
//        printf("derivative coefficient %u is %f + %fi\n", i, this->derivative_coefs[i].real(), this->derivative_coefs[i].imag());
//    }

//    for (uint i = 0; i <= order; ++i){
//        printf("regular coefficient %u is %f + %fi\n", i, reg_coefs[i].real(), reg_coefs[i].imag());
//    }
}

complex MainWindow::doFunction(complex x){
    complex ans = 1;
    for (complex root : this->roots){
        ans *= (x - root);
    }
    return ans;
}

complex MainWindow::doFunctionDerivative(complex x){
    complex ans = 0;
    for (uint power = 0; power < this->derivative_coefs.size(); ++power){
        complex term = this->derivative_coefs.at(power);
        for (uint i = 0; i < power; ++i){
            term *= x;
        }
        ans += term;
    }
    return ans;
}

std::vector<std::vector<complex>> MainWindow::getSets(std::vector<complex> list, uint set_size){

    std::vector<std::vector<complex>> ans;

//    printf("getSets called with list of size %lu to find sets of size %u\n", list.size(), set_size);
//    printf("List: ");
//    this->printList(list);

    if (list.size() < set_size){
        fprintf(stderr, "Set size greater than list size!\n");
        return ans;
    }

    if (set_size == 0){
        fprintf(stderr, "Set size is 0!\n");
        return ans;
    }

    if (list.size() == set_size){
        ans.push_back(list);
//        printf("Base case hit, returning items as only set\n");
        return ans;
    }

    if (set_size == 1){
        for (uint i = 0; i < list.size(); ++i){
            std::vector<complex> set;
            set.push_back(list.at(i));
            ans.push_back(set);
        }
//        printf("Base case hit, returning items as a column\n");
        return ans;
    }

    while(list.size() >= set_size){

//        printf("List: ");
//        this->printList(list);

        //pick a common number
        complex common_item = list.at(0);

        //remove it from the list, since this iteration will be, or complete, all sets with this number in them
        list.erase(list.begin());

        //recurse; get all sets of s - 1 size from the remaining items
        std::vector<std::vector<complex>> sublist_sets = this->getSets(list, set_size-1);

        //add common item to all sets and add sets to final list
        for (std::vector<complex> set : sublist_sets){
            set.push_back(common_item);
            ans.push_back(set);
        }
    }

//    printf("Returning:\n");
//    this->print2DList(ans);

    return ans;
}

//how many sets of size r can be chosen from a set of size n
uint MainWindow::nCr(uint n, uint r){
    return this->factorial(n) / (this->factorial(r) * this->factorial(n - r));
}

uint MainWindow::factorial(uint n){
    uint ans = 1;
    while(n > 1){
        ans *= n--;
    }
    return ans;
}

void MainWindow::printList(std::vector<complex> list){
    printf("(");
    for (uint c_idx = 0; c_idx < list.size(); ++c_idx){
        printf("%f", list.at(c_idx).real());
        if (c_idx < list.size()-1){
            printf(", ");
        }
    }
    printf(")\n");
}

void MainWindow::print2DList(std::vector<std::vector<complex>> list2d){
    for (const std::vector<complex> &set : list2d){
        printf("\t");
        this->printList(set);
    }
}
