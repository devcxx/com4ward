#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qextserialenumerator.h>
#include <QSettings>
#include <iostream>
#include <sstream>
#include <string>
#include "keyhelper.h"

const std::string SERVER_ADDRESS { "tcp://cig.nengxin.com.cn:1883" };
const std::string CLIENT_ID { "sync_publish_cpp" };
const std::string TOPIC { "/broadcast/200001/cominput/" };
const std::string MQTT_USER { "nengxin" };
const std::string MQTT_PASS { "NX@)!*" };

//const int QOS = 1;

/*
    Hash function by Peter J. Weinberger
    Archive of Reversing.ID
    Non-Cryptographic Hash
    Assemble:
    (gcc)
    $ g++ -m32 -S -masm=intel -o PJWHash.asm PJWHash.cpp
    (msvc)
    $ cl /c /FaPJWHash.asm PJWHash.cpp
    */
static uint32_t _PJWHash(const std::string& key)
{
    const uint32_t BitsInUnsignedInt = (uint32_t)(sizeof(uint32_t) * 8);
    const uint32_t ThreeQuarters = (uint32_t)((BitsInUnsignedInt * 3) / 4);
    const uint32_t OneEight = (uint32_t)(BitsInUnsignedInt / 8);
    const uint32_t HighBits = (uint32_t)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEight);

    uint32_t test = 0;
    uint32_t state = 0;

    for (size_t i = 0; i < key.length(); i++)
    {
        state = (state << OneEight) + key[i];

        // HighBits tidak diset
        test = state & HighBits;
        if (test)
        {
            state = ((state ^ (test >> ThreeQuarters)) & ~HighBits);
        }
    }
    return state;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_logger = new Logger(this, "log.txt", ui->editLog);

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    printf("List of ports:\n");
    for (int i = 0; i < ports.size(); i++) {
        //		printf("port name: %s\n", ports.at(i).portName.toLocal8Bit().constData());
        ui->cbxPort->addItem(ports.at(i).portName.toLocal8Bit().constData());
        //		printf("friendly name: %s\n", ports.at(i).friendName.toLocal8Bit().constData());
        //		printf("physical name: %s\n", ports.at(i).physName.toLocal8Bit().constData());
        //		printf("enumerator name: %s\n", ports.at(i).enumName.toLocal8Bit().constData());
        //		printf("===================================\n\n");
    }

    setWindowTitle("COM串口数据转发(COM port to MQTT redirector) v1.0");
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    setFixedSize(415,620);
    ui->lblC2TState->setText("Not Running");
    ui->lblMqttState->setText("Not Connected");
    ui->lblSocketState->setText("Not Connected");

    QStringList baudRates { "9600", "19200", "38400", "57600", "115200"};
    ui->cbxBaudRate->addItems(baudRates);
    ui->cbxBaudRate->setCurrentIndex(1);

    QStringList dataBits { "5", "6", "7", "8" };
    ui->cbxDataBits->addItems(dataBits);
    ui->cbxDataBits->setCurrentIndex(3);

    QStringList paritys { "no", "even", "odd", "mark", "space" };
    ui->cbxParity->addItems(paritys);
    ui->cbxParity->setCurrentIndex(0);

    QStringList stopBits { "1", "1.5", "2" };
    ui->cbxStopBits->addItems(stopBits);
    ui->cbxStopBits->setCurrentIndex(0);

    QStringList flowTypes { "off", "hardware", "xonxoff" };
    ui->cbxFlowType->addItems(flowTypes);
    ui->cbxFlowType->setCurrentIndex(0);

    ui->ledCom2Tcp->turnOff();
    ui->ledMqtt->turnOff();
    ui->ledSocket->turnOff();

    //     m_c2tThread = new Com2TcpThread();
    m_c2tProcess = new QProcess;
    connect(m_c2tProcess,SIGNAL(started()),SLOT(c2tProcStarted()));
    connect(m_c2tProcess,SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(c2tProcFinished(int,QProcess::ExitStatus)));
    connect(m_c2tProcess,SIGNAL(stateChanged(QProcess::ProcessState)),SLOT(c2tProcStateChanged(QProcess::ProcessState)));

    connect(m_c2tProcess,SIGNAL(readyRead()),this,SLOT(c2tProcReadyRead()));
    connect(m_c2tProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(c2tProcReadyReadStandardOutput()));

//    QByteArray machine_id;
//    KeyHelper::getMachineId(&machine_id);
//    auto midHash = static_cast<quint32>(_PJWHash(machine_id.toBase64().toStdString()));
//    stringstream ss;
//    ss << "c4w" << midHash;
//    m_clientID = ss.str();

//    qDebug() << QString::fromStdString(m_clientID);

    readConfig();

//    initSocketClient();
}

