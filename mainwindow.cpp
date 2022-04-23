#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <sane/sane.h>
#include <qstring.h>
#include <shell.h>
#include <cscanfront.h>
#include <string>
#include <cscanner.h>
#include <QThread>

std::vector<CScanner*> devices;
CScanner* activeDevice;
int selectedDeviceIndex = -1;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {  delete ui; }


void MainWindow::on_pushButton_clicked(){
    if (ui->comboBox->count() == 0) return;

    activeDevice->setSelectedColor(ui->comboBox_2->currentIndex());
    activeDevice->setSelectedResolution(ui->comboBox_3->currentIndex());
    activeDevice->setSelectedScanArea(ui->comboBox_4->currentIndex());
    ui->label_6->setText("Сканирование начато");

    try {
        QImage image = CScanFront::scanImage(activeDevice);
        image.save(ui->lineEdit->text() + ui->label_5->text());
        ui->label_6->setText("Сканирование завершено");
    } catch(std::exception e) {
        ui->label_6->setText("Ошибка сканирования");
    }
}


void MainWindow::on_pushButton_3_clicked() {
    devices = CScanFront::getDevices();

    if (devices.size() > 0) {
        ui->comboBox->clear();

        for (auto scanner : devices) {
            ui->comboBox->addItem(scanner->getModel());
        }

        selectDevice(ui->comboBox->currentIndex());

        ui->pushButton->setEnabled(true);
    }
}

void MainWindow::selectDevice(int deviceIndex) {
    selectedDeviceIndex = deviceIndex;
    activeDevice = devices[selectedDeviceIndex];

    ui->comboBox_2->clear();
    ui->comboBox_3->clear();
    ui->comboBox_4->clear();

    ui->comboBox_2->addItems(activeDevice->getSupportedColorModes());
    ui->comboBox_3->addItems(activeDevice->getSupportedResolutions());
    ui->comboBox_4->addItems(activeDevice->getSupportedScanAreas());

    ui->comboBox_2->setCurrentIndex(activeDevice->getDefaultColorIndex());
    ui->comboBox_3->setCurrentIndex(activeDevice->getDefaultResolutionIndex());
    ui->comboBox_4->setCurrentIndex(activeDevice->getDefaultScanAreaIndex());
    ui->label_5->setText(activeDevice->getExtension());
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    selectDevice(ui->comboBox->currentIndex());
}

