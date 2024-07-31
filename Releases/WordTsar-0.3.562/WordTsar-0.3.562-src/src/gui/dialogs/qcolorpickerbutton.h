#include <QPushButton>
#include <QColor>

class  QColorPickerButton : public QPushButton
{
    Q_OBJECT

public :
    explicit QColorPickerButton(QWidget *parent = nullptr) ;

    void setColor(QColor color) ;
    QColor color(void) ;

signals :

public slots :

protected :
    void mousePressEvent(QMouseEvent *event) override ;

private :
    QColor mColor ;
};
