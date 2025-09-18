#include "cscanner.h"
#include <QDebug>

//------------------- DUMMY SCANNER ----------------

CDummyScanner::CDummyScanner(QString device, QString vendor, QString model){
    this->device = device;
    this->vendor = vendor;
    this->model = model;

    this->colors = QStringList{"Color"};
    this->dpis = QStringList{"100"};
    this->pageFormats = QStringList{"Maximum Area"};
    this->scanAreas = QStringList{"-x 215.88 -y 296.973"};
    this->formats = QStringList{"jpeg"};
    this->extensions = QStringList{".jpg"};
    this->defaultColorIndex = 0;
    this->defaultResolutionIndex = 0;
    this->defaultScanAreaIndex = 0;
    this->extension = this->extensions[this->defaultColorIndex];
    
    // Инициализируем значения по умолчанию
    this->color = this->colors[this->defaultColorIndex];
    this->format = this->formats[this->defaultColorIndex];
    this->resolution = this->dpis[this->defaultResolutionIndex];
    this->pageFormat = this->pageFormats[this->defaultScanAreaIndex];
    this->scanArea = this->scanAreas[this->defaultScanAreaIndex];
    
    // Инициализируем пользовательскую область
    this->useCustomArea = false;
    this->customArea = QRect();
}

QString CDummyScanner::getDevice() {
    return this->device;
}

QString CDummyScanner::getVendor() {
    return this->vendor;
}

QString CDummyScanner::getModel() {
    return this->model;
}

int CDummyScanner::getDefaultColorIndex() {
    return this->defaultColorIndex;
}

QRect CDummyScanner::getCropRect() {
    return this->cropRect;
}

int CDummyScanner::getDefaultResolutionIndex() {
    return this->defaultResolutionIndex;
}

int CDummyScanner::getDefaultScanAreaIndex() {
    return this->defaultScanAreaIndex;
}

QStringList CDummyScanner::getSupportedColorModes() {
    return this->colors;
}

QString CDummyScanner::getExtension() {
    return this->extension;
}

QStringList CDummyScanner::getSupportedResolutions() {
    QStringList resolutions;
    for (const QString &res : this->dpis) {
        resolutions.push_back({res + " DPI"});
    }
    return resolutions;
}

QStringList CDummyScanner::getSupportedScanAreas() {
    return this->pageFormats;
}

bool CDummyScanner::getCropNeeded() {
    return this->cropNeeded;
}

void CDummyScanner::setSelectedColor(int index) {
    this->color = this->colors[index];
    this->format = this->formats[index];
    this->extension = this->extensions[index];
}

void CDummyScanner::setSelectedResolution(int index) {
    this->resolution = this->dpis[index];
}

void CDummyScanner::setSelectedScanArea(int index) {
    this->pageFormat = this->pageFormats[index];
    this->scanArea = this->scanAreas[index];
}

QList<QString> CDummyScanner::getArgs() {
    QList<QString> args;

    this->cropNeeded = false;
    
    // Добавляем базовые аргументы для dummy сканера
    args = {"-d", this->getDevice()};
    args.append("--mode=" + this->color);
    args.append("--resolution=" + this->resolution);
    args.append("--format=" + this->format);

    // Добавляем область сканирования
    if (this->useCustomArea && !this->customArea.isEmpty()) {
        // Используем пользовательскую область (в миллиметрах)
        // Конвертируем из пикселей в миллиметры (примерно 25.4 мм на дюйм)
        qreal dpi = this->resolution.toDouble();
        qreal mmPerPixel = 25.4 / dpi;
        
        int x = static_cast<int>(this->customArea.x() * mmPerPixel);
        int y = static_cast<int>(this->customArea.y() * mmPerPixel);
        int width = static_cast<int>(this->customArea.width() * mmPerPixel);
        int height = static_cast<int>(this->customArea.height() * mmPerPixel);
        
        qDebug() << "CDummyScanner: Custom area - Original:" << this->customArea;
        qDebug() << "CDummyScanner: DPI:" << dpi << "mmPerPixel:" << mmPerPixel;
        qDebug() << "CDummyScanner: Converted - x:" << x << "y:" << y << "w:" << width << "h:" << height;
        
        args.append("-l");
        args.append(QString::number(x));
        args.append("-t");
        args.append(QString::number(y));
        args.append("-x");
        args.append(QString::number(width));
        args.append("-y");
        args.append(QString::number(height));
        
        qDebug() << "CDummyScanner: Final args:" << args;
    } else if (!(this->pageFormat.startsWith("Maximum"))) {
        args += this->scanArea.split(" ");
    }

    return args;
}

// -------------------- HP SCANNER -----------------------

