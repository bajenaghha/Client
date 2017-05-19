#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAbstractSocket>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    client = new Client(this);
    connect(client,&Client::connected,this,[&](){
        this->ui->textEdit->append("Connected to server.\n");
        client->makeAuth(ui->username->text());
    },Qt::QueuedConnection);
    connect(client,&Client::disconnected,this,[&](){
        this->ui->textEdit->append("Disconnected to server.\n");
    },Qt::QueuedConnection);
    connect(client,&Client::error,this,[&](QString error){
        this->ui->textEdit->append("Error: "+error+"\n");
    },Qt::QueuedConnection);
    connect(client,&Client::disjoint,this,[&](QString name){
        this->ui->textEdit->append("Disjoint from: " + name+"\n");
    },Qt::QueuedConnection);
    connect(client,&Client::joint,this,[&](QString name){
        this->ui->textEdit->append("Joint to: " + name+"\n");
    },Qt::QueuedConnection);
    connect(client,&Client::startSendData,this,[&](){
        this->ui->textEdit->append("Start sending data...\n");
    },Qt::QueuedConnection);
    connect(client,&Client::madeAuth,this,[&](){
        this->ui->textEdit->append("Auth finished.\n");
    },Qt::QueuedConnection);
    connect(client,&Client::allUsers,this,[&](QStringList items){
        this->ui->listWidget->clear();
        this->ui->listWidget->addItems(items);
    },Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnconnect_clicked()
{
    client->connectToServer(ui->host->text());
}

void MainWindow::on_btnjoin_clicked()
{
    auto list = ui->listWidget->selectedItems();
    if (!list.isEmpty())
        client->joinToUser(ui->listWidget->selectedItems().first()->text());
    else
        this->ui->textEdit->append("Empty selection!\n");
}

void MainWindow::on_btndisconnect_clicked()
{
    client->disconnectFromServer();
}
