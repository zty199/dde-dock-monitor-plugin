#include "customcombobox.h"

#include <QWheelEvent>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void CustomComboBox::showPopup()
{
    return;
}

void CustomComboBox::wheelEvent(QWheelEvent *event)
{
    QComboBox::wheelEvent(event);

    int delta = event->angleDelta().y();

    if (delta > 0) {
        int prev = currentIndex() - 1;
        if (prev >= 0)
            setCurrentIndex(prev);
    } else if (delta < 0) {
        int next = currentIndex() + 1;
        if (next < count())
            setCurrentIndex(next);
    }

    event->accept(); // prevent propagation
}
