#include "cscanfront.h"

#include <QProcess>
#include "cscanner.h"
#include <qstring.h>
#include <QImage>
#include <QDebug>
#include <memory>



CScanFront::CScanFront() {

}

std::vector<CScanner*> CScanFront::getDevices() {
    std::vector<CScanner*> devices;
    QProcess proc;

    // Объекты фабрик с автоматическим управлением памятью
    std::unique_ptr<CBrotherScannerFactory> brotherFactory = std::make_unique<CBrotherScannerFactory>();
    std::unique_ptr<CDummyScannerFactory> dummyFactory = std::make_unique<CDummyScannerFactory>();
    std::unique_ptr<CHPScannerFactory> hpFactory = std::make_unique<CHPScannerFactory>();

    // Запускаем поиск сканеров
    proc.start("scanimage", {"-f", "%d=>%v=>%m%n"});

    // Если поиск затянулся
    if (!proc.waitForFinished()) {
        return devices;
    }

    // Читаем инфу из стандартного вывода команды
    QString data = QString::fromUtf8(proc.readAllStandardOutput().constData());

    if (data.isEmpty()) {
        return devices;
    }

    QStringList lines = data.split("\n");

    for (const QString &line : lines) {
        if (line.isEmpty()) continue;
        QStringList elements = line.split("=>");
        if (elements.size() < 3) continue; // Проверка на достаточное количество элементов
        
        if (elements[1] == "Hewlett-Packard") {
            devices.push_back(hpFactory->createScanner(elements[0], elements[1], elements[2]));
        } else if (elements[1] == "Brother") {
            devices.push_back(brotherFactory->createScanner(elements[0], elements[1], elements[2]));
        } else {
            devices.push_back(dummyFactory->createScanner(elements[0], elements[1], elements[2]));
        }
    }

    return devices;
}

QImage CScanFront::scanImage(CScanner* scanner) {
    QProcess proc;
    QImage image;

    QList args = scanner->getArgs();
    
    qDebug() << "CScanFront: Executing scanimage with args:" << args;

    proc.start("scanimage", args);
    if ((!proc.waitForFinished(-1))||(proc.exitCode())) {
        QString error = QString::fromUtf8(proc.readAllStandardError().constData());
        qDebug() << "CScanFront: scanimage error:" << error;
        throw std::runtime_error("Error:" + error.toStdString());
    }
    QByteArray data = proc.readAllStandardOutput();
    image = QImage::fromData(data);
    
    qDebug() << "CScanFront: Scanned image size:" << image.size();

    if (scanner->getCropNeeded()) {
        image = image.copy(scanner->getCropRect());
    }

    data.clear();
    args.clear();

    return image;
}
