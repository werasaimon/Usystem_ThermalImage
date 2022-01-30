#ifndef FULLWINDOW_H
#define FULLWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QCloseEvent>
#include <QMessageBox>
#include <QWindow>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QTcpSocket>

namespace Ui {
class FullWindow;
}

class IVideoThread;
class FullWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FullWindow(IVideoThread *_videoCapture = nullptr, QWidget *parent = nullptr);
    ~FullWindow();

    void setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture);

    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event);

    void setTCPSocket(QTcpSocket *newTCPSocket);

private slots:

    void slotTimerAlarm();

private:

    Ui::FullWindow *ui;
    QTimer *timer;

   IVideoThread *mOpenCV_videoCapture;
   QTcpSocket *m_TCPSocket;
};

#endif // FULLWINDOW_H
