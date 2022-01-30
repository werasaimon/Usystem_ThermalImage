#ifndef ITCPSERVER_H
#define ITCPSERVER_H

#include <QObject>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>         // std::thread
#include <iostream>
#include <stdio.h>
#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QDebug>

#include "../Common/IVideoThreadThermalCam.h"

class ITcpServer : public QObject
{
    Q_OBJECT
public:
    explicit ITcpServer(QObject *parent = 0);


    void listen(QHostAddress ip = QHostAddress::AnyIPv4, int port=6000);

public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();


private:

    void test(QStringList list);
    void startcam(QStringList list);
    void scam(QStringList list);
    void pos_fov(QStringList list);
    void filter(QStringList list);
    void temperature_range(QStringList list);
    void scale(QStringList list);
    void shutter(QStringList list);

    //--------------------------//
    void mode(QStringList list);
    void detection(QStringList list);
    void threshold(QStringList list);
    void DrawCountur(QStringList list);
    void DrawArea(QStringList list);
    void InvertMask(QStringList list);

    void PointThermal(QStringList list);
    void ThermalOnPoint(QStringList list);

    int m_NumCam;
    QString m_IPClient;

    void operator << (QString _rhs)
    {
        mTcpSocket->write(_rhs.toUtf8());
    }

private:
    QTcpServer * mTcpServer;
    QTcpSocket * mTcpSocket;

    IVideoThread *thread[10];


};

#endif // ITCPSERVER_H
