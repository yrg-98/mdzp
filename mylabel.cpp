#include "mylabel.h"

myLabel::myLabel(QWidget *parent) : QLabel(parent)
{
    corners[0][0] = Point( 191,78 );
    corners[0][1] = Point( 0,480 );
    corners[0][2] = Point( 640,480 );
    corners[0][3] = Point( 439,78 );
}
void myLabel::mousePressEvent(QMouseEvent *ev)
{
   if(flag_choose)
   {
        if( ev->button() ==  Qt::LeftButton)
             {             
               sum++;
               corners[0][sum-1]=Point(ev->x(),ev->y());
             }
   if(sum==4)
       {
       flag_choose=false;
       sum=0;
       setCursor(Qt::ArrowCursor);
       QMessageBox::information(this,tr("提示"),tr("选择完成"));

       }
   }
}
