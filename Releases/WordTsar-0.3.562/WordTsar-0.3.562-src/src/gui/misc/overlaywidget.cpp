#include "overlaywidget.h"

void cOverlayWidget::newParent()
{
    if (!parent) return;
    parent->installEventFilter(this);
    raise();
}


cOverlayWidget::cOverlayWidget(QWidget * inparent) : QWidget{inparent}
{
    parent = inparent ;
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    newParent();
}

//! Catches resize and child events from the parent widget
bool cOverlayWidget::eventFilter(QObject * obj, QEvent * ev)
{
    if (obj == parent)
    {
        if (ev->type() == QEvent::Resize)
        {
            resize(static_cast<QResizeEvent*>(ev)->size());
        }
        else if (ev->type() == QEvent::ChildAdded)
        {
            raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}

//! Tracks parent widget changes
bool cOverlayWidget::event(QEvent* ev)
{
    if (ev->type() == QEvent::ParentAboutToChange)
    {
        if (parent)
        {
            parent->removeEventFilter(this);
        }
    }
    else if (ev->type() == QEvent::ParentChange)
    {
        newParent();
    }
    return QWidget::event(ev);
}
