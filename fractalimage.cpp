#include "fractalimage.h"

FractalImage::FractalImage(QObject *parent)
    : QObject{parent}
{

    int num_roots = 5;
    double angle_step_rad = 2*M_PI/num_roots;
    for (int i = 0; i < num_roots; ++i){
        double angle_rad = i*angle_step_rad;
        QPointF pt(cos(angle_rad), sin(angle_rad));
        this->roots.push_back(this->qpointfToComplex(pt));
    }

    this->image = QImage(500, 500, QImage::Format_ARGB32);
    this->image.fill(Qt::GlobalColor::black);

    this->coord_to_ui_tform.translate(this->image.width()/2, this->image.height()/2);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);
    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

    this->colors.push_back(QColor(207, 73, 20));
    this->colors.push_back(QColor(249, 172, 59));
    this->colors.push_back(QColor(116, 139, 50));
    this->colors.push_back(QColor(152, 89, 20));
    this->colors.push_back(QColor(207, 178, 132));
    this->colors.push_back(QColor(44, 116, 139));

    this->updateImage();
}

void FractalImage::updateImage(){

//    QElapsedTimer timer;
//    timer.start();

    this->prepFunctionDerivative();

//    printf("prep function derivative took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();

    this->image.fill(Qt::GlobalColor::black);

    QList<QFuture<void>> threads;

    for (int y = 0; y < this->image.height(); y = ++y){

        QRgb* line = reinterpret_cast<QRgb*>(this->image.scanLine(y));

        QFuture<void> thread = QtConcurrent::run(&FractalImage::updateImageLine, this, y, line);
        threads.append(thread);
    }

    for (QFuture<void> thread : threads){
        thread.waitForFinished();
    }

//    QList<line_item_struct> lines_to_process;

//    for (int y = 0; y < this->image.height(); y = ++y){
//        QRgb* line = reinterpret_cast<QRgb*>(this->image.scanLine(y));
//        lines_to_process.append(line_item_struct(y, line));
//    }

//    QtConcurrent::blockingMap(lines_to_process, &FractalImage::updateImageLine_wstruct);

//    printf("fractal fill took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();
}

void FractalImage::updateImageLine_wstruct(line_item_struct line_item){
    this->updateImageLine(line_item.row, line_item.line_ptr);
}

void FractalImage::updateImageLine(int y, QRgb* lines){

    for (int x = 0; x < this->image.width(); ++x){

        QPointF pt(x, y);
        pt = this->ui_to_coord_tform.map(pt);
        complex start = this->qpointfToComplex(pt);

//            printf("\tpoint prep took %lld ns\n", timer.nsecsElapsed());
//            timer.restart();

        uint closest_root = this->findRoot(start);
//            printf("closest root: %u\n", closest_root);

//            printf("\troot finding took %lld ns\n", timer.nsecsElapsed());
//            timer.restart();

//            this->current_img.setPixel(x, y, this->colors.at(closest_root).rgb());
        lines[x] = this->colors.at(closest_root).rgb();

//            printf("\tpixel setting took %lld ns\n", timer.nsecsElapsed());
//            timer.restart();
    }
}

uint FractalImage::findRoot(complex x){

//    QElapsedTimer timer;
//    timer.start();

    for (int i = 0; i < this->num_iterations; ++i){
        complex x1 = x - this->doFunction(x)/this->doFunctionDerivative(x);
        if (isfinite(x1.real()) && isfinite(x1.imag())){
            x = x1;
        } else {
            break;
        }
    }

//    printf("Newtons method (%d iters) took %llu ns; ", this->num_iterations, timer.nsecsElapsed());
//    timer.restart();

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

//    printf("Closest root finding took %llu ns\n", timer.nsecsElapsed());

    return closest_root;
}

complex FractalImage::qpointfToComplex(QPointF p){
    //x is the real axis, y is the imaginary axis
    return complex(p.x(), p.y());
}

QPointF FractalImage::complexToQPointF(complex c){
    //x is the real axis, y is the imaginary axis
    return QPointF(c.real(), c.imag());
}

void FractalImage::prepFunctionDerivative(){
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

complex FractalImage::doFunction(complex x){

//    QElapsedTimer timer;
//    timer.start();

    complex ans = (x - this->roots.back());
    for (int i = this->roots.size() - 2; i >= 0; --i){
        ans *= (x - this->roots.at(i));
    }

//    printf("\tdoFunction took %llu ns\n", timer.nsecsElapsed());

    return ans;
}

complex FractalImage::doFunctionDerivative(complex x){

//    QElapsedTimer timer;
//    timer.start();

    complex ans = this->derivative_coefs.back();
    for (int power = this->derivative_coefs.size()-2; power >= 0; --power){
        ans = ans*x + this->derivative_coefs.at(power);
    }

//    printf("\tdoFunctionDerivative took %llu ns\n", timer.nsecsElapsed());

    return ans;
}

std::vector<std::vector<complex>> FractalImage::getSets(std::vector<complex> list, uint set_size){

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
uint FractalImage::nCr(uint n, uint r){
    return this->factorial(n) / (this->factorial(r) * this->factorial(n - r));
}

uint FractalImage::factorial(uint n){
    uint ans = 1;
    while(n > 1){
        ans *= n--;
    }
    return ans;
}

void FractalImage::printList(std::vector<complex> list){
    printf("(");
    for (uint c_idx = 0; c_idx < list.size(); ++c_idx){
        printf("%f", list.at(c_idx).real());
        if (c_idx < list.size()-1){
            printf(", ");
        }
    }
    printf(")\n");
}

void FractalImage::print2DList(std::vector<std::vector<complex>> list2d){
    for (const std::vector<complex> &set : list2d){
        printf("\t");
        this->printList(set);
    }
}


