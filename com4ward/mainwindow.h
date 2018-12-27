#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mqtt/client.h>
#include <memory>
#include <QPointer>
#include <QTcpSocket>
#include <QProcess>
#include <QLabel>

#include "logger.h"
#include "com2tcpthread.h"
#include "qextserialport.h"

/////////////////////////////////////////////////////////////////////////////
// Class to receive callbacks

class MainWindow;
class user_callback : public virtual mqtt::callback
{
public:
    user_callback(MainWindow* wnd);
    virtual void connected(const string& cause) override;
    void connection_lost(const std::string& cause) override;
    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override;


public:
    MainWindow* m_wnd;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

     void openCommPort();

private slots:
     void onComReadyRead();
     void onComDsrChanged(bool status);


private slots:
    void on_btnStart_clicked();
    void readTcpData();
    void c2tProcStarted();
    void c2tProcFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void c2tProcStateChanged(QProcess::ProcessState state);
    void c2tProcReadyRead();
    void c2tProcReadyReadStandardOutput();

    void on_btnStart_clicked(bool checked);

    void enableInput(bool enabled);

    void createMqttClient();
    void createSocketClient();

    void start();
    void stop();

    void readConfig();
    void saveConfig();

    void on_btnSubscribe_clicked(bool checked);

public:
    Ui::MainWindow *ui;

    std::unique_ptr<mqtt::client> m_client;
    std::unique_ptr<user_callback> m_callback;
//    std::unique_ptr<sample_mem_persistence> m_persist;
    QPointer<Logger> m_logger;
    QPointer<QTcpSocket> m_socket;
    QString m_publishTopic;
    QString m_subscribeTopic;
    QPointer<Com2TcpThread> m_c2tThread;

    QString m_comPath;
    QString m_hostAddr;
    QString m_hostPort;
    QPointer<QProcess> m_c2tProcess;
    bool m_isRunning=false;
    std::string m_clientID;
    QPointer<QextSerialPort> m_port;

};

#endif // MAINWINDOW_H
