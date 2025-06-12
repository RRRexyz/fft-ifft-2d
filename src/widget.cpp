#include "widget.hpp"
#include <QPushButton>
#include <QPixmap>
#include <QDebug>
#include <QString>
#include <QImage>
#include "fft.hpp"
#include "ifft.hpp"
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>


Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->sigma_prompt->hide();
    ui->sigma_slider->hide();
    ui->sigma_value->hide();

    connect(ui->enter_ok, &QPushButton::clicked, this, &Widget::on_enter_ok_clicked);
    connect(ui->without_lpf, &QRadioButton::toggled, this, &Widget::on_without_lpf_stateChanged);
    connect(ui->sigma_slider, &QSlider::valueChanged, this, &Widget::on_with_sigma_slider_valueChanged);
    connect(ui->sigma_value, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &Widget::on_with_sigma_value_valueChanged);

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
        cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
        QImage gray_image_toshow(gray_image.data, gray_image.cols, gray_image.rows, gray_image.step, QImage::Format_Grayscale8);
        QPixmap raw_image_pixmap = QPixmap::fromImage(gray_image_toshow);
        ui->raw_image->setPixmap(raw_image_pixmap);

        Xkv = FFT2D(gray_image, "uchar");
        cv::Mat Xkv_abs = cv::Mat_<double>(Xkv.size());
        for(int i=0; i<Xkv.size[0]; i++)
        {
            for(int j=0; j<Xkv.size[1]; j++)
            {
                Xkv_abs.at<double>(i, j) = std::abs(Xkv.at<std::complex<double>>(i, j));
            }
        }

        // 将频域图转换为灰度图显示出来
        cv::Mat Xkv_8u;
        cv::convertScaleAbs(Xkv_abs, Xkv_8u);
        QImage Xkv_8u_toshow(Xkv_8u.data, Xkv_8u.cols, Xkv_8u.rows, Xkv_8u.step, QImage::Format_Grayscale8);
        QPixmap Xkv_8u_pixmap = QPixmap::fromImage(Xkv_8u_toshow);
        ui->fft_image->setPixmap(Xkv_8u_pixmap);

        // 直接对频域图进行IFFT，得到复原图像
        cv::Mat xnm_recovered = IFFT2D(Xkv, gray_image.size[0], gray_image.size[1], "uchar");
        QImage recovered_image_toshow(xnm_recovered.data, xnm_recovered.cols, xnm_recovered.rows, xnm_recovered.step, QImage::Format_Grayscale8);
        recovered_image_pixmap = QPixmap::fromImage(recovered_image_toshow);
        recoveredMSE = computeMSE(gray_image, xnm_recovered);
        recoveredPSNR = computePSNR(gray_image, xnm_recovered);

        // 先对频域图进行低通滤波，再进行IFFT，得到复原图像
        cv::Mat Xkv_filtered = cv::Mat_<std::complex<double>>(Xkv.size[0], Xkv.size[1]);
        Xkv_filtered.setTo(0);
        cv::Mat filter = createGaussianLPF(cv::Size(Xkv.size[0], Xkv.size[1]), ui->sigma_value->value());

        for(int i=0; i<Xkv.size[0]; i++)
        for(int j=0; j<Xkv.size[1]; j++)
        {
            Xkv_filtered.at<std::complex<double>>(i, j) = Xkv.at<std::complex<double>>(i, j) * filter.at<std::complex<double>>(i, j);
        }
        cv::Mat xnm_filtered_recovered = IFFT2D(Xkv_filtered, gray_image.size[0], gray_image.size[1], "uchar");
        QImage lpf_recovered_image_toshow(xnm_filtered_recovered.data, xnm_filtered_recovered.cols, xnm_filtered_recovered.rows, xnm_filtered_recovered.step, QImage::Format_Grayscale8);
        lpf_recovered_image_pixmap = QPixmap::fromImage(lpf_recovered_image_toshow);
        lpfMSE = computeMSE(gray_image, xnm_filtered_recovered);
        lpfPSNR = computePSNR(gray_image, xnm_filtered_recovered);

        if(ui->without_lpf->isChecked()) 
        {
            ui->recovered_image->setPixmap(recovered_image_pixmap);
            ui->vs_prompt->setText("Recovered Image VS Original Image:");
            ui->mse_value->setText(QString::number(recoveredMSE));
            ui->psnr_value->setText(QString::number(recoveredPSNR)+"dB");
        }
        else if(ui->with_lpf->isChecked())
        {
            ui->recovered_image->setPixmap(lpf_recovered_image_pixmap);
            ui->vs_prompt->setText("Filtered Image VS Original Image:");
            ui->mse_value->setText(QString::number(lpfMSE));
            ui->psnr_value->setText(QString::number(lpfPSNR)+"dB");
        }
    }
    else
    {
        ui->raw_image->setText("no image");
    }
}


