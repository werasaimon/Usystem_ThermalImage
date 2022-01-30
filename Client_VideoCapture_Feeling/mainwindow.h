#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QMouseEvent>
#include <QTcpSocket>
#include <QCloseEvent>
#include "labelvideo.h"
#include "../Common/scommand.hpp"

#define MAX_CAMERAS_COUNT 4

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);

private slots:
    void onReadyRead();
    void on_pushButton_Connect_clicked();
    void on_pushButton_StreamCam_clicked();
    void on_comboBox_codec_currentTextChanged(const QString &arg1);
    void on_InitOpenCV_button_clicked();
    void on_horizontalSlider_FOV_sliderMoved(int position);
    void on_comboBox_Mode_currentIndexChanged(int index);
    void on_pushButton_2_clicked();

    void on_comboBox_Scale_currentIndexChanged(int index);

    void on_checkBox_Shutter_toggled(bool checked);

    void on_checkBox_DetectionThermalArea_toggled(bool checked);

    void on_horizontalSlider_Thresh_valueChanged(int value);

    void on_comboBox_Method_currentIndexChanged(int index);

    void on_checkBox_DrawCountur_toggled(bool checked);

    void on_checkBox_DrawArea_toggled(bool checked);

    void on_checkBox_InvertMask_toggled(bool checked);

    void on_checkBox_PointThermal_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    IVideoThread *m_VideoCaptureStream[MAX_CAMERAS_COUNT];
    int n_count;

    QTcpSocket  m_TCPSocket;

    bool m_isStream;
    bool m_isStartStreamCamServer;
    bool m_isConnect;
};
#endif // MAINWINDOW_H
