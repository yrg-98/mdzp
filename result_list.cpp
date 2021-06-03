#include "result_list.h"
#include "ui_result_list.h"

result_list::result_list(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::result_list)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setWindowTitle("处理结果列表");
    this->setWindowIcon(QIcon(":/jg.png"));



}

result_list::~result_list()
{
    delete ui;
}

Mat result_list::QImage2cvMat(QImage image)
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

QImage result_list::cvMat2QImage(const Mat &mat ) {
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
void result_list::ShowMat2Label(const Mat &mat)

 {
     QImage img=cvMat2QImage(mat);
     QImage* img1=&img;
     *img1 = img1->scaled(ui->label->size(), Qt::IgnoreAspectRatio);
     ui->label->setPixmap(QPixmap::fromImage(*img1));
 }
void result_list::on_pushButton_clicked()
{
    int index=ui->comboBox->currentIndex();
    ShowMat2Label(mylist[index].mat);
    ui->textEdit->setText(tr("图片路径：%1\n\r"
                              "左上角坐标为：(%2,%3)\n\r"
                              "右上角坐标为：(%4,%5)\n\r"
                              "左下角坐标为：(%6,%7)\n\r"
                              "右下角坐标为：(%8,%9)\n\r"
                              "圆心坐标为：(%10,%11)\n\r"
                              "距离摄像头的水平距离为：%12 米\n\r"
                              "距离摄像头的垂直距离为：%13 米\n\r"
                              "%14\n\r"
                              "处理时间：%15\n\r")
                             .arg(mylist[index].path)
                             .arg(mylist[index].rect[0][0])
                             .arg(mylist[index].rect[0][1])
                             .arg(mylist[index].rect[1][0])
                             .arg(mylist[index].rect[1][1])
                             .arg(mylist[index].rect[2][0])
                             .arg(mylist[index].rect[2][1])
                             .arg(mylist[index].rect[3][0])
                             .arg(mylist[index].rect[3][1])
                             .arg(mylist[index].circle[0])
                             .arg(mylist[index].circle[1])
                             .arg(mylist[index].realcircle[0])
                             .arg(mylist[index].realcircle[1])
                             .arg(mylist[index].if_in)
                             .arg(mylist[index].time)
            );
}

void result_list::InitComboBox()
{
    ui->comboBox->clear();
    for(int i=0;i<nlist;i++)
       ui->comboBox->addItem(tr("%1").arg(mylist[i].path));
}

void result_list::on_actionsave_triggered()
{
    QString excel_path=QFileDialog::getOpenFileName(this,"选择导出表格",".","*.xls *.xlsx");
    if(excel_path==NULL)
    {
        QMessageBox::information(this,tr("提示"),tr("打开表格失败！"));
        return;
    }
    newExcel(excel_path);
    appendSheet("鸣笛抓拍系统检测");
    setCellValue(1,1,"图片路径");
    setCellValue(1,2,"像素横坐标x");
    setCellValue(1,3,"像素纵坐标y");
    setCellValue(1,4,"距离摄像头的水平距离");
    setCellValue(1,5,"距离摄像头的垂直距离");
    setCellValue(1,6,"圆心是否在框内");
    setCellValue(1,7,"处理时间");
    for(int row=1;row<=nlist;row++)
    {
            setCellValue(row+1,1,mylist[row-1].path);
            setCellValue(row+1,2,QString::number(mylist[row-1].circle[0]));
            setCellValue(row+1,3,QString::number(mylist[row-1].circle[1]));
            setCellValue(row+1,4,QString::number(mylist[row-1].realcircle[0]));
            setCellValue(row+1,5,QString::number(mylist[row-1].realcircle[1]));
            setCellValue(row+1,6,mylist[row-1].if_in);
            setCellValue(row+1,7,mylist[row-1].time);
    }
    saveExcel(excel_path);
    freeExcel();
    QMessageBox::information(this,tr("提示"),tr("导出成功！"));
}
void result_list::newExcel(const QString &fileName)
{
    pApplication = new QAxObject();
    pApplication->setControl("Excel.Application");//连接Excel控件
    pApplication->dynamicCall("SetVisible(bool)", false);//false不显示窗体
    pApplication->setProperty("DisplayAlerts", false);//不显示任何警告信息。
    pWorkBooks = pApplication->querySubObject("Workbooks");
    QFile file(fileName);
    if (file.exists())
    {
        pWorkBook = pWorkBooks->querySubObject("Open(const QString &)", fileName);
    }
    else
    {
        pWorkBooks->dynamicCall("Add");
        pWorkBook = pApplication->querySubObject("ActiveWorkBook");
    }
    pSheets = pWorkBook->querySubObject("Sheets");
    pSheet = pSheets->querySubObject("Item(int)", 1);
}

void result_list::appendSheet(const QString &sheetName)
{
    int cnt = pSheets->property("Count").toInt();
    QAxObject *pLastSheet = pSheets->querySubObject("Item(int)", cnt);
    pSheets->querySubObject("Add(QVariant)", pLastSheet->asVariant());
    pSheet = pSheets->querySubObject("Item(int)", cnt);
    pLastSheet->dynamicCall("Move(QVariant)", pSheet->asVariant());
    pSheet->setProperty("Name", sheetName);
}
void result_list::setCellValue(int row, int column, const QString &value)
{
    QAxObject *pRange = pSheet->querySubObject("Cells(int,int)", row, column);
    pRange->dynamicCall("Value", value);
}
void result_list::saveExcel(const QString &fileName)

{
    pWorkBook->dynamicCall("SaveAs(const QString &)",
                           QDir::toNativeSeparators(fileName));
}
void  result_list::freeExcel()

{
    if (pApplication != NULL)
    {
        pApplication->dynamicCall("Quit()");
        delete pApplication;
        pApplication = NULL;
    }
}

