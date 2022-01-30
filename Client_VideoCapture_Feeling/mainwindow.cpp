#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QResizeEvent>
#include <QHostAddress>
#include <QDebug>
#include <QMessageBox>

#include "../Common/IVideoThread.h"

namespace
{

std::string lisen_UDP_gst_JPEG_read_video( int port , int fps)
{
    std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
            " ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,"
            "framerate=" + std::to_string(fps) + "/1 ! "
            "rtpjpegdepay ! jpegdec ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
    return gst_video_send;
}


std::string lisen_UDP_gst_H264_read_video( int port , int fps)
{
    std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
            " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96,"
            "framerate=" + std::to_string(fps) + "/1 ! "
            "rtph264depay ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
    return gst_video_send;
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("streming thermal camera");
    connect(&m_TCPSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    //-------------------------------------------------------------------------------//
    n_count = 1;
    m_VideoCaptureStream[0] =  new IVideoThread(this , "Camera " + QString::number(1));
    ui->labelOpenCVFrame->setIndexCamera(0);
    ui->labelOpenCVFrame->setOpenCV_videoCapture(m_VideoCaptureStream[0]);

    connect(m_VideoCaptureStream[0], &IVideoThread::newPixmapCapture, this, [&]()
    {
       if(m_VideoCaptureStream[0]->getIsRun())
       {
           int w = ui->labelOpenCVFrame->width();
           int h = ui->labelOpenCVFrame->height();
           ui->labelOpenCVFrame->setPixmap(m_VideoCaptureStream[0]->pixmap().scaled(w,h));
       }
       else
       {
           ui->labelOpenCVFrame->setText("Camera[0]");
       }
    });
    //-------------------------------------------------------------------------------//

    m_isStream=true;
    m_isStartStreamCamServer=false;
    m_isConnect=false;

    //..-----------------------------------------..//
    ui->checkBox_Shutter->setEnabled(false);
    ui->horizontalSlider_FOV->setEnabled(false);
    ui->comboBox_Scale->setEnabled(false);
    ui->comboBox_Mode->setEnabled(false);
    ui->label_FOV->setEnabled(false);
    ui->label_2->setEnabled(false);
    ui->label_4->setEnabled(false);
    ui->spinBox_temperatureMIN->setEnabled(false);
    ui->spinBox_temperatureMAX->setEnabled(false);
    ui->pushButton_ApplyTemperatureRange->setEnabled(false);
    //..-----------------------------------------..//

}

MainWindow::~MainWindow()
{
    for (int i = 0; i < n_count; ++i)
    {
        if(m_VideoCaptureStream[i])
        {
            delete m_VideoCaptureStream[i];
            m_VideoCaptureStream[i] = nullptr;
        }
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);

    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "PROGRAM EXIT",
                                                                tr("Exit to App \n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
    {
        event->ignore();
    }
    else
    {
        if(m_isStartStreamCamServer)
        {
            QString start_command = "scam stop \n";
            m_TCPSocket.write(start_command.toStdString().c_str(),start_command.toStdString().size());
        }

        m_isStartStreamCamServer = false;
        event->accept();
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
   // qDebug() << QString::number(event->pos().x()) << QString::number(event->pos().y());
}

void MainWindow::on_pushButton_Connect_clicked()
{
    if(!m_isConnect)
    {
        QString IP = ui->lineEdit_IP->text();
        m_TCPSocket.connectToHost(QHostAddress(IP), 6000);

         if (m_TCPSocket.waitForConnected(1000))
         {
             qDebug("Connected!");
             m_isConnect = true;
             ui->pushButton_Connect->setText("Disconnect");

             connect(&m_TCPSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

             ui->labelOpenCVFrame->setTCPSocket(&m_TCPSocket);
         }
         else
         {
            qDebug("No Connected!");
            m_isConnect = false;
         }
    }
    else
    {
        m_isConnect = false;
        ui->pushButton_Connect->setText("Connect");
        m_TCPSocket.disconnectFromHost();
        ui->lineEdit_IP->setEnabled(true);
    }
}

void MainWindow::on_pushButton_StreamCam_clicked()
{
    if(m_isStartStreamCamServer == false)
    {
        QString codec = ui->comboBox_codec->currentText();
        int bitrate = ui->spinBox_Bitrate->value();
        int count_cam = n_count;
        QString start_command = "startcam " + codec + " " +
                QString::number(count_cam) + " " +
                QString::number(bitrate) + " my_ip \n";
        m_TCPSocket.write(start_command.toStdString().c_str(),start_command.toStdString().size());
        m_isStartStreamCamServer = true;
    }
    else
    {
        m_isStartStreamCamServer = false;
        QString start_command = "scam stop \n";
        m_TCPSocket.write(start_command.toStdString().c_str(),start_command.toStdString().size());
    }
}

void MainWindow::on_comboBox_codec_currentTextChanged(const QString &arg1)
{
    if(arg1 == QString("H264"))
    {
        ui->spinBox_Bitrate->setEnabled(true);
    }
    else if(arg1 == QString("JPEG"))
    {
        ui->spinBox_Bitrate->setEnabled(false);
    }
}

void MainWindow::onReadyRead()
{
    QByteArray datas = m_TCPSocket.readAll();
    qDebug() << datas;
}


void MainWindow::on_InitOpenCV_button_clicked()
{
    if(m_isStream)
    {
        for (int i = 0; i < n_count; ++i)
        {
            bool isOpen=false;
           if(ui->comboBox_codec->currentText() == QString("H264"))
           {
               isOpen=m_VideoCaptureStream[i]->VideoCapture().open(lisen_UDP_gst_H264_read_video(5000+i,30));
           }
           else if(ui->comboBox_codec->currentText() == QString("JPEG"))
           {
               isOpen=m_VideoCaptureStream[i]->VideoCapture().open(lisen_UDP_gst_JPEG_read_video(5000+i,30));
           }

           if(isOpen == true)
           {
               qDebug() << "Start Stream";
               m_VideoCaptureStream[i]->setIsRun(true);
               m_VideoCaptureStream[i]->start(QThread::HighestPriority);

               //..-----------------------------------------..//
               ui->checkBox_Shutter->setEnabled(true);
               ui->horizontalSlider_FOV->setEnabled(true);
               ui->comboBox_Scale->setEnabled(true);
               ui->comboBox_Mode->setEnabled(true);
               ui->label_FOV->setEnabled(true);
               ui->label_2->setEnabled(true);
               ui->label_4->setEnabled(true);
               ui->spinBox_temperatureMIN->setEnabled(true);
               ui->spinBox_temperatureMAX->setEnabled(true);
               ui->pushButton_ApplyTemperatureRange->setEnabled(true);
               //..-----------------------------------------..//
           }
        }

        ui->InitOpenCV_button->setText("Stop");
        m_isStream=false;
        ui->comboBox_codec->setEditable(false);
        ui->comboBox_codec->setDisabled(true);
    }
    else
    {
        for (int i = 0; i < n_count; ++i)
        {
            m_VideoCaptureStream[i]->setIsRun(false);
            m_VideoCaptureStream[i]->exit(0);
        }

        ui->InitOpenCV_button->setText("Start");
        m_isStream=true;
        ui->comboBox_codec->setEditable(true);
        ui->comboBox_codec->setDisabled(false);


        //..-----------------------------------------..//
        ui->checkBox_Shutter->setEnabled(false);
        ui->horizontalSlider_FOV->setEnabled(false);
        ui->comboBox_Scale->setEnabled(false);
        ui->comboBox_Mode->setEnabled(false);
        ui->label_FOV->setEnabled(false);
        ui->label_2->setEnabled(false);
        ui->label_4->setEnabled(false);
        ui->spinBox_temperatureMIN->setEnabled(false);
        ui->spinBox_temperatureMAX->setEnabled(false);
        ui->pushButton_ApplyTemperatureRange->setEnabled(false);
        //..-----------------------------------------..//

    }
}

void MainWindow::on_horizontalSlider_FOV_sliderMoved(int position)
{
    qDebug() << " FOV: " << position;
    ui->label_FOV->setText(" FOV: " + QString::number(position));
    QString command = "pos_fov " + QString::number(position);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_comboBox_Mode_currentIndexChanged(int index)
{
    qDebug() << "Index Filter: " << index+1;
    QString command = "filter " + QString::number(index+1);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_pushButton_2_clicked()
{
    int t_min = ui->spinBox_temperatureMIN->value();
    int t_max = ui->spinBox_temperatureMAX->value();
    qDebug() << "Temperature Range: " << t_min << "-" << t_max;
    QString command = "temperature_range " + QString::number(t_min) + " " + QString::number(t_max);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_comboBox_Scale_currentIndexChanged(int index)
{
    qDebug() << "Index scale: " << index+1;
    QString command = "scale " + QString::number(index+1);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_Shutter_toggled(bool checked)
{
    QString command = "shutter " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_DetectionThermalArea_toggled(bool checked)
{
    QString command = "detection " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_horizontalSlider_Thresh_valueChanged(int value)
{
    QString command = "threshold " + QString::number(value);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_comboBox_Method_currentIndexChanged(int index)
{
    QString command = "mode " + QString::number((index+1));
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_DrawCountur_toggled(bool checked)
{
    QString command = "DrawCountur " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_DrawArea_toggled(bool checked)
{
    QString command = "DrawArea " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_InvertMask_toggled(bool checked)
{
    QString command = "InvertMask " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}


void MainWindow::on_checkBox_PointThermal_toggled(bool checked)
{
    QString command = "ThermalOnPoint " + QString::number(checked);
    m_TCPSocket.write(command.toStdString().c_str(),command.toStdString().size());
}

