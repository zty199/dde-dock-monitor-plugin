#ifndef STREAMCHART_H
#define STREAMCHART_H

#include "type.h"

#include <QWidget>
#include <QQueue>

class StreamChart : public QWidget
{
    Q_OBJECT

public:
    explicit StreamChart(QWidget *parent = nullptr);
    ~StreamChart() override;

protected:
    void paintEvent(QPaintEvent *) override;

public slots:
    // 定时更新数据图标的函数
    void updateChart(struct Data &);

public:
    int height;
    int width;
    int spacing;
    QColor colorBorder, colorBackground, color1, color2;
    int borderRound;

private:
    QPen *penb, *pen1, *pen2;
    QQueue<struct Data> *queue;
};

#endif // STREAMCHART_H
