#include "itcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

#include "../Common/blockreader.h"
#include "../Common/scommand.hpp"


namespace
{
    unsigned constexpr const_hash(char const *input)
    {
        return *input ?
                    static_cast<unsigned int>(*input) + 33 * const_hash(input + 1) :
                    5381;
    }

}



ITcpServer::ITcpServer(QObject *parent)
    : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &ITcpServer::slotNewConnection);
}

void ITcpServer::listen(QHostAddress ip, int port)
{
    if(!mTcpServer->listen(ip, port))
    {
        qDebug() << "server is not started";
    }
    else
    {
        qDebug() << "server is started";
    }
}

void ITcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();

    mTcpSocket->write("Hello, World!!! I am echo server!\r\n");
    m_IPClient = mTcpSocket->peerAddress().toString();
    qDebug() << m_IPClient;

    connect(mTcpSocket, &QTcpSocket::readyRead, this, &ITcpServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &ITcpServer::slotClientDisconnected);
}

void ITcpServer::slotServerRead()
{
    while(mTcpSocket->bytesAvailable()>0)
    {


//        QString key;
//        BlockReader br(mTcpSocket);
//        br.stream() >> key;


        QByteArray array = mTcpSocket->readAll();
        QString  str(array);
        QRegExp rx("[ ]");// match a comma or a space
        QStringList list = str.split(rx, QString::SkipEmptyParts);

        QString key_0 = list.at(0);
        switch (const_hash(key_0.toStdString().c_str()))
        {
            case const_hash("startcam"): startcam(list); break;
            case const_hash("scam"): scam(list); break;
            case const_hash("exit"): exit(0); break;
            case const_hash("pos_fov"): pos_fov(list); break;
            case const_hash("filter"): filter(list); break;
            case const_hash("temperature_range"): temperature_range(list); break;
            case const_hash("scale"): scale(list); break;
            case const_hash("shutter"): shutter(list); break;
            case const_hash("detection"): detection(list); break;
            case const_hash("threshold"): threshold(list); break;
            case const_hash("mode"): mode(list); break;

            case const_hash("DrawArea"): DrawArea(list); break;
            case const_hash("DrawCountur"): DrawCountur(list); break;
            case const_hash("InvertMask"): InvertMask(list); break;

            case const_hash("PointThermal"): PointThermal(list); break;
             case const_hash("ThermalOnPoint"): ThermalOnPoint(list); break;

            default:
            mTcpSocket->write("no command \n");
                break;
        }

    }
}

void ITcpServer::slotClientDisconnected()
{
    mTcpSocket->close();
}


void ITcpServer::startcam(QStringList list)
{
    std::string codec = list.at(1).toStdString();
    int n_cam = list.at(2).toInt();
    int bitrate = list.at(3).toInt();
    std::string ip = list.at(4).toStdString();

    if(ip == "my_ip" || ip == "")
    {
        ip = m_IPClient.toStdString();
    }


    //--------------------------------------//
    if (QFile::exists("ccgenericc2.xml"))
    {
        QFile::remove("ccgenericc2.xml");
        std::cout << "DELETE  ccgenericc.xml" << std::endl;
    }
    if (QFile::copy(":/genericc.xml", "ccgenericc2.xml"))
    {
        std::cout << "YES COPY FILE  ccgenericc.xml" << std::endl;
    }
    //--------------------------------------//


    //--------------- Thread Video Capture -----------------//

    const int nn_cameras = (m_NumCam = n_cam);
    for(int i=0; i < nn_cameras; ++i)
    {
        thread[i] = new IVideoThreadThermalCam(nullptr,"ccgenericc2.xml");
        bool isOK = (static_cast<IVideoThreadThermalCam*>(thread[i])->err == 0);

        if(isOK == true)
        {
            if(codec == "H264")
            {
                int fourcc = cv::VideoWriter::fourcc('H','2','6','4');
                thread[i]->VideoWriter().open(cm::udp_gst_H264_send_video(ip,5000+i,bitrate),
                                              fourcc, (double)30, cv::Size(640, 480), true);
                std::cout << "Initilize : H264 \n";
                *this << ( "Initilize : H264 \n" );
            }
            else if(codec == "JPEG")
            {
                int fourcc = cv::VideoWriter::fourcc('J','P','E','G');
                thread[i]->VideoWriter().open(cm::udp_gst_JPEG_send_video(ip,5000+i),
                                              fourcc, (double)30, cv::Size(640, 480), true);
                std::cout << "Initilize : JPEG \n";
                *this << ( "Initilize : JPEG \n" );
            }

            if(thread[i]->VideoWriter().isOpened())
            {
                thread[i]->setIsRun(true);
                thread[i]->start(QThread::HighestPriority);
            }
        }
    }

}

