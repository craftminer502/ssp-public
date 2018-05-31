#include <QCoreApplication>
#include <QApplication>
#include <iostream>
#include <QSerialPort>
#include <QDebug>
#include <Windows.h>
#include <QElapsedTimer>
#include <QPushButton>
#include <QTimer>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

using namespace std;

QSerialPort serial;

//From internet
template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
class Sensor {
public:
    Sensor() {
        qDebug()<<QString::fromStdString("Sensor created!");
    }
    enum Type {
        TEMPERATURE, HUMIDTY, MOISTURE
    };

    int getData() {
        return this->data;
    }

    void setData(int data) {
        this->data = data;
    }

    Type getType() {
        return this->type;
    }

    void setType(Type type) {
        this->type = type;
    }
private:
    Type type;
    int data;
};

class SensorTemperature : public Sensor {
public:
    SensorTemperature();
};
class SensorHumidity : public Sensor {
public:
   SensorHumidity();
};

class SensorMoisture : public Sensor {
public:
    SensorMoisture();
    int getIncr();
};

class Greenhouse {
public:
    Greenhouse* next = this;
    SensorTemperature temp;
    SensorHumidity humi;
    SensorMoisture mois;
    void add(Greenhouse* gHouse);
    Greenhouse* getNewest();
    QString getGreenhouseData();
};

void Greenhouse::add(Greenhouse* gHouse) {
    if(this->next == NULL) {
        this->next = gHouse;
    } else {
        this->next->add(gHouse);
    }
}

Greenhouse* Greenhouse::getNewest() {
    if(this->next == NULL) {
        return this;
    } else {
        return this->next->getNewest();
    }
}

SensorTemperature::SensorTemperature() {
    this->setType(Sensor::Type::TEMPERATURE);
    this->setData(0);
}

SensorHumidity::SensorHumidity() {
     this->setType(Sensor::Type::HUMIDTY);
     this->setData(0);
 }

SensorMoisture::SensorMoisture() {
    this->setType(Sensor::Type::MOISTURE);
    this->setData(50);
}

int SensorMoisture::getIncr() {
    srand(time(0));
    return this->getData()+rand()%7-3;
}

QString Greenhouse::getGreenhouseData() {
    return QString::fromStdString("Temperature: "+ std::to_string(this->temp.getData()) +", Humidity: "+ std::to_string(this->humi.getData()) +", Moisture: "+ std::to_string(this->mois.getData()));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Greenhouse* gHouse = new Greenhouse();
    //qDebug()<<gHouse->getGreenhouseData();
    //initGUI();
    serial.setPortName("COM3");
    serial.open(QIODevice::ReadWrite);
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    while(!serial.isOpen()) {
        serial.open(QIODevice::ReadWrite);
    }

    if (serial.isOpen() && serial.isReadable()) {
        QByteArray input;

        while(true) {
            Sleep(50);

            serial.waitForReadyRead(6);
            if(serial.bytesAvailable()>=6) {
                input = serial.readAll();
                if(input != NULL || input != "\x00") {
                    if(input.length() == 6) {
                        std::vector<std::string> x = split(input.toStdString(), ':'); //separate
                        int temp, humi;
                        istringstream(x[1]) >> temp;
                        istringstream(x[2]) >> humi;
                        if(gHouse->next != NULL) {
                            gHouse->temp.setData(temp); //pass
                            gHouse->humi.setData(humi); //pass
                            gHouse->mois.setData(gHouse->mois.getIncr());
                            gHouse->next = NULL;
                        } else {
                            Greenhouse* tempo = new Greenhouse;
                            tempo->temp.setData(temp); //pass
                            tempo->humi.setData(humi); //pass
                            tempo->mois.setData(gHouse->mois.getIncr());
                            tempo->next = NULL;
                            gHouse->add(tempo);
                            qDebug()<<gHouse->getNewest()->getGreenhouseData();
                        }
                    }
                }
            }
        }
    }
    return app.exec();
}

