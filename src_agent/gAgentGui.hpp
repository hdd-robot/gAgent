/********************************************************************************
** Form generated from reading UI file 'gAgentGuiB13761.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef GAGENTGUIB13761_H
#define GAGENTGUIB13761_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_guiWindow
{
public:
    QAction *actionClose;
    QAction *actionGAgent;
    QWidget *centralwidget;
    QGraphicsView *graphicsView;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *guiWindow)
    {
        if (guiWindow->objectName().isEmpty())
            guiWindow->setObjectName(QStringLiteral("guiWindow"));
        guiWindow->resize(795, 483);
        guiWindow->setCursor(QCursor(Qt::ArrowCursor));
        actionClose = new QAction(guiWindow);
        actionClose->setObjectName(QStringLiteral("actionClose"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/close/icons/inside-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionClose->setIcon(icon);
        actionGAgent = new QAction(guiWindow);
        actionGAgent->setObjectName(QStringLiteral("actionGAgent"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/gAgent/icons/share-icon8.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionGAgent->setIcon(icon1);
        centralwidget = new QWidget(guiWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setGeometry(QRect(-2, 1, 800, 460));
        graphicsView->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CrossCursor)));
        guiWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(guiWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        guiWindow->setStatusBar(statusbar);

        retranslateUi(guiWindow);

        QMetaObject::connectSlotsByName(guiWindow);
    } // setupUi

    void retranslateUi(QMainWindow *guiWindow)
    {
        guiWindow->setWindowTitle(QApplication::translate("guiWindow", "MainWindow", 0));
        actionClose->setText(QApplication::translate("guiWindow", "Close", 0));
#ifndef QT_NO_TOOLTIP
        actionClose->setToolTip(QApplication::translate("guiWindow", "<html><head/><body><p>Close</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        actionClose->setShortcut(QApplication::translate("guiWindow", "Ctrl+Q", 0));
        actionGAgent->setText(QApplication::translate("guiWindow", "gAgent", 0));
#ifndef QT_NO_TOOLTIP
        actionGAgent->setToolTip(QApplication::translate("guiWindow", "gAgent", 0));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class guiWindow: public Ui_guiWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // GAGENTGUIB13761_H
