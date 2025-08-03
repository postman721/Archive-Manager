#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QMimeData>
#include <QUrl>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    // App‑wide font and dark style
    QFont font("Segoe UI", 10);
    qApp->setFont(font);

    QString style = R"(
    QWidget { background-color: #2e2e2e; color: #f0f0f0; }
    QPushButton { background-color: #444; border: none; border-radius: 5px; }
    QPushButton:hover { background-color: #666; }
    QListWidget { border: 1px solid #555; background-color: #3c3c3c; padding: 4px; }
    QStatusBar { background-color: #2e2e2e; border-top: 1px solid #444; }
    QScrollBar:vertical { background: #2e2e2e; width: 8px; }
)";
    qApp->setStyleSheet(style);

    // Allow Ctrl / Shift multi‑selection of files inside the archive
    ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Disable extract buttons until an archive is opened
    ui->extractButton->setEnabled(false);
    ui->extractAllButton->setEnabled(false);
}

MainWindow::~MainWindow() { delete ui; }

//--------------------------------------------------------------
//  Drag & Drop support for creating archives
//--------------------------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    droppedPaths.clear();
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (!path.isEmpty())
            droppedPaths << path;
    }

    ui->listWidget->clear();
    ui->listWidget->addItems(droppedPaths);
    ui->statusBar->showMessage(tr("Ready to create archive."));
}

//--------------------------------------------------------------
//  OPEN ARCHIVE  → list contents only
//--------------------------------------------------------------
void MainWindow::on_openButton_clicked()
{
    QString archivePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Archive"),
        QDir::homePath(),
        tr("Archive Files (*.zip *.tar *.tar.gz *.tar.bz2 *.tar.xz);;Zip (*.zip);;Tar (*.tar *.tar.gz *.tar.bz2 *.tar.xz)"));

    if (archivePath.isEmpty())
        return;

    // Determine type and list contents
    QString program;
    QStringList listArgs;
    bool isZip = archivePath.endsWith(".zip", Qt::CaseInsensitive);

    if (isZip) {
        program = "unzip";
        listArgs << "-l" << archivePath;
    } else {
        program = "tar";
        if (archivePath.endsWith(".tar.gz", Qt::CaseInsensitive))
            listArgs << "-tzf" << archivePath;
        else if (archivePath.endsWith(".tar.bz2", Qt::CaseInsensitive))
            listArgs << "-tjf" << archivePath;
        else if (archivePath.endsWith(".tar.xz", Qt::CaseInsensitive))
            listArgs << "-tJf" << archivePath;
        else
            listArgs << "-tf" << archivePath;
    }

    QProcess listProc;
    listProc.setProcessChannelMode(QProcess::MergedChannels);
    listProc.start(program, listArgs);
    listProc.waitForFinished();

    QString output = listProc.readAll().trimmed();
    QStringList fileList;

    if (isZip) {
        // Parse unzip -l output: skip header/footer lines
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        int start = 3;
        int end = lines.size() - 2;
        for (int i = start; i < end; ++i) {
            QString line = lines.at(i).trimmed();
            QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                fileList << parts.last();
            }
        }
    } else {
        fileList = output.split('\n', Qt::SkipEmptyParts);
    }

    if (listProc.exitCode() != 0 || fileList.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to read archive:\n%1").arg(output));
        return;
    }

    ui->listWidget->clear();
    ui->listWidget->addItems(fileList);
    ui->statusBar->showMessage(tr("Archive loaded: %1").arg(QFileInfo(archivePath).fileName()));

    currentArchivePath = archivePath;
    ui->extractButton->setEnabled(true);
    ui->extractAllButton->setEnabled(true);
}

//--------------------------------------------------------------
//  HELPER  – perform extraction; items empty ⇒ extract all
//--------------------------------------------------------------
static bool runExtract(const QString &archivePath,
                       const QString &outputDir,
                       const QStringList &items,
                       QString &errorOut)
{
    bool isZip = archivePath.endsWith(".zip", Qt::CaseInsensitive);
    QString program = isZip ? "unzip" : "tar";
    QStringList args;

    if (isZip) {
        args << "-o" << archivePath;
        if (!items.isEmpty())
            args.append(items);
        args << "-d" << outputDir;
    } else {
        if (archivePath.endsWith(".tar.gz", Qt::CaseInsensitive))
            args << "-xzf" << archivePath;
        else if (archivePath.endsWith(".tar.bz2", Qt::CaseInsensitive))
            args << "-xjf" << archivePath;
        else if (archivePath.endsWith(".tar.xz", Qt::CaseInsensitive))
            args << "-xJf" << archivePath;
        else
            args << "-xf" << archivePath;

        args << "-C" << outputDir;
        if (!items.isEmpty())
            args.append(items);
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(program, args);
    proc.waitForFinished();

    errorOut = proc.readAll();
    return proc.exitCode() == 0;
}

//--------------------------------------------------------------
//  EXTRACT SELECTED
//--------------------------------------------------------------
void MainWindow::on_extractButton_clicked()
{
    if (currentArchivePath.isEmpty()) {
        QMessageBox::warning(this, tr("No Archive"), tr("Open an archive first."));
        return;
    }

    QStringList selectedItems;
    for (QListWidgetItem *item : ui->listWidget->selectedItems())
        selectedItems << item->text();

    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, tr("Nothing Selected"), tr("Select one or more items (or use Extract All)."));
        return;
    }

    QString outputDir = QFileDialog::getExistingDirectory(this, tr("Select Extraction Directory"), QDir::homePath());
    if (outputDir.isEmpty())
        return;

    // Warn if any extracted item already exists
    for (const QString &entry : selectedItems) {
        QFileInfo target(outputDir + QDir::separator() + entry);
        if (target.exists()) {
            auto reply = QMessageBox::warning(
                this,
                tr("Overwrite Exists"),
                tr("'%1' already exists in '%2'. Overwrite?" ).arg(entry).arg(outputDir),
                QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
            break; // only prompt once
        }
    }

    QString err;
    if (runExtract(currentArchivePath, outputDir, selectedItems, err)) {
        QMessageBox::information(this, tr("Success"), tr("Extracted %1 item(s) to:\n%2").arg(selectedItems.size()).arg(outputDir));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Extraction failed:\n\n%1").arg(err));
    }
}

