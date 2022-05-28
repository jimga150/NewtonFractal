#include "polynomial.h"

Polynomial::Polynomial()
{
    this->changeNumRoots(5);
}

void Polynomial::changeNumRoots(int num_roots){

    this->roots.clear();
    double angle_step_rad = 2*M_PI/num_roots;

    for (int i = 0; i < num_roots; ++i){
        double angle_rad = i*angle_step_rad;
        QPointF pt(cos(angle_rad), sin(angle_rad));
        this->roots.push_back(qpointfToComplex(pt));
    }
}

void Polynomial::changeNumIters(int num_iters)
{
    this->num_iterations = num_iters;
}

uint Polynomial::findRoot(complex x){

//    QElapsedTimer timer;
//    timer.start();

    for (int i = 0; i < this->num_iterations; ++i){
//        complex x1 = x - this->doFunction(x)/this->doFunctionDerivative(x);
        complex x1 = x - this->doFunctionOverDeriv(x);
        if (std::isfinite(x1.real()) && std::isfinite(x1.imag())){
            x = x1;
        } else {
            break;
        }
    }

//    printf("Newtons method (%d iters) took %llu ns; ", this->num_iterations, timer.nsecsElapsed());
//    timer.restart();

    QPointF root_guess_pt = complexToQPointF(x);
    double min_dist = DBL_MAX;
    uint closest_root = this->roots.size() + 1;

    for (uint i = 0; i < this->roots.size(); ++i){
        QPointF root_pt = complexToQPointF(this->roots.at(i));
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

void Polynomial::prepFunctionDerivative(){
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

complex Polynomial::doFunction(complex x){

//    QElapsedTimer timer;
//    timer.start();

    complex ans = (x - this->roots.back());
    for (int i = this->roots.size() - 2; i >= 0; --i){
        ans *= (x - this->roots.at(i));
    }

//    printf("\tdoFunction took %llu ns\n", timer.nsecsElapsed());

    return ans;
}

complex Polynomial::doFunctionDerivative(complex x){

//    QElapsedTimer timer;
//    timer.start();

    complex ans = this->derivative_coefs.back();
    for (int power = this->derivative_coefs.size()-2; power >= 0; --power){
        ans = ans*x + this->derivative_coefs.at(power);
    }

//    printf("\tdoFunctionDerivative took %llu ns\n", timer.nsecsElapsed());

    return ans;
}

complex Polynomial::doFunctionOverDeriv(complex x){

//    QElapsedTimer timer;
//    timer.start();

    //turns out that for any polynomial f(x) with n complex roots r_0 ... r_n:
    //f'(x)/f(x) = sum(i = 0 ... n, 1/(x-r_i))

    complex ans = this->invertComplex(x - this->roots.back());
    for (int i = this->roots.size() - 2; i >= 0; --i){
        ans += this->invertComplex(x - this->roots.at(i));
    }

//    printf("\tdoFunction/derivative took %llu ns\n", timer.nsecsElapsed());

    return this->invertComplex(ans);
}

complex Polynomial::invertComplex(complex c){
    double denominator = std::norm(c);
    return complex(c.real()/denominator, -c.imag()/denominator);
}

std::vector<std::vector<complex>> Polynomial::getSets(std::vector<complex> list, uint set_size){

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
uint Polynomial::nCr(uint n, uint r){
    return this->factorial(n) / (this->factorial(r) * this->factorial(n - r));
}

uint Polynomial::factorial(uint n){
    uint ans = 1;
    while(n > 1){
        ans *= n--;
    }
    return ans;
}

void Polynomial::printList(std::vector<complex> list){
    printf("(");
    for (uint c_idx = 0; c_idx < list.size(); ++c_idx){
        printf("%f", list.at(c_idx).real());
        if (c_idx < list.size()-1){
            printf(", ");
        }
    }
    printf(")\n");
}

void Polynomial::print2DList(std::vector<std::vector<complex>> list2d){
    for (const std::vector<complex> &set : list2d){
        printf("\t");
        this->printList(set);
    }
}

