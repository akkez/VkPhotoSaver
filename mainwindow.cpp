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

QNetworkAccessManager* nam;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
             this, SLOT(finishedGetAlbumsSlot(QNetworkReply*)));

    QUrl url(QString("https://api.vk.com/method/photos.getAlbums?v=5&need_system=1&count=50&access_token=").append(this->token));
    QNetworkReply* reply = nam->get(QNetworkRequest(url));
}

void MainWindow::finishedGetAlbumsSlot(QNetworkReply* reply)
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

            this->authScreen = false;
            this->chooseScreen = true;
            ui->authGroupBox->setVisible(false);
            ui->chooseGroupBox->setVisible(true);
        }
    }
    else
    {
        QMessageBox::critical(this, QString("Ошибка"), QString("Невозможно установить соединение с сервером"));
    }
    delete reply;
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
        QString albumId = ui->albumComboBox->itemData(ui->albumComboBox->currentIndex()).toString(); //ui->albumComboBox->itemData().toString());
        QMessageBox::information(this, QString(), QString("my album: ").append(albumId));
    } else {
        QMessageBox::information(this, QString(), QString("album url: ").append(ui->albumEdit->text()));
    }
}
