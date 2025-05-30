#include "widget.hpp"
#include <QPushButton>
#include <QPixmap>
#include <QDebug>
#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->enter_ok, &QPushButton::clicked, this, &Widget::on_enter_ok_clicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_enter_ok_clicked()
{
    QString file_path = ui->file_path->text();
    QPixmap file(file_path);
    if(!file.isNull())
    {
        cv::Mat image = cv::imread(file_path.toStdString());
        cv::Mat gray_image;
        cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
        QImage gray_image_toshow(gray_image.data, gray_image.cols, gray_image.rows, gray_image.step, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(gray_image_toshow);
        ui->raw_image->setPixmap(pixmap);
    }
    else
    {
        ui->raw_image->setText("no image");
    }
}