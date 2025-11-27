/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *dijstra_alg;
    QAction *A_star_alg;
    QAction *ant_alg;
    QAction *importJSON;
    QAction *podlozka;
    QAction *Run;
    QWidget *centralwidget;
    QGraphicsView *graphicsView_map;
    QMenuBar *menubar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *run;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 610);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(800, 610));
        MainWindow->setStyleSheet(QString::fromUtf8("/* --- Glassy light-blue / light-purple theme --- */\n"
"/* \320\236\321\201\320\275\320\276\320\262\320\275\321\213\320\265 \321\206\320\262\320\265\321\202\320\260 (\320\277\320\276\320\264\321\201\320\272\320\260\320\267\320\272\320\260): \320\274\320\276\320\266\320\275\320\276 \320\277\320\276\320\264\320\277\321\200\320\260\320\262\320\270\321\202\321\214 rgba() \320\277\320\276 \320\262\320\272\321\203\321\201\321\203 */\n"
"QWidget {\n"
"    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 rgba(18,22,30,200), stop:1 rgba(10,12,18,220)); /* \321\202\321\221\320\274\320\275\321\213\320\271 \321\204\320\276\320\275 \320\276\320\272\320\275\320\260 */\n"
"    color: rgba(240, 246, 255, 0.95); /* \320\276\321\201\320\275\320\276\320\262\320\275\320\276\320\271 \321\206\320\262\320\265\321\202 \321\202\320\265\320\272\321\201\321\202\320\260 */\n"
"    font-family: \"Segoe UI\", \"Roboto\", \"Arial\";\n"
"    font-size: 12px;\n"
"}\n"
"\n"
"/* \320\246\320\265\320\275\321\202\321\200"
                        "\320\260\320\273\321\214\320\275\320\260\321\217 \302\253\321\201\321\202\320\265\320\272\320\273\321\217\320\275\320\275\320\260\321\217\302\273 \320\277\320\260\320\275\320\265\320\273\321\214 \342\200\224 \320\261\320\276\320\273\321\214\321\210\320\270\320\275\321\201\321\202\320\262\320\276 \320\262\320\270\320\264\320\266\320\265\321\202\320\276\320\262 \320\275\320\260\321\201\320\273\320\265\320\264\321\203\321\216\321\202 \321\201\321\202\320\270\320\273\321\214 \320\276\321\202 QWidget */\n"
"QWidget#centralwidget, QFrame#centralwidget {\n"
"    background-color: rgba(255, 255, 255, 0.03); /* \321\202\320\276\320\275\320\272\320\260\321\217 \321\201\320\262\320\265\321\202\320\273\320\260\321\217 \320\277\320\273\321\221\320\275\320\272\320\260 */\n"
"    border: 1px solid rgba(255,255,255,0.06);\n"
"    border-radius: 10px;\n"
"}\n"
"\n"
"/* GraphicsView \342\200\224 \320\261\320\265\320\273\320\260\321\217 \320\276\320\261\320\273\320\260\321\201\321\202\321\214 \321\201 \320\273\321\221\320\263\320"
                        "\272\320\276\320\271 \302\253\321\200\320\260\320\274\320\272\320\276\320\271 \321\201\321\202\320\265\320\272\320\273\320\260\302\273 */\n"
"QGraphicsView, QGraphicsView#graphicsView_map {\n"
"    background-color: rgba(255,255,255,0.02);\n"
"    border: 1px solid rgba(200,210,255,0.06);\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"/* \320\234\320\265\320\275\321\216-\320\261\320\260\321\200 */\n"
"QMenuBar {\n"
"    background: transparent;\n"
"    spacing: 6px;\n"
"    padding: 6px 8px;\n"
"}\n"
"\n"
"QMenuBar::item {\n"
"    background: rgba(255,255,255,0.02);\n"
"    padding: 6px 10px;\n"
"    margin: 0 3px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"QMenuBar::item:selected {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 rgba(160,200,255,0.12), stop:1 rgba(200,160,255,0.10));\n"
"    color: rgba(255,255,255,0.98);\n"
"}\n"
"\n"
"/* \320\234\320\265\320\275\321\216 \320\262\321\213\320\277\320\260\320\264\320\260\321\216\321\211\320\265\320\265 */\n"
"QMenu {\n"
"    backgrou"
                        "nd-color: rgba(15,18,22,220);\n"
"    border: 1px solid rgba(255,255,255,0.05);\n"
"    padding: 6px;\n"
"    border-radius: 8px;\n"
"    color: rgba(240,246,255,0.95);\n"
"}\n"
"\n"
"QMenu::item {\n"
"    padding: 6px 24px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"QMenu::item:selected {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 rgba(140,180,255,0.14), stop:1 rgba(190,140,255,0.12));\n"
"    color: rgba(10,10,12,0.98);\n"
"}\n"
"\n"
"/* \320\232\320\275\320\276\320\277\320\272\320\270 */\n"
"QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 rgba(130,180,255,0.12), stop:1 rgba(200,160,255,0.10));\n"
"    border: 1px solid rgba(255,255,255,0.07);\n"
"    color: rgba(10,10,12,0.95);\n"
"    padding: 6px 12px;\n"
"    border-radius: 8px;\n"
"    min-height: 28px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 rgba(130,180,255,0.20), stop:1 rgba(200,160,255,0.16));\n"
""
                        "    transform: none;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgba(120,150,220,0.22);\n"
"    border: 1px solid rgba(100,120,180,0.25);\n"
"}\n"
"\n"
"/* Flat buttons / tool buttons (\320\274\320\265\320\275\321\216 \320\274\320\276\320\266\320\265\321\202 \320\270\321\201\320\277\320\276\320\273\321\214\320\267\320\276\320\262\320\260\321\202\321\214 \321\202\320\260\320\272\320\270\320\265) */\n"
"QToolButton {\n"
"    background: transparent;\n"
"    border: none;\n"
"    padding: 4px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"QToolButton:hover {\n"
"    background: rgba(255,255,255,0.03);\n"
"}\n"
"\n"
"/* \320\241\321\202\320\260\321\202\321\203\321\201\320\261\320\260\321\200 */\n"
"QStatusBar {\n"
"    background: transparent;\n"
"    border-top: 1px solid rgba(255,255,255,0.03);\n"
"    padding: 4px;\n"
"    color: rgba(200,210,230,0.9);\n"
"}\n"
"\n"
"/* \320\237\320\276\320\273\321\217 \320\262\320\262\320\276\320\264\320\260 */\n"
"QLineEdit, QTextEdit, QPlainTextEdit {\n"
"  "
                        "  background-color: rgba(255,255,255,0.02);\n"
"    border: 1px solid rgba(255,255,255,0.05);\n"
"    border-radius: 6px;\n"
"    padding: 6px;\n"
"    color: rgba(240,246,255,0.95);\n"
"}\n"
"\n"
"/* \320\232\320\276\320\274\320\261\320\276\320\261\320\276\320\272\321\201 */\n"
"QComboBox {\n"
"    background-color: rgba(255,255,255,0.02);\n"
"    border: 1px solid rgba(255,255,255,0.05);\n"
"    padding: 4px 8px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"    subcontrol-origin: padding;\n"
"    subcontrol-position: top right;\n"
"    width: 20px;\n"
"    border-left: 1px solid rgba(255,255,255,0.03);\n"
"}\n"
"\n"
"/* \320\237\321\200\320\276\320\263\321\200\320\265\321\201\321\201\320\261\320\260\321\200 */\n"
"QProgressBar {\n"
"    background-color: rgba(255,255,255,0.02);\n"
"    border: 1px solid rgba(255,255,255,0.04);\n"
"    border-radius: 8px;\n"
"    text-align: center;\n"
"    padding: 2px;\n"
"    color: rgba(240,246,255,0.95);\n"
"}\n"
"\n"
"QProgressBar::chunk {\n"
" "
                        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 rgba(120,170,255,0.6), stop:1 rgba(190,130,255,0.6));\n"
"    border-radius: 8px;\n"
"}\n"
"\n"
"/* \320\241\320\277\320\270\321\201\320\272\320\270 \320\270 \320\264\320\265\321\200\320\265\320\262\320\276 */\n"
"QListView, QTreeView {\n"
"    background-color: rgba(255,255,255,0.015);\n"
"    border: 1px solid rgba(255,255,255,0.03);\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"/* \320\241\320\272\321\200\320\276\320\273\320\273\320\261\320\260\321\200\321\213 (\321\201\320\264\320\265\321\200\320\266\320\260\320\275\320\275\321\213\320\271 \321\201\321\202\320\270\320\273\321\214) */\n"
"QScrollBar:vertical {\n"
"    background: transparent;\n"
"    width: 10px;\n"
"    margin: 6px 2px 6px 2px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical {\n"
"    background: rgba(200,210,255,0.10);\n"
"    min-height: 20px;\n"
"    border-radius: 5px;\n"
"    border: 1px solid rgba(255,255,255,0.03);\n"
"}\n"
"\n"
"QScrollBar::add-line, QScrollBar::sub-"
                        "line { height: 0; }\n"
"\n"
"/* Tooltip */\n"
"QToolTip {\n"
"    background-color: rgba(20,24,30,230);\n"
"    color: rgba(240,246,255,0.95);\n"
"    border: 1px solid rgba(255,255,255,0.05);\n"
"    padding: 6px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"/* Disabled state \342\200\224 \320\274\320\265\320\275\320\265\320\265 \321\217\321\200\320\272\320\276 */\n"
"*:disabled {\n"
"    color: rgba(200,210,220,0.6);\n"
"}\n"
"\n"
"/* \320\234\320\270\320\275\320\270-\320\277\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \320\264\320\273\321\217 \320\260\320\272\321\206\320\265\320\275\321\202\320\276\320\262 \342\200\224 \320\270\321\201\320\277\320\276\320\273\321\214\320\267\320\276\320\262\320\260\321\202\321\214 \320\272\320\273\320\260\321\201\321\201 .accent \320\263\320\264\320\265 \320\275\321\203\320\266\320\275\320\276 */\n"
"QWidget.accent, QPushButton.accent {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 rgba(120,170,255,0.24), stop:1 rgba(200,14"
                        "0,255,0.20));\n"
"    border: 1px solid rgba(180,190,255,0.10);\n"
"    color: rgba(8,8,10,0.95);\n"
"}\n"
""));
        dijstra_alg = new QAction(MainWindow);
        dijstra_alg->setObjectName("dijstra_alg");
        dijstra_alg->setCheckable(true);
        dijstra_alg->setChecked(true);
        A_star_alg = new QAction(MainWindow);
        A_star_alg->setObjectName("A_star_alg");
        A_star_alg->setCheckable(true);
        ant_alg = new QAction(MainWindow);
        ant_alg->setObjectName("ant_alg");
        ant_alg->setCheckable(true);
        ant_alg->setEnabled(false);
        importJSON = new QAction(MainWindow);
        importJSON->setObjectName("importJSON");
        podlozka = new QAction(MainWindow);
        podlozka->setObjectName("podlozka");
        Run = new QAction(MainWindow);
        Run->setObjectName("Run");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy1);
        graphicsView_map = new QGraphicsView(centralwidget);
        graphicsView_map->setObjectName("graphicsView_map");
        graphicsView_map->setGeometry(QRect(10, 10, 781, 531));
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(100);
        sizePolicy2.setVerticalStretch(100);
        sizePolicy2.setHeightForWidth(graphicsView_map->sizePolicy().hasHeightForWidth());
        graphicsView_map->setSizePolicy(sizePolicy2);
        graphicsView_map->setMinimumSize(QSize(781, 531));
        graphicsView_map->setMaximumSize(QSize(16777215, 16777215));
        graphicsView_map->setTabletTracking(false);
        graphicsView_map->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        graphicsView_map->setFrameShadow(QFrame::Shadow::Sunken);
        graphicsView_map->setLineWidth(2);
        graphicsView_map->setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
        graphicsView_map->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::NoViewportUpdate);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 40));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        menu_2 = new QMenu(menubar);
        menu_2->setObjectName("menu_2");
        run = new QMenu(menubar);
        run->setObjectName("run");
        run->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu->menuAction());
        menubar->addAction(menu_2->menuAction());
        menubar->addAction(run->menuAction());
        menu->addAction(importJSON);
        menu->addAction(podlozka);
        menu_2->addAction(dijstra_alg);
        menu_2->addAction(A_star_alg);
        menu_2->addAction(ant_alg);
        run->addAction(Run);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        dijstra_alg->setText(QCoreApplication::translate("MainWindow", "\320\224\320\265\320\271\320\272\321\201\321\202\321\200\320\260", nullptr));
        A_star_alg->setText(QCoreApplication::translate("MainWindow", "A*", nullptr));
        ant_alg->setText(QCoreApplication::translate("MainWindow", "\320\234\321\203\321\200\320\260\320\262\321\214\320\270\320\275\321\213\320\271 \320\277\320\276\320\270\321\201\320\272 (\320\262 \321\200\320\260\320\267\321\200\320\260\320\261\320\276\321\202\320\272\320\265)", nullptr));
        importJSON->setText(QCoreApplication::translate("MainWindow", "\320\230\320\274\320\277\320\276\321\200\321\202 JSON \320\263\321\200\320\260\321\204\320\260", nullptr));
        podlozka->setText(QCoreApplication::translate("MainWindow", "\320\230\320\274\320\277\320\276\321\200\321\202 \320\277\320\276\320\264\320\273\320\276\320\266\320\272\320\270 (.PNG)", nullptr));
        Run->setText(QCoreApplication::translate("MainWindow", "Run", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\320\230\320\274\320\277\320\276\321\200\321\202", nullptr));
        menu_2->setTitle(QCoreApplication::translate("MainWindow", "\320\220\320\273\320\263\320\276\321\200\320\270\321\202\320\274", nullptr));
        run->setTitle(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\277\321\203\321\201\320\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
