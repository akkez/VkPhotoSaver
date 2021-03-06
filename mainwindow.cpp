#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QRegExp>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QFileDialog>
#include <QDir>

QNetworkAccessManager* nam;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->setFixedSize(this->size());
    ui->chooseGroupBox->setVisible(false);
    ui->downloadGroupBox->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_initAuthButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://oauth.vk.com/authorize?client_id=4440951&response_type=token&scope=photos&redirect_uri=http://api.vk.com/blank.html"));
    ui->tokenEdit->setFocus();
}

void MainWindow::on_authButton_clicked()
{
    QString tokenUrl = ui->tokenEdit->text();

    QRegExp expression("access_token=([0-9a-f]+)");

    int pos = expression.indexIn(tokenUrl);
    if (pos == -1) {
        QMessageBox::critical(this, QString("Ошибка"), QString("Во введенном тексте не найден access_token"));
        return;
    } 
    this->token = expression.cap(1);

    this->getAlbums();
}

void MainWindow::getAlbums() {
    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(albumListReceivedSlot(QNetworkReply*)));

    QUrl url(QString("https://api.vk.com/method/photos.getAlbums?v=5&need_system=1&count=50&access_token=").append(this->token));
    QNetworkReply* reply = nam->get(QNetworkRequest(url));
}

void MainWindow::albumListReceivedSlot(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes);

        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject["error"].isObject()) {
            QJsonObject error = jsonObject["error"].toObject();
            QString message = error["error_msg"].toString();
            QMessageBox::critical(this, QString("Ошибка"), message);
        } else {
            QJsonObject bag = jsonObject["response"].toObject();
            QJsonArray items = bag["items"].toArray();

            QJsonObject album;
            ui->albumComboBox->clear();
            for (int i = 0; i < items.size(); i++) {
                album = items[i].toObject();
                ui->albumComboBox->addItem(album["title"].toString());
                QJsonValue albumId = album["id"];

               // qDebug()<<albumId.toVariant().toInt()<<"\n";

                ui->albumComboBox->setItemData(i, albumId.toVariant());
            }

            this->goToChooseScreen();
        }
    }
    else
    {
        QMessageBox::critical(this, QString("Ошибка"), QString("Невозможно установить соединение с сервером: ").append(reply->errorString()));
    }
    reply->manager()->deleteLater();
    reply->deleteLater();
}

void MainWindow::on_radioButton1_clicked()
{
    ui->albumComboBox->setEnabled(true);
    ui->albumEdit->setEnabled(false);
}

void MainWindow::on_radioButton2_clicked()
{
    ui->albumComboBox->setEnabled(false);
    ui->albumEdit->setEnabled(true);
}

void MainWindow::on_chooseAlbumBotton_clicked()
{
    if (ui->albumComboBox->isEnabled()) {
        this->albumId = ui->albumComboBox->itemData(ui->albumComboBox->currentIndex()).toString();
    } else {
        QRegExp expression("album(-?[0-9]+)_([0-9]+)");
        QString albumUrl = ui->albumEdit->text();

        int pos = expression.indexIn(albumUrl);
        if (pos > -1) {
            this->ownerId = expression.cap(1);
            this->albumId = expression.cap(2);
        } else {
            QMessageBox::critical(this, QString("Ошибка"), QString("Ссылка на альбом неправильная"));
            return;
        }
    }

    QString dirname = QFileDialog::getExistingDirectory(this, QString("Выберите папку для сохранения альбома"), QDir::currentPath());
    if( !dirname.isNull() && QDir(dirname).exists())
    {
        this->dirname = dirname;
    } else {
        //save folder does not exists
        return;
    }

    this->offset = 0;
    this->getPhotoList();
}

void MainWindow::getPhotoList() {
    int offset = this->offset;

    ui->statusLabel->setText(QString("Получение списка фотографий %1-%2...").arg(offset).arg(offset + 999));

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(photoListReceivedSlot(QNetworkReply*)));

    QString requestUrl(QString("https://api.vk.com/method/photos.get?v=5&owner_id=%1&album_id=%2&offset=%3&count=1000&access_token=%4")
                       .arg(this->ownerId)
                       .arg(this->albumId)
                       .arg(offset)
                       .arg(this->token));
    QUrl url(requestUrl);
    QNetworkReply* reply = nam->get(QNetworkRequest(url));
}

void MainWindow::goToDownloadScreen() {
    this->downloadScreen = true;
    this->chooseScreen = false;
    ui->downloadGroupBox->setVisible(true);
    ui->chooseGroupBox->setVisible(false);
    ui->authGroupBox->setVisible(false);
    ui->retryButton->setVisible(false);
}