static std::vector<BaudRateType> BaudRateArgs {
    BAUD9600, BAUD19200, BAUD38400, BAUD56000, BAUD115200
};

static std::vector<DataBitsType> DataBitsArgs {
    DATA_5, DATA_6, DATA_7, DATA_8
};

static std::vector<ParityType> ParityArgs {
    PAR_NONE, PAR_EVEN, PAR_ODD, PAR_MARK, PAR_SPACE
};

static std::vector<FlowType> FlowTypes {
    FLOW_OFF, FLOW_HARDWARE, FLOW_XONXOFF
};

static std::vector<StopBitsType> StopBitsArgs
{
    STOP_1, STOP_1_5, STOP_2
};

void MainWindow::openCommPort()
{
    QString portName = ui->cbxPort->currentText();
    m_port = new QextSerialPort(portName, QextSerialPort::EventDriven);
    m_port->setBaudRate(BaudRateArgs.at(ui->cbxBaudRate->currentIndex()));
    m_port->setFlowControl(FlowTypes.at(ui->cbxFlowType->currentIndex()));
    m_port->setParity(ParityArgs.at(ui->cbxParity->currentIndex()));
    m_port->setDataBits(DataBitsArgs.at(ui->cbxDataBits->currentIndex()));
    m_port->setStopBits(StopBitsArgs.at(ui->cbxStopBits->currentIndex()));

    if (m_port->open(QIODevice::ReadWrite) == true) {
        connect(m_port, SIGNAL(readyRead()), this, SLOT(onComReadyRead()));
        connect(m_port, SIGNAL(dsrChanged(bool)), this, SLOT(onComDsrChanged(bool)));
        if (!(m_port->lineStatus() & LS_DSR))
            qDebug() << "warning: device is not turned on";
        qDebug() << "listening for data on" << m_port->portName();
    }
    else {
        qDebug() << "device failed to open:" << m_port->errorString();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    stop();
}

void MainWindow::onComReadyRead()
{
    QByteArray bytes;
    int a = m_port->bytesAvailable();
    bytes.resize(a);
    m_port->read(bytes.data(), bytes.size());
    qDebug() << "bytes read:" << bytes.size();
    qDebug() << "bytes:" << bytes;

    m_client->publish(m_publishTopic.toStdString(), bytes.data(), bytes.length());
    m_logger->write(bytes);
}

void MainWindow::onComDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}


void MainWindow::on_btnStart_clicked()
{

}

void MainWindow::readTcpData()
{
    QByteArray ba = m_socket->readAll();
    try {
        auto pubmsg = mqtt::make_message(m_publishTopic.toStdString(), ba.data());
        pubmsg->set_qos(1);
        m_client->publish(pubmsg);

    } catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << std::endl;
    }
    m_logger->write(ba);
}

void MainWindow::c2tProcStarted()
{
    qDebug()<<"started";
    createSocketClient();
}

void MainWindow::c2tProcFinished(int exitCode,QProcess::ExitStatus exitStatus)
{
    qDebug()<<"finished";
    qDebug()<<exitCode;// 被调用程序的main返回的int
    qDebug()<<exitStatus;// QProcess::ExitStatus(NormalExit)
    qDebug() <<"finished-output-readAll:";
    qDebug()<<QString::fromLocal8Bit(m_c2tProcess->readAll());// ""
    qDebug()<<"finished-output-readAllStandardOutput:";
    qDebug()<<QString::fromLocal8Bit(m_c2tProcess->readAllStandardOutput());// ""
}

void MainWindow::c2tProcStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
    case QProcess::NotRunning:
        qDebug()<<"Not Running";
        ui->lblC2TState->setText("Not Running");
        ui->ledCom2Tcp->turnOff();
        break;
    case QProcess::Starting:
        qDebug()<<"Starting";
        ui->lblC2TState->setText("Starting");
        break;
    case QProcess::Running:
        qDebug()<<"Running";
        ui->lblC2TState->setText("Running");
        ui->ledCom2Tcp->turnOn();
        break;
    default:
        qDebug()<<"otherState";
        ui->lblC2TState->setText("OtherState");
        break;
    }
}

void MainWindow::c2tProcReadyRead()
{
    qDebug()<<"readyRead-readAll:";
    m_logger->write(QString::fromLocal8Bit(m_c2tProcess->readAll()));// "hello it is ok!"
    qDebug()<<"readyRead-readAllStandardOutput:";
    m_logger->write(QString::fromLocal8Bit(m_c2tProcess->readAllStandardOutput()));// ""
}

void MainWindow::c2tProcReadyReadStandardOutput()
{
    qDebug()<<"readyReadStandardOutput-readAll:";
    m_logger->write(QString::fromLocal8Bit(m_c2tProcess->readAll()));// ""
    qDebug()<<"readyReadStandardOutput-readAllStandardOutput:";
    m_logger->write(QString::fromLocal8Bit(m_c2tProcess->readAllStandardOutput()));// ""
}