void Widget::on_without_lpf_stateChanged(bool state)
{
    if(state) // 选中了“不进行低通滤波”选项
    {
        if(!recovered_image_pixmap.isNull())
        {
            ui->recovered_image->setPixmap(recovered_image_pixmap);
        }
        else
        {
            ui->recovered_image->setText("no image");
        }
        ui->sigma_prompt->hide();
        ui->sigma_slider->hide();
        ui->sigma_value->hide();
        ui->vs_prompt->setText("Recovered Image VS Original Image:");
        ui->mse_value->setText(QString::number(recoveredMSE));
        ui->psnr_value->setText(QString::number(recoveredPSNR)+"dB");
    }
    else // 选中了“进行低通滤波”选项
    {
        if(!lpf_recovered_image_pixmap.isNull())
        {
            ui->recovered_image->setPixmap(lpf_recovered_image_pixmap);
        }
        else
        {
            ui->recovered_image->setText("no image");
        }
        ui->sigma_prompt->show();
        ui->sigma_slider->show();
        ui->sigma_value->show();
        ui->vs_prompt->setText("Filtered Image VS Original Image:");
        ui->mse_value->setText(QString::number(lpfMSE));
        ui->psnr_value->setText(QString::number(lpfPSNR)+"dB");
    }
}


void Widget::on_with_sigma_slider_valueChanged(int value)
{
    ui->sigma_value->setValue(value);
    cv::Mat Xkv_filtered = cv::Mat_<std::complex<double>>(Xkv.size[0], Xkv.size[1]);
    Xkv_filtered.setTo(0);
    cv::Mat filter = createGaussianLPF(cv::Size(Xkv.size[0], Xkv.size[1]), ui->sigma_value->value());

    for(int i=0; i<Xkv.size[0]; i++)
    for(int j=0; j<Xkv.size[1]; j++)
    {
        Xkv_filtered.at<std::complex<double>>(i, j) = Xkv.at<std::complex<double>>(i, j) * filter.at<std::complex<double>>(i, j);
    }
    cv::Mat xnm_filtered_recovered = IFFT2D(Xkv_filtered, gray_image.size[0], gray_image.size[1], "uchar");
    QImage lpf_recovered_image_toshow(xnm_filtered_recovered.data, xnm_filtered_recovered.cols, xnm_filtered_recovered.rows, xnm_filtered_recovered.step, QImage::Format_Grayscale8);
    lpf_recovered_image_pixmap = QPixmap::fromImage(lpf_recovered_image_toshow);

    if(ui->with_lpf->isChecked())
    {
        ui->recovered_image->setPixmap(lpf_recovered_image_pixmap);
    }
}


void Widget::on_with_sigma_value_valueChanged(int value)
{
    ui->sigma_slider->setValue(value);
    cv::Mat Xkv_filtered = cv::Mat_<std::complex<double>>(Xkv.size[0], Xkv.size[1]);
    Xkv_filtered.setTo(0);
    cv::Mat filter = createGaussianLPF(cv::Size(Xkv.size[0], Xkv.size[1]), ui->sigma_value->value());

    for(int i=0; i<Xkv.size[0]; i++)
    for(int j=0; j<Xkv.size[1]; j++)
    {
        Xkv_filtered.at<std::complex<double>>(i, j) = Xkv.at<std::complex<double>>(i, j) * filter.at<std::complex<double>>(i, j);
    }
    cv::Mat xnm_filtered_recovered = IFFT2D(Xkv_filtered, gray_image.size[0], gray_image.size[1], "uchar");
    QImage lpf_recovered_image_toshow(xnm_filtered_recovered.data, xnm_filtered_recovered.cols, xnm_filtered_recovered.rows, xnm_filtered_recovered.step, QImage::Format_Grayscale8);
    lpf_recovered_image_pixmap = QPixmap::fromImage(lpf_recovered_image_toshow);

    if(ui->with_lpf->isChecked())
    {
        ui->recovered_image->setPixmap(lpf_recovered_image_pixmap);
    }

    lpfMSE = computeMSE(gray_image, xnm_filtered_recovered);
    lpfPSNR = computePSNR(gray_image, xnm_filtered_recovered);
    ui->mse_value->setText(QString::number(lpfMSE));
    ui->psnr_value->setText(QString::number(lpfPSNR)+"dB");
}