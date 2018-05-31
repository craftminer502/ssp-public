#include <QApplication>
#include <iostream>
#include <QSerialPort>
#include <QDebug>
#include <Windows.h>
#include <thread>
#include <string>
#include <sstream>

using namespace std;

QSerialPort serial;

//The below split method is taken from
//https://gist.github.com/KPB3rd/fbe2b0474ae8a777cc1a893a3822347a
//All credit goes to the original creator assumed to be "KPB3rd"
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

//Sensor class
class Sensor {
public: //Access is public for these methods and fields
    Sensor() {
        //qDebug()<<QString::fromStdString("Sensor created!"); //Debug to see if sensor objects are create.
    }
    enum Type { //Enumeration of the types
        TEMPERATURE, HUMIDTY, MOISTURE, WEED
    };

    int getData() { //Getter
        return this->data;
    }

    void setData(int data) { //Setter
        this->data = data;
    }

    Type getType() {
        return this->type;
    }

    void setType(Type type) {
        this->type = type;
    }
private: //Access is private
    Type type;
    int data;
};

class SensorTemperature : public Sensor { //Inherits from Sensor with public access
public:
    SensorTemperature(); //Prototyped contructor
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

class SensorWeed : public Sensor {
public:
    SensorWeed();
    int getIncr();
};

class Greenhouse { //Protyope of the greenhouse which also functions as linkedlist
public:
    Greenhouse();
    Greenhouse* next = NULL;
    SensorTemperature* temp;
    SensorHumidity* humi;
    SensorMoisture* mois;
    SensorWeed* weed;
    void add(Greenhouse* gHouse);
    Greenhouse* getNewest();
    QString getGreenhouseData();
};

Greenhouse::Greenhouse() {//Implementation
    this->temp = new SensorTemperature();
    this->humi = new SensorHumidity();
    this->mois = new SensorMoisture();
    this->weed = new SensorWeed();
}

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

SensorWeed::SensorWeed() {
    this->setType(Sensor::Type::WEED);
    this->setData(0);
}

int SensorWeed::getIncr() {
    srand(time(0));
    return this->getData()+rand()%4;
}

QString Greenhouse::getGreenhouseData() {
    return QString::fromStdString("Temperature: "+ std::to_string(this->temp->getData()) +", Humidity: "+ std::to_string(this->humi->getData()) +", Moisture: "+ std::to_string(this->mois->getData()) +", Weed: "+ std::to_string(this->weed->getData()));
}

int main(int argc, char *argv[])//Main method - called ón start.
{
    QApplication app(argc, argv);//passes command arguments
    Greenhouse* gHouse = new Greenhouse();//Greenhouse instance is initalized
    //qDebug()<<gHouse->getGreenhouseData(); //Debug
    //initGUI(); //GUI to be made later
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

        int counter = 0;
        while(true) {
            Sleep(50);

            serial.waitForReadyRead(6);
            if(serial.bytesAvailable()>=6) {
                input = serial.readAll();
                if(input != NULL || input != "\x00") {
                    if(input.length() == 6) {
                        std::vector<std::string> x = split(input.toStdString(), ':'); //Uses the method for splitting strings by delimiter.
                        int temp, humi;
                        istringstream(x[1]) >> temp; //Converts string to int.
                        istringstream(x[2]) >> humi;
                        int oldWeed;
                        if(counter < 1) {
                            gHouse->temp->setData(temp); //pass
                            gHouse->humi->setData(humi); //pass
                            gHouse->mois->setData(gHouse->mois->getIncr());
                            gHouse->weed->setData(gHouse->weed->getIncr());
                            gHouse->next = NULL; //sets to null for linkedlist purposes
                            oldWeed = gHouse->weed->getData();
                        } else {
                            Greenhouse* tempo = new Greenhouse;
                            tempo->temp->setData(temp); //pass
                            tempo->humi->setData(humi); //pass
                            tempo->mois->setData(gHouse->mois->getIncr());
                            tempo->weed->setData(gHouse->weed->getIncr()+oldWeed);
                            tempo->next = NULL;
                            gHouse->add(tempo);
                            oldWeed = tempo->weed->getData();
                        }
                        qDebug()<<gHouse->getNewest()->getGreenhouseData(); //Gets newest link and prints
                        if(gHouse->getNewest()->weed->getData() > 10) {
                            qDebug()<<QString::fromStdString("You currently have "+ std::to_string(gHouse->getNewest()->weed->getData()) +" weeds growing.");
                            qDebug()<<QString::fromStdString("It is way too many so you have to weed out otherwise your tomatoes will not survive ;(");
                        }
                        if (gHouse->getNewest()->weed->getData() > 30) {
                            oldWeed = 0;
                            qDebug()<<QString::fromStdString("I took care of it this time but you must do a better job as a tomatokeeper next time.");
                        }
                            counter++;
                    }
                }
            }
        }
    }
    return app.exec();
}

