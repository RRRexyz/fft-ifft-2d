#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <QWidget>
#include "ui_widget.h"

class Widget : public QWidget
{
    public:
        explicit Widget(QWidget *parent = nullptr);
        ~Widget();
    private:
        Ui::Widget *ui;

        void on_enter_ok_clicked();
};


#endif