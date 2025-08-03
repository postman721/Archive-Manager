#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QDragEnterEvent>
#include <QDropEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_openButton_clicked();        // Load archive and list contents
    void on_extractButton_clicked();     // Extract selected items
    void on_extractAllButton_clicked();  // Extract entire archive
    void on_createButton_clicked();      // Create archive from dropped items

private:
    Ui::MainWindow *ui { nullptr };
    QString currentArchivePath;  // Set after opening an archive
    QStringList droppedPaths;    // Files/folders dragged from the file manager
};

#endif // MAINWINDOW_H
