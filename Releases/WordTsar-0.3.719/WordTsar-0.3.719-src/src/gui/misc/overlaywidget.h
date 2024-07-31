#ifndef COVERLAYWIDGET_H
#define COVERLAYWIDGET_H

#include <QtGui>
#include <QWidget>

class cOverlayWidget : public QWidget
{
   void newParent() ;

public:
   cOverlayWidget(QWidget * inparent = {}) ;

protected:
   //! Catches resize and child events from the parent widget
   bool eventFilter(QObject * obj, QEvent * ev) override ;

   //! Tracks parent widget changes
   bool event(QEvent* ev) override ;

   QWidget *parent ;
};

#endif // COVERLAYWIDGET_H