CHPScanner::CHPScanner(QString device, QString vendor, QString model) {
    this->device = device;
    this->vendor = vendor;
    this->model = model;

    this->colors = QStringList{"Color", "Gray", "Lineart"};
    this->dpis = QStringList{"100", "200", "300", "600", "1200"};
    this->pageFormats = QStringList{"Maximum Area", "A4 (210x297mm)", "Letter (8.5x11 in)", "4R (4x6in)"};
    this->scanAreas = QStringList{"-x 215.9 -y 297.01", "-x 210 -y 297",
                       "-x 215.9 -y 279.4", "-x 152.4 -y 101.6"};
    this->formats = QStringList{"jpeg", "jpeg", "tiff"};
    this->extensions = QStringList{".jpg", ".jpg", ".tiff"};
    this->defaultColorIndex = 0;
    this->defaultResolutionIndex = 2;
    this->defaultScanAreaIndex = 1;
    this->extension = this->extensions[this->defaultColorIndex];
    
    // Инициализируем значения по умолчанию
    this->color = this->colors[this->defaultColorIndex];
    this->format = this->formats[this->defaultColorIndex];
    this->resolution = this->dpis[this->defaultResolutionIndex];
    this->pageFormat = this->pageFormats[this->defaultScanAreaIndex];
    this->scanArea = this->scanAreas[this->defaultScanAreaIndex];
    
    // Инициализируем пользовательскую область
    this->useCustomArea = false;
    this->customArea = QRect();
}

QString CHPScanner::getDevice() {
    return this->device;
}
QString CHPScanner::getVendor() {
    return this->vendor;
}
QString CHPScanner::getModel() {
    return this->model;
}
int CHPScanner::getDefaultColorIndex() {
    return this->defaultColorIndex;
}
bool CHPScanner::getCropNeeded() {
    return this->cropNeeded;
}

QRect CHPScanner::getCropRect() {
    return this->cropRect;
}

int CHPScanner::getDefaultResolutionIndex() {
    return this->defaultResolutionIndex;
}

int CHPScanner::getDefaultScanAreaIndex() {
    return this->defaultScanAreaIndex;
}

QStringList CHPScanner::getSupportedColorModes() {
    return this->colors;
}

QString CHPScanner::getExtension() {
    return this->extension;
}

QStringList CHPScanner::getSupportedResolutions() {
    QStringList resolutions;
    for (const QString &res : this->dpis) {
        resolutions.push_back({res + " DPI"});
    }
    return resolutions;
}

QStringList CHPScanner::getSupportedScanAreas() {
    return this->pageFormats;
}

void CHPScanner::setSelectedColor(int index) {
    this->color = this->colors[index];
    this->format = this->formats[index];
    this->extension = this->extensions[index];
}

void CHPScanner::setSelectedResolution(int index) {
    this->resolution = this->dpis[index];
}
void CHPScanner::setSelectedScanArea(int index) {
    this->pageFormat = this->pageFormats[index];
    this->scanArea = this->scanAreas[index];
}

QList<QString> CHPScanner::getArgs() {
    QList<QString> args;
    this->cropNeeded = false;
    args = {"-d", this->getDevice()};
    args.append("--mode=" + this->color);
    args.append("--resolution=" + this->resolution);
    args.append("--format="+this->format);

    // Добавляем область сканирования
    if (this->useCustomArea && !this->customArea.isEmpty()) {
        // Используем пользовательскую область (в миллиметрах)
        qreal dpi = this->resolution.toDouble();
        qreal mmPerPixel = 25.4 / dpi;
        
        int x = static_cast<int>(this->customArea.x() * mmPerPixel);
        int y = static_cast<int>(this->customArea.y() * mmPerPixel);
        int width = static_cast<int>(this->customArea.width() * mmPerPixel);
        int height = static_cast<int>(this->customArea.height() * mmPerPixel);
        
        args.append("-l");
        args.append(QString::number(x));
        args.append("-t");
        args.append(QString::number(y));
        args.append("-x");
        args.append(QString::number(width));
        args.append("-y");
        args.append(QString::number(height));
    } else if (this->pageFormat.startsWith("A4") && this->resolution=="300") {
        this->cropNeeded = true;
        this->cropRect = QRect(8,0,2488,3500); // 2480x3508
        return args;
    } else if (!(this->pageFormat.startsWith("Maximum"))) {
        args += this->scanArea.split(" ");
    }
    return args;
}

// ------------------------- BROTHER SCANNER -------------------------

