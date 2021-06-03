#include "mdzp.h"
#include "ui_mdzp.h"

Mdzp::Mdzp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Mdzp)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/mdzp.png"));
    this->setFixedSize(this->size());
    this->setWindowTitle("鸣笛抓拍检定系统");
    QToolBar *myToolBar=new QToolBar(this);
    addToolBar(Qt::TopToolBarArea,myToolBar);
    myToolBar->setAllowedAreas( Qt::TopToolBarArea);
    myToolBar->setMovable(false);
    myToolBar->setFloatable(false);
    //图标1 裁剪
    QPushButton * btn = new QPushButton("" , this);
    myToolBar->addWidget(btn);
    btn->setIcon(QIcon(":/jt.png"));
    connect(btn,&QPushButton::clicked,this,&Mdzp::choose_area);
    btn->setToolTip("按逆时针选择进行图像处理的区域");
    //图标2 找圆心
    QPushButton *find2circle=new QPushButton("",this);
    myToolBar->addWidget(find2circle);
    find2circle->setIcon(QIcon(":/find_2circle.png"));
    connect(find2circle,&QPushButton::clicked,this,&Mdzp::find2circle);
    find2circle->setToolTip("求两圆心距离");

}
Mdzp::~Mdzp()
{
    delete ui;
}
Mat Mdzp::QImage2cvMat(QImage image)
{
    Mat mat;
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}
double Mdzp::square(int x1,int y1,int x2,int y2,int x3,int y3)
 {
     double square=abs((x1*y2-x1*y3+x2*y3-x2*y1+x3*y1-x3*y2)/2);
     return square;
 }
QImage Mdzp::cvMat2QImage(const Mat &mat ) {
  switch ( mat.type() )
  {
     case CV_8UC4:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32 );
        return image;
     }
     // 8-bit, 3 channel
     case CV_8UC3:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888 );
        return image.rgbSwapped();
     }
     case CV_8UC1:
     {
        static QVector<QRgb>  sColorTable;
        // only create our color table once
        if ( sColorTable.isEmpty() )
        {
           for ( int i = 0; i < 256; ++i )
              sColorTable.push_back( qRgb( i, i, i ) );
        }
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
        return image;
     }
     default:
        qDebug("Image format is not supported: depth=%d and %d channels\n", mat.depth(), mat.channels());
        break;
  }
  return QImage();
}
void Mdzp::ShowMat2Label(const Mat &mat)

 {
     QImage img=cvMat2QImage(mat);
     QImage* img1=&img;
     *img1 = img1->scaled(ui->label->size(), Qt::IgnoreAspectRatio);
     ui->label->setPixmap(QPixmap::fromImage(*img1));
 }
Mat Mdzp::cut_picture(Mat &mat)
{
    Mat dst;
    corners[0][0]=ui->label->corners[0][0];
    corners[0][1]=ui->label->corners[0][1];
    corners[0][2]=ui->label->corners[0][2];
    corners[0][3]=ui->label->corners[0][3];
    const Point* ppt[1]={corners[0]};
    int npt[] = {4};    
    cv::Mat mask(480,640,CV_8UC1, cv::Scalar(0,0,0));
    fillPoly(mask, ppt, npt,1, Scalar(255,255,255));
    cv::bitwise_and(mat, mask, dst);
    return dst;
}
void Mdzp::on_actionsrc_triggered()
{
    ShowMat2Label(src);
}

void Mdzp::on_actionn1_triggered()
{
    path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.jpg *.png *.bmp)"));
    QImage* img = new QImage;
    if(!(img->load(path))) //加载图像
    {
       QMessageBox::information(this, tr("打开图像失败"), tr("打开图像失败!"));
       delete img;
       return;
    }
    src=QImage2cvMat(*img);
    cvtColor(src,gray,COLOR_BGR2GRAY);
    threshold(gray,ezh,190,255,THRESH_BINARY_INV);
    medianBlur(src,zzlb,(9,9));
    GaussianBlur(src,gslb,Size(0,0),3,3);
    ShowMat2Label(src);
}
void Mdzp::on_actionimport_video_triggered()
{
    path = QFileDialog::getOpenFileName(this, tr("选择视频"), ".", tr("Video Files(*.mp4)"));
    if(path=="")
    {
       QMessageBox::information(this, tr("打开视频失败"), tr("打开视频失败!"));
       return;
    }
    player = new QMediaPlayer;
    videoWidget = new QVideoWidget;
    videoWidget->resize(640, 480);
    player->setVideoOutput(videoWidget);
    player->setMedia(QUrl::fromLocalFile(path));
    videoWidget->show();
    player->play();
   // this->setCentralWidget(videoWidget);
    // ui ->label->show(videoWidget);
   QPixmap mypxmap=QPixmap::grabWidget(videoWidget);
   // player->pause();
    ui->label->setPixmap(mypxmap);
}
void Mdzp::on_actionn2_triggered()
{
    ShowMat2Label(zzlb);
}
void Mdzp::on_actionn3_triggered()
{
    ShowMat2Label(gslb);
}
void Mdzp::on_actionn4_triggered()
{
    Mat cannysz;
    GaussianBlur(gray,cannysz,Size(3,3),0);
    Canny(cannysz,cannysz,50,150);
    ShowMat2Label(cannysz);
}

