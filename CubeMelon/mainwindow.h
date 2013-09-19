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
    struct ICompAdapter;
}

//---------------------------------------------------------------------------//

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public:
    void addConsoleText(const wchar_t* text);
    void addListItem(CubeMelon::ICompAdapter* ca);
    void removeListItem();
    void clearList();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MainWindow* ui;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

#endif // MAINWINDOW_H
