#include "camera.h"
#include "ui_camera.h"

camera::camera(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::camera)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setWindowTitle("摄像头参数输入");
    this->setWindowIcon(QIcon(":/sxt.png"));
    ui->textEdit->setText(QString::number(H2));
    ui->textEdit_2->setText(QString::number(Dmin2));
    ui->textEdit_3->setText(QString::number(Dmax2));
    ui->textEdit_4->setText(QString::number(width2));
    ui->textEdit_5->setText(QString::number(height2));
    ui->textEdit_6->setText(QString::number(Beta2));
}

camera::~camera()
{
    delete ui;
}

void camera::on_pushButton_clicked()
{
    H2=ui->textEdit->toPlainText().toDouble();
    Dmin2=ui->textEdit_2->toPlainText().toDouble();
    Dmax2=ui->textEdit_3->toPlainText().toDouble();
    width2=ui->textEdit_4->toPlainText().toDouble();
    height2=ui->textEdit_5->toPlainText().toDouble();
    Beta2=ui->textEdit_6->toPlainText().toDouble();
    QMessageBox::information(this,tr("提示"),tr("输入成功！"));
    this->close();
}