void MainWindow::on_btnStart_clicked(bool checked)
{
    if (checked) {
        start();
        ui->btnStart->setText("停止(Stop)");
        enableInput(false);
        saveConfig();
    } else {
        stop();
        enableInput(true);
        ui->btnStart->setText("启动(Start)");
    }
}

void MainWindow::enableInput(bool enabled)
{
    if (enabled) {
        ui->editClientID->setDisabled(false);
        ui->editPublishTopic->setDisabled(false);
        ui->editSubscribeTopic->setDisabled(false);
        ui->cbxPort->setDisabled(false);
        ui->cbxBaudRate->setDisabled(false);
        ui->cbxDataBits->setDisabled(false);
        ui->cbxParity->setDisabled(false);
        ui->cbxStopBits->setDisabled(false);
        ui->cbxFlowType->setDisabled(false);
    } else {
        ui->editClientID->setDisabled(true);
        ui->editPublishTopic->setDisabled(true);
        ui->editSubscribeTopic->setDisabled(true);
        ui->cbxPort->setDisabled(true);
        ui->cbxBaudRate->setDisabled(true);
        ui->cbxDataBits->setDisabled(true);
        ui->cbxParity->setDisabled(true);
        ui->cbxStopBits->setDisabled(true);
        ui->cbxFlowType->setDisabled(true);
    }
}

void MainWindow::createMqttClient()
{
    m_clientID = ui->editClientID->text().toStdString();
    std::unique_ptr<mqtt::client> client(new mqtt::client(SERVER_ADDRESS, m_clientID));
    m_client = std::move(client) ;

    std::unique_ptr<user_callback> callback(new user_callback(this));
    m_callback = std::move(callback);


    mqtt::connect_options connOpts;
    connOpts.set_user_name(MQTT_USER);
    connOpts.set_password(MQTT_PASS);
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(false);
    connOpts.set_automatic_reconnect(true);

    std::cout << "...OK" << std::endl;


    m_client->set_callback(*(m_callback.get()));

    m_publishTopic = ui->editPublishTopic->text();
    m_subscribeTopic = ui->editSubscribeTopic->text();

    try {
        std::cout << "\nConnecting..." << std::endl;
        m_client->connect(connOpts);
        m_client->subscribe(m_subscribeTopic.toStdString());
        ui->ledMqtt->turnOn();
    } catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return;
    }
}

void MainWindow::createSocketClient()
{
    // Start TCP client
    m_socket = new QTcpSocket(this);
    connect( m_socket, SIGNAL(readyRead()), SLOT(readTcpData()) );

    QString strTcpPort;
//    int port = ui->editTcpPort->text().toInt();
    int port = strTcpPort.toInt();
    m_socket->connectToHost("127.0.0.1", port);
    if( m_socket->waitForConnected() ) {
        //            _pSocket->write( data );
        ui->lblSocketState->setText("Connected");
        ui->ledSocket->turnOn();
    }
}



void MainWindow::start()
{
    createMqttClient();

//    QStringList args;

//    int baudRateIndex = ui->cbxBaudRate->currentIndex();
//    args.append("--baud");
//    args.append(BaudRateArgs.at(baudRateIndex));
//    int dataBitIndex = ui->cbxDataBits->currentIndex();
//    args.append("--data");
//    args.append(DataBitsArgs.at(dataBitIndex));
//    int parityIndex = ui->cbxParity->currentIndex();
//    args.append("--parity");
//    args.append(ParityArgs.at(parityIndex));
//    int stopBitIndex = ui->cbxStopBits->currentIndex();
//    args.append("--stop");
//    args.append(StopBitsArgs.at(stopBitIndex));

//    bool ignoreDSR = ui->chkIgnoreDSR->isChecked();
//    if (ignoreDSR)
//        args.append("--ignore-dsr");

//    QString comPort = QString("\\\\.\\%1").arg(ui->cbxPort->currentText());
//    args.append(comPort);

//    QString tcpPort = ui->editTcpPort->text();
//    args.append(tcpPort);

//    QString launchPath = QCoreApplication::applicationDirPath() + "/com2tcp.exe";
//    qDebug() << launchPath;

//    qDebug() << args;

//    m_c2tProcess->start(launchPath, args);
    openCommPort();

}

void MainWindow::stop()
{
    if (m_client->is_connected()) {
        m_client->disconnect();
        ui->ledMqtt->turnOff();
    }
//    if (m_c2tProcess->state() == QProcess::Running) {
//        m_c2tProcess->close();
//        ui->ledCom2Tcp->turnOff();
//    }
//    if (m_socket->isOpen()) {
//        m_socket->close();
//        ui->ledSocket->turnOff();
//    }
    if (m_port->isOpen()) {
        m_port->close();
    }
}