void Mdzp::on_actionn6_triggered()
{
    Mat dst;
    Mat bkgd(480,640,CV_8UC1,Scalar(0,0,0));
 GaussianBlur(gray,dst,Size(3,3),0);
 Canny(dst,dst,50,150);
    dst=(cut_picture(dst)).clone();
 ShowMat2Label(cut_picture(dst));
    vector<Vec4i>lines;
    HoughLinesP(dst,lines,1,CV_PI/180,70,50,40);
    for(size_t i=0;i<lines.size();i++)
    {
        Vec4i I=lines[i];
        line(bkgd,Point(I[0],I[1]),Point(I[2],I[3]),Scalar(255,255,255),1);
    }
    rect_dst=bkgd;
    int sum1=2000;
    int sum2=2000;
    int sum3=0;
    int sum4=0;
    for(int i=0;i<480;i++)
        for(int j=0;j<640;j++)
       if (bkgd.at<uchar>(i,j)==255)
       {
            if ((i+j)<sum1){
                sum1=i+j;
                x1=j;
                y1=i;}
            if ((i+640-j)<sum2){
            sum2=i+640-j;
            x2=j;
            y2=i;}
            if ((i+640-j)>sum3){
            sum3=i+640-j;
            x3=j;
            y3=i;}
            if((i+j)>sum4){
            sum4=i+j;
            x4=j;
            y4=i;}
       }
 Mat hsv,circle ;
 cvtColor(src, hsv,COLOR_BGR2HSV);
 inRange(hsv, Scalar(0,43,46), Scalar(10,255,255),circle);
 bitwise_and(circle, cut_picture(gray), dst);
 threshold(dst,dst,1,255,THRESH_BINARY);
 flag_circle=false;
 circle_dst=dst;
 //获取图像轮廓
     vector<vector<Point>>contours;	//每个轮廓中的点
     vector<Vec4i>hierarchy;			//轮廓的索引
     findContours(dst, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());
     qDebug()<<contours.size();
     for (int i = 0; i < contours.size(); ++i)
     {
         Mat tmp(contours.at(i));
         Moments moment=moments(tmp, false);
         if (moment.m00 != 0)//除数不能为0
         {
              cx = cvRound(moment.m10 / moment.m00);//计算重心横坐标
              cy = cvRound(moment.m01 / moment.m00);//计算重心纵坐标
              flag_circle=true;
         }
     }
 bitwise_or(rect_dst,circle_dst,mymat);
 ShowMat2Label(mymat);
  double s1=square(x1,y1,x2,y2,cx,cy);
  double s2=square(x2,y2,x4,y4,cx,cy);
  double s3=square(x3,y3,x4,y4,cx,cy);
  double s4=square(x1,y1,x3,y3,cx,cy);
  double sum=square(x1,y1,x2,y2,x3,y3)+square(x2,y2,x3,y3,x4,y4);
   H=mycamera->H2;
   Dmin=mycamera->Dmin2;
   Dmax=mycamera->Dmax2;
   width=mycamera->width2;
   height=mycamera->height2;
   Beta=mycamera->Beta2;
   int X0 = cx;
   int Y0 = cy;
   double Alpha = (atan2(Dmin,H))*180/(M_PI);
   double Theta = (atan2(Dmax,H))*180/(M_PI)-Alpha;
   double Delte = (height-Y0)/height*Theta;
   Y1 = H*tan((Alpha+Delte)*(M_PI)/180);
   double B1 = Y1*tan((Beta/2)*(M_PI)/180);
   X1 = 2*B1*(X0-(width/2))/width;
   int X11=X1*100;
   X1=X11/100.00;
   int Y11=Y1*100;
   Y1=Y11/100.00;
   QString result;
   result+= tr(" 左上角坐标为：(%1,%2)\n\r"
                 "右上角坐标为：(%3,%4)\n\r"
                 "左下角坐标为：(%5,%6)\n\r"
                 "右下角坐标为：(%7,%8)\n\r")
              .arg(x1).arg(y1).arg(x2).arg(y2).arg(x3).arg(y3).arg(x4).arg(y4);
   if (flag_circle)
         {
           result+=tr("圆心坐标为(%1,%2)\n\r").arg(cx).arg(cy);
            if (s1+s2+s3+s4 <= sum)
                   {
                      if_in=("圆心在目标框内\n\r");
                      result+=tr("%1").arg(if_in);
                   }
            else
                   {
                      if_in=("圆心不在目标框内\n\r");
                      result+=tr("%1").arg(if_in);
                   }
         }
   else result+=tr("未检测到圆心\n\r");
   result+=tr("距离摄像头的水平距离为: %1m\r\n 距离摄像头的垂直距离为: %2m\n\r").arg(X1).arg(Y1);
  // ShowMat2Label(mymat);
   QMessageBox::information(this,tr("检测结果"),tr("%1").arg(result));

   //发送至列表
   QDateTime local(QDateTime::currentDateTime());
   time= local.toString("yyyy-MM-dd:hh:mm:ss");
   mylist[nlist]={path,mymat,x1,y1,x2,y2,x3,y3,x4,y4,cx,cy,if_in,X1,Y1,time};
   nlist++;
}
void Mdzp::on_actionn9_triggered()
{
    ShowMat2Label(gray);
}
void Mdzp::on_actionn8_triggered()
{
    ShowMat2Label(ezh);
}
void Mdzp::on_actionn10_triggered()
{
       QString filename1 = QFileDialog::getSaveFileName(this,tr("Save Image"),"",tr("Images (*.png *.bmp *.jpg)")); //选择路径
       QScreen *screen = QGuiApplication::primaryScreen();
       screen->grabWindow(ui->label->winId()).save(filename1);
}
void Mdzp::on_actionhelp_triggered()
{
    QMessageBox::information(this,tr("帮助"),tr("技术支持：姚润广"));
}
void Mdzp::on_actioncamera1_triggered()
{
    mycamera->show();
}

