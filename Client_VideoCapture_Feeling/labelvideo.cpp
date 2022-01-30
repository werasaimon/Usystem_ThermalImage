#include "labelvideo.h"
#include "fullwindow.h"
#include "../Common/IVideoThread.h"
#include <QMouseEvent>
#include <QDebug>

LabelVideo::LabelVideo(QWidget *parent) :
    QLabel(parent) ,
    m_TCPSocket(nullptr)
{

}

void LabelVideo::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        qDebug() << "double click";

        if(mOpenCV_videoCapture)
        {
            if(mOpenCV_videoCapture->getIsRun() && !mOpenCV_videoCapture->getIsWindow())
            {
                    mOpenCV_videoCapture->setIsWindow(true);
                    m_FullWindow = new FullWindow(mOpenCV_videoCapture);
                    if(m_TCPSocket) m_FullWindow->setTCPSocket(m_TCPSocket);
                    m_FullWindow->show();

                    qDebug() << "start camera vindow";
            }
            else
            {
                    qDebug() << "this window is already running";
            }

        }
    }
}

void LabelVideo::setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture)
{
    mOpenCV_videoCapture = newOpenCV_videoCapture;
}


int LabelVideo::IndexCamera() const
{
    return m_IndexCamera;
}

void LabelVideo::setIndexCamera(int newIndexCamera)
{
    m_IndexCamera = newIndexCamera;
}

void LabelVideo::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug() << QString::number(event->pos().x());
//    qDebug() << QString::number(event->pos().y());

    if(m_TCPSocket)
    {
          QString command = "PointThermal " + QString::number(event->pos().x()) + " " +
                                              QString::number(event->pos().y()) + " " +
                                              QString::number(width()) + " " +
                                              QString::number(height());
          m_TCPSocket->write(command.toStdString().c_str(),command.toStdString().size());
    }
}

void LabelVideo::setTCPSocket(QTcpSocket *newTCPSocket)
{
    m_TCPSocket = newTCPSocket;
}
