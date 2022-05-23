#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>

class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CustomGraphicsView(QWidget *parent = nullptr);

    void resizeEvent(QResizeEvent *event);

signals:
    void resized(QSize size);
};

#endif // CUSTOMGRAPHICSVIEW_H