//--------------------------------------------------------------
//  EXTRACT ALL
//--------------------------------------------------------------
void MainWindow::on_extractAllButton_clicked()
{
    if (currentArchivePath.isEmpty()) {
        QMessageBox::warning(this, tr("No Archive"), tr("Open an archive first."));
        return;
    }

    QString outputDir = QFileDialog::getExistingDirectory(this, tr("Select Extraction Directory"), QDir::homePath());
    if (outputDir.isEmpty())
        return;

    // Gather all entries
    QStringList allEntries;
    for (int i = 0; i < ui->listWidget->count(); ++i)
        allEntries << ui->listWidget->item(i)->text();

    // Warn if any extracted item already exists
    for (const QString &entry : allEntries) {
        QFileInfo target(outputDir + QDir::separator() + entry);
        if (target.exists()) {
            auto reply = QMessageBox::warning(
                this,
                tr("Overwrite Exists"),
                tr("'%1' already exists in '%2'. Overwrite?" ).arg(entry).arg(outputDir),
                QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
            break;
        }
    }

    QString err;
    if (runExtract(currentArchivePath, outputDir, {}, err)) {
        QMessageBox::information(this, tr("Success"), tr("Extracted entire archive to:\n%1").arg(outputDir));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Extraction failed:\n\n%1").arg(err));
    }
}

//--------------------------------------------------------------
//  CREATE ARCHIVE
//--------------------------------------------------------------
void MainWindow::on_createButton_clicked()
{
    if (droppedPaths.isEmpty()) {
        QMessageBox::warning(this, tr("No Items"), tr("Please drag and drop files or folders first."));
        return;
    }

    QString selectedFilter;
    QString archivePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Archive"),
        QDir::homePath() + QDir::separator() + "archive",
        tr("Tar (*.tar);;Tar GZ (*.tar.gz);;Tar BZ2 (*.tar.bz2);;Tar XZ (*.tar.xz)"),
        &selectedFilter);

    if (archivePath.isEmpty())
        return;

    // Ensure extension matches chosen format
    static QRegularExpression extRe(R"(\.(tar|tar\.gz|tar\.bz2|tar\.xz)$)", QRegularExpression::CaseInsensitiveOption);
    if (!extRe.match(archivePath).hasMatch()) {
        if (selectedFilter.contains(".tar.gz"))
            archivePath += ".tar.gz";
        else if (selectedFilter.contains(".tar.bz2"))
            archivePath += ".tar.bz2";
        else if (selectedFilter.contains(".tar.xz"))
            archivePath += ".tar.xz";
        else
            archivePath += ".tar";
    }

    // Warn if file exists and confirm overwrite
    if (QFileInfo::exists(archivePath)) {
        auto reply = QMessageBox::warning(
            this,
            tr("File Exists"),
            tr("The file '%1' already exists. Do you want to overwrite it?").arg(archivePath),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    // Check that all dropped items share the same parent directory
    QFileInfo baseFile(droppedPaths.first());
    QString baseDir = baseFile.absolutePath();

    QStringList relPaths;
    for (const QString &full : droppedPaths) {
        QFileInfo fi(full);
        if (fi.absolutePath() != baseDir) {
            QMessageBox::warning(this, tr("Error"), tr("All files must reside in the same folder."));
            return;
        }
        relPaths << fi.fileName();
    }

    QString program;
    QStringList args;
    bool useZip = archivePath.endsWith(".zip", Qt::CaseInsensitive);

    if (useZip) {
        program = "zip";
        args << "-r" << archivePath;
        args.append(relPaths);
    } else {
        program = "tar";
        if (archivePath.endsWith(".tar.gz", Qt::CaseInsensitive))
            args << "-czf" << archivePath;
        else if (archivePath.endsWith(".tar.bz2", Qt::CaseInsensitive))
            args << "-cjf" << archivePath;
        else if (archivePath.endsWith(".tar.xz", Qt::CaseInsensitive))
            args << "-cJf" << archivePath;
        else
            args << "-cf" << archivePath;
        args.append(relPaths);
    }

    QProcess proc;
    proc.setWorkingDirectory(baseDir);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(program, args);
    proc.waitForFinished();

    QString output = proc.readAll();

    if (proc.exitCode() == 0) {
        QMessageBox::information(this, tr("Success"), tr("Archive created:\n%1").arg(archivePath));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create archive:\n\n%1").arg(output));
    }
}
