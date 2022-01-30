#ifndef LABELVIDEO_H
#define LABELVIDEO_H

#include <QLabel>
#include <QObject>
#include <QMouseEvent>
#include <QDebug>
#include <QTcpSocket>

class FullWindow;
class IVideoThread;
class LabelVideo : public QLabel
{
    Q_OBJECT
public:
    LabelVideo(QWidget *parent = nullptr);

    void mouseDoubleClickEvent( QMouseEvent * event );

    void setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture);

    int IndexCamera() const;
    void setIndexCamera(int newIndexCamera);

    void mouseMoveEvent(QMouseEvent* event);

    void setTCPSocket(QTcpSocket *newTCPSocket);

private:

    int m_IndexCamera;
    IVideoThread *mOpenCV_videoCapture;
    FullWindow *m_FullWindow;

    QTcpSocket *m_TCPSocket;

};

#endif // LABELVIDEO_H
