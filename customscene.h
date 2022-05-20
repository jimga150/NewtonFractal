#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>

class CustomScene : public QGraphicsScene {

    Q_OBJECT

public:

    bool mouse_down = false;

signals:

    void pixelClicked(QPoint p);

    void mouseDraggedTo(QPoint p);

    void mouseReleased();

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent *event){

        const QPointF p = event->scenePos();

        //printf("Scene saw press at %f, %f\n", p.x(), p.y());

        this->mouse_down = true;

        emit this->pixelClicked(p.toPoint());
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event){

        if (!this->mouse_down) return;

        const QPointF p = event->scenePos();

        //printf("Scene saw drag at %f, %f\n", p.x(), p.y());

        emit this->mouseDraggedTo(p.toPoint());
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
        Q_UNUSED(event)

        //const QPointF p = event->scenePos();

        //printf("Scene saw release at %f, %f\n", p.x(), p.y());

        this->mouse_down = false;

        emit this->mouseReleased();
    }

};

#endif // CUSTOMSCENE_H