void MainWindow::goToChooseScreen() {
    this->authScreen = false;
    this->chooseScreen = true;
    ui->authGroupBox->setVisible(false);
    ui->chooseGroupBox->setVisible(true);
    ui->downloadGroupBox->setVisible(false);
}

void MainWindow::photoListReceivedSlot(QNetworkReply* reply) {
    QVector<QString> results;
    this->goToDownloadScreen();

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes);
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject["error"].isObject()) {
            QJsonObject error = jsonObject["error"].toObject();
            QString message = error["error_msg"].toString();
            QMessageBox::critical(this, QString("Ошибка"), message);
            this->goToChooseScreen();
        } else {
            QJsonObject bag = jsonObject["response"].toObject();
            QJsonArray items = bag["items"].toArray();

            QJsonObject photo;
            QString photoUrl;
            results.clear();
            for (int i = 0; i < items.size(); i++) {
                photo = items[i].toObject();

                if (photo["photo_2560"].isString()) {
                    photoUrl = photo["photo_2560"].toString();
                } else if (photo["photo_1280"].isString()) {
                    photoUrl = photo["photo_1280"].toString();
                } else if (photo["photo_807"].isString()) {
                    photoUrl = photo["photo_807"].toString();
                } else if (photo["photo_604"].isString()) {
                    photoUrl = photo["photo_604"].toString();
                }

                results.push_back(photoUrl);

            //    qDebug()<<photoUrl;
            }

            if (results.size() > 0) {
                this->photoUrls += results;
                this->offset += 1000;
                this->getPhotoList();
            } else {
                this->currentPhoto = 0;
                ui->totalProgressBar->setMaximum(this->photoUrls.size());
                ui->totalProgressBar->setValue(0);
                this->downloadNextPhoto();
            }
        }
    }
    else
    {
        QMessageBox::critical(this, QString("Ошибка"), QString("Невозможно установить соединение с сервером"));
        this->goToChooseScreen();
    }

    reply->manager()->deleteLater();
    reply->deleteLater();
}

QString MainWindow::getLocalPhotoFilename(QString photoUrl) {
    QStringList urlParts = photoUrl.split("/");
    QString filename = urlParts[urlParts.size() - 1];

    return QString(this->dirname).append("/").append(filename);
}

void MainWindow::downloadNextPhoto() {
    if (this->photoUrls.size() == 0) {
        QMessageBox::warning(this, "", "Выбранный альбом пуст");
        ui->retryButton->setVisible(true);
        return;
    }

    QString photoUrl;
    while (true) {
        photoUrl = this->photoUrls[this->currentPhoto];
        QFile file(this->getLocalPhotoFilename(photoUrl));
        if (!file.exists() || this->currentPhoto == this->photoUrls.size() - 1) {
            break;
        }
        this->currentPhoto++;
    }

    ui->totalProgressBar->setValue(this->currentPhoto);
    ui->statusLabel->setText(QString("Скачивание фотографии #").append(QString::number(this->currentPhoto + 1)));
    ui->photoUrlLabel->setText(this->photoUrls[this->currentPhoto]);

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(photoDownloadFinishedSlot(QNetworkReply*)));

    QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(photoUrl)));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
             this, SLOT(photoDownloadProgressSlot(qint64,qint64)));
}

void MainWindow::photoDownloadFinishedSlot(QNetworkReply* reply) {
    bool result = false;

    QString filename = this->getLocalPhotoFilename(reply->url().toString());

    QFile file(filename);
    qDebug() << filename;
    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            result = true;
        } else {
            QMessageBox::critical(this, "Ошибка", QString("Невозможно записать файл %1 на диск").arg(filename));
        }
    } else {
        result = true;
    }

    reply->manager()->deleteLater();
    reply->deleteLater();
    if (result) {
        this->currentPhoto++;
        if (this->currentPhoto >= this->photoUrls.size()) {
            ui->totalProgressBar->setValue(this->photoUrls.size());
            QMessageBox::information(this, "", "Загрузка фотографий из альбома успешно завершена");
            QDesktopServices::openUrl(QUrl(QString("file:///").append(this->dirname), QUrl::TolerantMode));
            ui->retryButton->setVisible(true);
            return;
        } else {
            this->downloadNextPhoto();
        }
    }
}

void MainWindow::photoDownloadProgressSlot(qint64 received, qint64 total) {
    ui->progressBar->setFormat(QString("%1 KB / %2 KB").arg((int)(received / 1024)).arg((int)(total / 1024)));
    ui->progressBar->setValue((int)(received * 100 / total));
//    qDebug() << "received " << received << ", total "<< total;
}

void MainWindow::on_retryButton_clicked()
{
    this->photoUrls.clear();
    this->goToChooseScreen();
}
