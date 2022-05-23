#include "customgraphicsview.h"

CustomGraphicsView::CustomGraphicsView(QWidget *parent)
    : QGraphicsView{parent}
{

}

void CustomGraphicsView::resizeEvent(QResizeEvent *event){
    emit this->resized(event->size());
}
