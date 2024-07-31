#include <QColorDialog>

#include "qcolorpickerbutton.h"


QColorPickerButton::QColorPickerButton(QWidget *parent) : QPushButton(parent)
{
    setFlat(true) ;
}


void QColorPickerButton::setColor(QColor color)
{
    setFlat(true) ;

    QPalette pal = palette();
    pal.setColor(QPalette::Button, color);
    setAutoFillBackground(true);
    setPalette(pal);
    update();

    mColor = color ;
}

QColor QColorPickerButton::color(void)
{
    return mColor ;
}

void QColorPickerButton::mousePressEvent(QMouseEvent *event)
{
    QColorDialog color(this) ;

    int res = color.exec() ;
    if(res != 0)
    {
        mColor = color.currentColor() ;

        setColor(mColor) ;
    }
}