void ITcpServer::scam(QStringList list)
{
    std::string comman = list.at(1).toStdString();
    if(m_NumCam > 0)
    {
        const int nn_cameras = (m_NumCam);
        if(comman == "stop")
        {
            for(int i=0; i<nn_cameras; ++i)
            {
                if( thread[i]->getIsRun())
                {
                  thread[i]->setIsRun(false);
                  thread[i]->exit(0);
                }
               // thread[i]->terminate();
                delete  thread[i];
                thread[i] = nullptr;
            }
            evo_irimager_daemon_kill();
            std::cout << "stop stream thread \n";
            *this <<  "stop stream thread  \n";
        }
        else if(comman == "off")
        {
            for(int i=0; i<nn_cameras; ++i)
            {
                if( thread[i]->getIsRun())
                {
                  thread[i]->setIsRun(false);
                  thread[i]->exit(0);
                }
            }
            ::evo_irimager_terminate();
            std::cout << "stop stream cam \n";
            *this <<  "stop stream cam \n";
        }
        else if(comman == "on")
        {
            for(int i=0; i<nn_cameras; ++i)
            {
                if(!thread[i]->getIsRun())
                {
                    thread[i]->setIsRun(true);
                    if(thread[i]->VideoWriter().isOpened())
                    {
                        thread[i]->start(QThread::HighestPriority);
                    }
                }
            }
            evo_irimager_daemon_is_running();
            std::cout << "start stream cam \n";
            *this <<  "start stream cam \n";
            QThread::usleep(100000);
        }
    }
    else
    {
        std::cout << "No cam : " << m_NumCam << " \n";
        *this << "No cam : " + QString::number(m_NumCam) + " \n";

    }
}

void ITcpServer::pos_fov(QStringList list)
{
    qDebug() << list;
    int pos_fov = list.at(1).toInt();
    ::evo_irimager_set_focusmotor_pos(pos_fov);
}

void ITcpServer::filter(QStringList list)
{
    int filter_index = list.at(1).toInt();
    ::evo_irimager_set_palette(filter_index);
}

void ITcpServer::temperature_range(QStringList list)
{
    int t_min = list.at(1).toInt();
    int t_max = list.at(2).toInt();
    ::evo_irimager_set_temperature_range(t_min,t_max);
}

void ITcpServer::scale(QStringList list)
{
    int index = list.at(1).toInt();
    ::evo_irimager_set_palette_scale(index);
}

void ITcpServer::shutter(QStringList list)
{
  int checked = list.at(1).toInt();
  ::evo_irimager_set_shutter_mode(checked);
}

void ITcpServer::detection(QStringList list)
{
    int isDetection = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.isDetectionThermalArea = isDetection;
    }
}

void ITcpServer::threshold(QStringList list)
{
    int threshold = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.ThresholdDetection = threshold;
    }
}

void ITcpServer::mode(QStringList list)
{
    int mode = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.Mode = mode;
    }
}

void ITcpServer::DrawCountur(QStringList list)
{
    int isDrawCountur = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.isDrawCountur = isDrawCountur;
    }
}

void ITcpServer::DrawArea(QStringList list)
{
    int isDrawArea = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.isDrawArea = isDrawArea;
    }
}

void ITcpServer::InvertMask(QStringList list)
{
    int isInvertMask = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.isInvertMask = isInvertMask;
    }
}

void ITcpServer::PointThermal(QStringList list)
{
  int x = list.at(1).toInt();
  int y = list.at(2).toInt();
  int w = list.at(3).toInt();
  int h = list.at(4).toInt();
  if(thread[0]->Type() == TYPE_CAM::THERMAL)
  {
      float t_w = static_cast<IVideoThreadThermalCam*>(thread[0])->t_w;
      float t_h = static_cast<IVideoThreadThermalCam*>(thread[0])->t_h;

      float unit_w = t_w / w;
      float unit_h = t_h / h;

      float X = floor(x * unit_w);
      float Y = floor(y * unit_h);

      if((X >= 0 && X <= t_w) && (Y >= 0 && Y <= t_h))
      {
          int thermal = static_cast<IVideoThreadThermalCam*>(thread[0])->ThermalPoint(X,Y);
          qDebug() << x << y << thermal;
      }
  }
}

void ITcpServer::ThermalOnPoint(QStringList list)
{
    int isThermalPoint = list.at(1).toInt();
    if(thread[0]->Type() == TYPE_CAM::THERMAL)
    {
      static_cast<IVideoThreadThermalCam*>(thread[0])->m_DispatcherControl.isThermalPoint = isThermalPoint;
    }
}
