#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <QWidget>
#include "ui_widget.h"
#include <opencv2/opencv.hpp>

class Widget : public QWidget
{
    public:
        explicit Widget(QWidget *parent = nullptr);
        ~Widget();
    private:
        Ui::Widget *ui;
        QPixmap recovered_image_pixmap; // 重建图像的QPixmap
        QPixmap lpf_recovered_image_pixmap; // 低通滤波后的重建图像的QPixmap
        cv::Mat Xkv; // FFT后的结果
        cv::Mat gray_image; // 灰度图
        double recoveredMSE; // 重建图像与原图的均方误差
        double recoveredPSNR; // 重建图像与原图的峰值信噪比
        double lpfMSE; // 低通滤波后的重建图像与原图的均方误差
        double lpfPSNR; // 低通滤波后的重建图像与原图的峰值信噪比

        void on_enter_ok_clicked();
        void on_without_lpf_stateChanged(bool state);
        void on_with_sigma_slider_valueChanged(int value);
        void on_with_sigma_value_valueChanged(int value);
};


#endif