#include "widget.hpp"
#include <QPushButton>
#include <QPixmap>
#include <QDebug>
#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "fft.hpp"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->enter_ok, &QPushButton::clicked, this, &Widget::on_enter_ok_clicked);

    // cv::Mat xn = (cv::Mat_<uchar>(1, 3) << 1, 1, 1);
    // cv::Mat Xk = FFT(xn, 4);
    // for(int i=0; i<Xk.size[1]; i++)
    // {
    //     std::cout << "X(" << i << ")=" << Xk.at<std::complex<double>>(0, i) << std::endl;
    // }


    // cv::Mat xnm = (cv::Mat_<uchar>(4, 4) << 1, 1, 1, 1,
    //                                         1, 1, 1, 1,
    //                                         1, 1, 1, 1,
    //                                         1, 1, 1, 1);

    // cv::Mat Xk0 = FFT2D(xnm, "uchar");
    // for(int i=0; i<Xk0.size[0]; i++)
    // {
    //     for(int j=0; j<Xk0.size[1]; j++)
    //     {
    //         std::cout << "X(" << i << "," << j << ")=" << Xk0.at<std::complex<double>>(i, j) << " | ";
    //     }
    //     std::cout << std::endl;
    // }

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
        // QImage gray_image_toshow(gray_image.data, gray_image.cols, gray_image.rows, gray_image.step, QImage::Format_Grayscale8);
        // QPixmap pixmap = QPixmap::fromImage(gray_image_toshow);
        // ui->raw_image->setPixmap(pixmap);

        cv::Mat Xkv = FFT2D(gray_image, "uchar");

        std::cout << "Xkv=" << Xkv.at<std::complex<double>>(0, 0) << std::endl;
        cv::Mat Xkv_abs = cv::Mat_<double>(Xkv.size());
        for(int i=0; i<Xkv.size[0]; i++)
        {
            for(int j=0; j<Xkv.size[1]; j++)
            {
                Xkv_abs.at<double>(i, j) = std::abs(Xkv.at<std::complex<double>>(i, j));
            }
        }
        std::cout << "Xkv_abs=" << Xkv_abs.at<double>(0, 0) << std::endl;

        double minVal, maxVal;
        cv::minMaxLoc(Xkv_abs, &minVal, &maxVal); 
        std::cout << "minVal=" << minVal << ",maxVal=" << maxVal << std::endl;

        cv::Mat Xkv_8u;
        cv::convertScaleAbs(Xkv_abs, Xkv_8u);
        QImage Xkv_8u_toshow(Xkv_8u.data, Xkv_8u.cols, Xkv_8u.rows, Xkv_8u.step, QImage::Format_Grayscale8);
        QPixmap Xkv_8u_pixmap = QPixmap::fromImage(Xkv_8u_toshow);
        ui->raw_image->setPixmap(Xkv_8u_pixmap);
    }
    else
    {
        ui->raw_image->setText("no image");
    }
}