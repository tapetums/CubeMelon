#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class IPlugin;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void setOwner(IPlugin* owner);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    struct Impl;
    Impl* pimpl;
};

#endif // MAINWINDOW_H