void Mdzp::on_actionresult_list_triggered()
{
     result_list *myresult_list=new result_list();
     myresult_list->nlist=nlist;
     for(int i=0;i<nlist;i++)
         myresult_list->mylist[i]=mylist[i];
     myresult_list->InitComboBox();
     myresult_list->show();
}


void Mdzp::choose_area()
{
    ui->label->setCursor(Qt::CrossCursor);
    ui->label->flag_choose=true;
}
void Mdzp::find2circle()
{
    Mat hsv2,circle2 ;
    int cx2[2],cy2[2],tempcx,tempcy;
    int num_of_circle=0;
    cvtColor(src, hsv2,COLOR_BGR2HSV);

    inRange(hsv2, Scalar(0,43,46), Scalar(10,255,255),circle2);
    bitwise_and(circle2, cut_picture(src), circle2);
    threshold(circle2,circle2,1,255,THRESH_BINARY);

    //获取图像轮廓
        vector<vector<Point>>contours;	//每个轮廓中的点
        vector<Vec4i>hierarchy;			//轮廓的索引
        findContours(circle2, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());
        qDebug()<<contours.size();
        for (int i = 0; i < contours.size(); ++i)
        {
            Mat tmp(contours.at(i));
            Moments moment=moments(tmp, false);
            if (moment.m00 != 0)//除数不能为0
            {
               tempcx = cvRound(moment.m10 / moment.m00);//计算重心横坐标
               tempcy = cvRound(moment.m01 / moment.m00);//计算重心纵坐标
               if (num_of_circle==0)
               {
                   cx2[num_of_circle]=tempcx;
                   cy2[num_of_circle]=tempcy;
                   num_of_circle++;
               }
               if(num_of_circle==1)
               {
                   if((tempcx-cx2[0])*(tempcx-cx2[0])+(tempcy-cy2[0])*(tempcy-cy2[0])>25)
                   {
                       cx2[num_of_circle]=tempcx;
                       cy2[num_of_circle]=tempcy;
                       num_of_circle++;
                   }
               }
            }
            if(num_of_circle==2)break;
        }
        ShowMat2Label(circle2);
        if(num_of_circle==2)
        {
            int dist_2circle=(cx2[0]-cx2[1])*(cx2[0]-cx2[1])+(cy2[0]-cy2[1])*(cy2[0]-cy2[1]);
            QMessageBox::information(this,tr("提示"),tr("两云彩图圆心距离为：%1").arg(dist_2circle));
        }
        else
        {
            QMessageBox::information(this,tr("提示"),tr("未识别到两圆心"));
        }

}

