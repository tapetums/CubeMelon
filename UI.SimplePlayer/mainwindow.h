#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//---------------------------------------------------------------------------//

namespace Ui
{
    class MainWindow;
}

namespace CubeMelon
{
    struct IComponent;
}

//---------------------------------------------------------------------------//

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void setOwner(CubeMelon::IComponent* owner);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

#endif // MAINWINDOW_H