CBrotherScanner::CBrotherScanner(QString device, QString vendor, QString model) {
    this->device = device;
    this->vendor = vendor;
    this->model = model;

    this->colors = QStringList{"24bit Color", "True Gray", "Black & White", "Gray[Error Diffusion]"};
    this->dpis = QStringList{"100", "200", "300", "400", "600"};
    this->pageFormats = QStringList{"Maximum Area", "A4 (210x297mm)", "Letter (8.5x11 in)", "4R (4x6in)"};
    this->scanAreas = QStringList{"-x 215.9 -y 297.01", "-x 210 -y 297",
                       "-x 215.9 -y 279.4", "-x 152.4 -y 101.6"};
    this->formats = QStringList{"jpeg", "jpeg", "tiff", "tiff"};
    this->extensions = QStringList{".jpg", ".jpg", ".tiff", ".tiff"};
    this->defaultColorIndex = 0;
    this->defaultResolutionIndex = 3;
    this->defaultScanAreaIndex = 1;
    this->extension = this->extensions[this->defaultColorIndex];
    
    // Инициализируем значения по умолчанию
    this->color = this->colors[this->defaultColorIndex];
    this->format = this->formats[this->defaultColorIndex];
    this->resolution = this->dpis[this->defaultResolutionIndex];
    this->pageFormat = this->pageFormats[this->defaultScanAreaIndex];
    this->scanArea = this->scanAreas[this->defaultScanAreaIndex];
    
    // Инициализируем пользовательскую область
    this->useCustomArea = false;
    this->customArea = QRect();
}
QString CBrotherScanner::getDevice() {
    return this->device;
}
QString CBrotherScanner::getVendor() {
    return this->vendor;
}
QString CBrotherScanner::getModel() {
    return this->model;
}
bool CBrotherScanner::getCropNeeded() {
    return this->cropNeeded;
}

QRect CBrotherScanner::getCropRect() {
    return this->cropRect;
}


QString CBrotherScanner::getExtension() {
    return this->extension;
}

int CBrotherScanner::getDefaultColorIndex() {
    return this->defaultColorIndex;
}

int CBrotherScanner::getDefaultResolutionIndex() {
    return this->defaultResolutionIndex;
}

int CBrotherScanner::getDefaultScanAreaIndex() {
    return this->defaultScanAreaIndex;
}

QStringList CBrotherScanner::getSupportedColorModes() {
    return this->colors;
}

QStringList CBrotherScanner::getSupportedResolutions() {
    QStringList resolutions;
    for (const QString &res : this->dpis) {
        resolutions.push_back({res + " DPI"});
    }
    return resolutions;
}

QStringList CBrotherScanner::getSupportedScanAreas() {
    return this->pageFormats;
}

void CBrotherScanner::setSelectedColor(int index) {
    this->color = this->colors[index];
    this->format = this->formats[index];
    this->extension = this->extensions[index];
}

void CBrotherScanner::setSelectedResolution(int index) {
    this->resolution = this->dpis[index];
}

void CBrotherScanner::setSelectedScanArea(int index) {
    this->pageFormat = this->pageFormats[index];
    this->scanArea = this->scanAreas[index];
}

QList<QString> CBrotherScanner::getArgs() {
    QList<QString> args;
    this->cropNeeded = false;
    args = {"-d", this->device};
    args.append("--mode=" + this->color);
    args.append("--resolution=" + this->resolution);
    args.append("--format="+this->format);

    // Добавляем область сканирования
    if (this->useCustomArea && !this->customArea.isEmpty()) {
        // Используем пользовательскую область (в миллиметрах)
        qreal dpi = this->resolution.toDouble();
        qreal mmPerPixel = 25.4 / dpi;
        
        int x = static_cast<int>(this->customArea.x() * mmPerPixel);
        int y = static_cast<int>(this->customArea.y() * mmPerPixel);
        int width = static_cast<int>(this->customArea.width() * mmPerPixel);
        int height = static_cast<int>(this->customArea.height() * mmPerPixel);
        
        args.append("-l");
        args.append(QString::number(x));
        args.append("-t");
        args.append(QString::number(y));
        args.append("-x");
        args.append(QString::number(width));
        args.append("-y");
        args.append(QString::number(height));
    } else if (!(this->pageFormat.startsWith("Maximum"))) {
        args += this->scanArea.split(" ");
    }
    return args;
}

// Реализация setCustomScanArea для всех сканеров
void CDummyScanner::setCustomScanArea(const QRect& area) {
    this->useCustomArea = true;
    this->customArea = area;
}

void CDummyScanner::resetCustomScanArea() {
    this->useCustomArea = false;
    this->customArea = QRect();
}

void CHPScanner::setCustomScanArea(const QRect& area) {
    this->useCustomArea = true;
    this->customArea = area;
}

void CHPScanner::resetCustomScanArea() {
    this->useCustomArea = false;
    this->customArea = QRect();
}

void CBrotherScanner::setCustomScanArea(const QRect& area) {
    this->useCustomArea = true;
    this->customArea = area;
}

void CBrotherScanner::resetCustomScanArea() {
    this->useCustomArea = false;
    this->customArea = QRect();
}