void MainWindow::readConfig()
{
    QSettings *configIniRead = new QSettings("./conf.ini", QSettings::IniFormat);
    QString clientID = configIniRead->value("/Configure/ClientID").toString();
    QString publishTopic = configIniRead->value("/Configure/PublishTopic").toString();
    QString subscribeTopic = configIniRead->value("/Configure/SubscribeTopic").toString();

    int portIndex = configIniRead->value("/COM/PortIndex").toInt();
    int baudRateIndex = configIniRead->value("/COM/BaudRate", 1).toInt();
    int dataBitIndex = configIniRead->value("/COM/DataBit", 3).toInt();
    int parityIndex = configIniRead->value("/COM/Parity", 0).toInt();
    int stopBitIndex = configIniRead->value("/COM/StopBit", 0).toInt();
    int flowTypeIndex = configIniRead->value("/COM/FlowType", 0).toInt();
//    bool ignoreDSR = configIniRead->value("/COM/IgnoreDSR", false).toBool();

    ui->editClientID->setText(clientID);
    ui->editPublishTopic->setText(publishTopic);
    ui->editSubscribeTopic->setText(subscribeTopic);

    ui->cbxPort->setCurrentIndex(portIndex);
    ui->cbxBaudRate->setCurrentIndex(baudRateIndex);
    ui->cbxDataBits->setCurrentIndex(dataBitIndex);
    ui->cbxParity->setCurrentIndex(parityIndex);
    ui->cbxStopBits->setCurrentIndex(stopBitIndex);
    ui->cbxFlowType->setCurrentIndex(flowTypeIndex);
}

void MainWindow::saveConfig()
{
    QSettings *configIniWrite = new QSettings("./conf.ini", QSettings::IniFormat);
    QString clientID = ui->editClientID->text();
    QString publishTopic = ui->editPublishTopic->text();
    QString subscribeTopic = ui->editSubscribeTopic->text();

    int portIndex = ui->cbxPort->currentIndex();
    int baudRateIndex = ui->cbxBaudRate->currentIndex();
    int dataBitIndex = ui->cbxDataBits->currentIndex();
    int parityIndex = ui->cbxParity->currentIndex();
    int stopBitIndex = ui->cbxStopBits->currentIndex();
    int flowTypeIndex = ui->cbxFlowType->currentIndex();

    configIniWrite->setValue("/Configure/ClientID", clientID);
    configIniWrite->setValue("/Configure/PublishTopic", publishTopic);
    configIniWrite->setValue("/Configure/SubscribeTopic", subscribeTopic);
//    configIniWrite->setValue("/Configure/PortIndex", portIndex);

    configIniWrite->setValue("/COM/PortIndex", portIndex);
    configIniWrite->setValue("/COM/BaudRate", baudRateIndex);
    configIniWrite->setValue("/COM/DataBit", dataBitIndex);
    configIniWrite->setValue("/COM/Parity", parityIndex);
    configIniWrite->setValue("/COM/StopBit", stopBitIndex);
    configIniWrite->setValue("/COM/FlowType", flowTypeIndex);
}

user_callback::user_callback(MainWindow *wnd) : m_wnd(wnd){}

void user_callback::connected(const string &cause)
{
     m_wnd->ui->lblMqttState->setText("Connected");
//     m_wnd->m_client->subscribe(m_wnd->m_subscribeTopic.toStdString());
}

void user_callback::connection_lost(const string &cause)
{
    //        std::cout << "\nConnection lost" << std::endl;
    //        if (!cause.empty())
    //            std::cout << "\tcause: " << cause << std::endl;
    m_wnd->ui->lblMqttState->setText("Connection lost");
}

void user_callback::message_arrived(mqtt::const_message_ptr msg)
{
    //        std::cout << "\nConnection lost" << std::endl;
    //        if (!cause.empty())
    //            std::cout << "\tcause: " << cause << std::endl;
    std::cout << msg->get_topic() << ": " << msg->get_payload_str() << std::endl;
    m_wnd->m_port->write(msg->get_payload().data(), msg->get_payload().length());
    m_wnd->m_logger->write(QString::fromStdString(msg->get_payload_str()));
}

void MainWindow::on_btnSubscribe_clicked(bool checked)
{
     m_subscribeTopic = ui->editSubscribeTopic->text();
     if (checked) {
         qDebug() << "subscribe " << m_subscribeTopic;
         m_client->subscribe(m_subscribeTopic.toStdString());
     } else {
         qDebug() << "unsubscribe " << m_subscribeTopic;
         m_client->unsubscribe(m_subscribeTopic.toStdString());
     }

}
