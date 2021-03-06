/********************************************************************************
** Form generated from reading UI file 'sentrygui.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SENTRYGUI_H
#define UI_SENTRYGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sentryguiClass
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *camVLabel;
    QLineEdit *camVDisplay;
    QLabel *turretHLabel;
    QLineEdit *turretHDisplay;
    QLabel *camHLabel;
    QLineEdit *camHDisplay;
    QLabel *rangeLabel;
    QLineEdit *rangeDisplay;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *statusLabel;
    QLineEdit *statusDisplay;
    QLabel *modeLabel;
    QLineEdit *modeDisplay;
    QGraphicsView *camView;
    QTextEdit *console;
    QPushButton *resetButton;
    QPushButton *calibrateButton;
    QPushButton *stopButton;
    QWidget *layoutWidget2;
    QVBoxLayout *verticalLayout;
    QLabel *feedbackLabel;
    QPushButton *VeryHigh;
    QPushButton *High;
    QPushButton *Hit;
    QPushButton *Low;
    QPushButton *VeryLow;
    QButtonGroup *feedbackGroup;

    void setupUi(QWidget *sentryguiClass)
    {
        if (sentryguiClass->objectName().isEmpty())
            sentryguiClass->setObjectName(QStringLiteral("sentryguiClass"));
        sentryguiClass->resize(1332, 559);
        sentryguiClass->setFocusPolicy(Qt::ClickFocus);
        layoutWidget = new QWidget(sentryguiClass);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 510, 821, 35));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        camVLabel = new QLabel(layoutWidget);
        camVLabel->setObjectName(QStringLiteral("camVLabel"));
        QFont font;
        font.setPointSize(11);
        camVLabel->setFont(font);
        camVLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(camVLabel);

        camVDisplay = new QLineEdit(layoutWidget);
        camVDisplay->setObjectName(QStringLiteral("camVDisplay"));
        camVDisplay->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(camVDisplay->sizePolicy().hasHeightForWidth());
        camVDisplay->setSizePolicy(sizePolicy);
        camVDisplay->setMaximumSize(QSize(100, 16777215));
        camVDisplay->setFont(font);
        camVDisplay->setMouseTracking(false);
        camVDisplay->setAcceptDrops(false);
        camVDisplay->setLayoutDirection(Qt::LeftToRight);
        camVDisplay->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        camVDisplay->setReadOnly(true);

        horizontalLayout->addWidget(camVDisplay);

        turretHLabel = new QLabel(layoutWidget);
        turretHLabel->setObjectName(QStringLiteral("turretHLabel"));
        turretHLabel->setFont(font);
        turretHLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(turretHLabel);

        turretHDisplay = new QLineEdit(layoutWidget);
        turretHDisplay->setObjectName(QStringLiteral("turretHDisplay"));
        turretHDisplay->setEnabled(true);
        sizePolicy.setHeightForWidth(turretHDisplay->sizePolicy().hasHeightForWidth());
        turretHDisplay->setSizePolicy(sizePolicy);
        turretHDisplay->setMaximumSize(QSize(100, 16777215));
        turretHDisplay->setFont(font);
        turretHDisplay->setMouseTracking(false);
        turretHDisplay->setAcceptDrops(false);
        turretHDisplay->setReadOnly(true);

        horizontalLayout->addWidget(turretHDisplay);

        camHLabel = new QLabel(layoutWidget);
        camHLabel->setObjectName(QStringLiteral("camHLabel"));
        camHLabel->setFont(font);
        camHLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(camHLabel);

        camHDisplay = new QLineEdit(layoutWidget);
        camHDisplay->setObjectName(QStringLiteral("camHDisplay"));
        camHDisplay->setEnabled(true);
        sizePolicy.setHeightForWidth(camHDisplay->sizePolicy().hasHeightForWidth());
        camHDisplay->setSizePolicy(sizePolicy);
        camHDisplay->setMaximumSize(QSize(100, 16777215));
        camHDisplay->setFont(font);
        camHDisplay->setMouseTracking(false);
        camHDisplay->setAcceptDrops(false);
        camHDisplay->setReadOnly(true);

        horizontalLayout->addWidget(camHDisplay);

        rangeLabel = new QLabel(layoutWidget);
        rangeLabel->setObjectName(QStringLiteral("rangeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(rangeLabel->sizePolicy().hasHeightForWidth());
        rangeLabel->setSizePolicy(sizePolicy1);
        rangeLabel->setFont(font);
        rangeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(rangeLabel);

        rangeDisplay = new QLineEdit(layoutWidget);
        rangeDisplay->setObjectName(QStringLiteral("rangeDisplay"));
        rangeDisplay->setEnabled(true);
        sizePolicy.setHeightForWidth(rangeDisplay->sizePolicy().hasHeightForWidth());
        rangeDisplay->setSizePolicy(sizePolicy);
        rangeDisplay->setMaximumSize(QSize(100, 16777215));
        rangeDisplay->setFont(font);
        rangeDisplay->setMouseTracking(false);
        rangeDisplay->setAcceptDrops(false);
        rangeDisplay->setReadOnly(true);

        horizontalLayout->addWidget(rangeDisplay);

        layoutWidget1 = new QWidget(sentryguiClass);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(840, 510, 474, 35));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        statusLabel = new QLabel(layoutWidget1);
        statusLabel->setObjectName(QStringLiteral("statusLabel"));
        statusLabel->setFont(font);
        statusLabel->setScaledContents(true);
        statusLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(statusLabel);

        statusDisplay = new QLineEdit(layoutWidget1);
        statusDisplay->setObjectName(QStringLiteral("statusDisplay"));
        statusDisplay->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(statusDisplay->sizePolicy().hasHeightForWidth());
        statusDisplay->setSizePolicy(sizePolicy2);
        statusDisplay->setMaximumSize(QSize(300, 16777215));
        statusDisplay->setFont(font);
        statusDisplay->setMouseTracking(false);
        statusDisplay->setAcceptDrops(false);
        statusDisplay->setFrame(true);
        statusDisplay->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        statusDisplay->setReadOnly(true);

        horizontalLayout_2->addWidget(statusDisplay);

        modeLabel = new QLabel(layoutWidget1);
        modeLabel->setObjectName(QStringLiteral("modeLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(modeLabel->sizePolicy().hasHeightForWidth());
        modeLabel->setSizePolicy(sizePolicy3);
        modeLabel->setMinimumSize(QSize(50, 0));
        modeLabel->setFont(font);
        modeLabel->setScaledContents(true);
        modeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(modeLabel);

        modeDisplay = new QLineEdit(layoutWidget1);
        modeDisplay->setObjectName(QStringLiteral("modeDisplay"));
        modeDisplay->setEnabled(true);
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(modeDisplay->sizePolicy().hasHeightForWidth());
        modeDisplay->setSizePolicy(sizePolicy4);
        modeDisplay->setMaximumSize(QSize(100, 16777215));
        modeDisplay->setFont(font);
        modeDisplay->setMouseTracking(false);
        modeDisplay->setAcceptDrops(false);
        modeDisplay->setReadOnly(true);

        horizontalLayout_2->addWidget(modeDisplay);

        camView = new QGraphicsView(sentryguiClass);
        camView->setObjectName(QStringLiteral("camView"));
        camView->setGeometry(QRect(50, 10, 650, 490));
        console = new QTextEdit(sentryguiClass);
        console->setObjectName(QStringLiteral("console"));
        console->setGeometry(QRect(720, 10, 431, 491));
        resetButton = new QPushButton(sentryguiClass);
        resetButton->setObjectName(QStringLiteral("resetButton"));
        resetButton->setGeometry(QRect(1170, 70, 131, 34));
        calibrateButton = new QPushButton(sentryguiClass);
        calibrateButton->setObjectName(QStringLiteral("calibrateButton"));
        calibrateButton->setGeometry(QRect(1170, 120, 131, 34));
        stopButton = new QPushButton(sentryguiClass);
        stopButton->setObjectName(QStringLiteral("stopButton"));
        stopButton->setGeometry(QRect(1170, 170, 131, 34));
        layoutWidget2 = new QWidget(sentryguiClass);
        layoutWidget2->setObjectName(QStringLiteral("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(1170, 220, 131, 230));
        verticalLayout = new QVBoxLayout(layoutWidget2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        feedbackLabel = new QLabel(layoutWidget2);
        feedbackLabel->setObjectName(QStringLiteral("feedbackLabel"));
        sizePolicy1.setHeightForWidth(feedbackLabel->sizePolicy().hasHeightForWidth());
        feedbackLabel->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        feedbackLabel->setFont(font1);
        feedbackLabel->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

        verticalLayout->addWidget(feedbackLabel);

        VeryHigh = new QPushButton(layoutWidget2);
        feedbackGroup = new QButtonGroup(sentryguiClass);
        feedbackGroup->setObjectName(QStringLiteral("feedbackGroup"));
        feedbackGroup->addButton(VeryHigh);
        VeryHigh->setObjectName(QStringLiteral("VeryHigh"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(VeryHigh->sizePolicy().hasHeightForWidth());
        VeryHigh->setSizePolicy(sizePolicy5);

        verticalLayout->addWidget(VeryHigh);

        High = new QPushButton(layoutWidget2);
        feedbackGroup->addButton(High);
        High->setObjectName(QStringLiteral("High"));
        sizePolicy5.setHeightForWidth(High->sizePolicy().hasHeightForWidth());
        High->setSizePolicy(sizePolicy5);

        verticalLayout->addWidget(High);

        Hit = new QPushButton(layoutWidget2);
        feedbackGroup->addButton(Hit);
        Hit->setObjectName(QStringLiteral("Hit"));
        sizePolicy5.setHeightForWidth(Hit->sizePolicy().hasHeightForWidth());
        Hit->setSizePolicy(sizePolicy5);

        verticalLayout->addWidget(Hit);

        Low = new QPushButton(layoutWidget2);
        feedbackGroup->addButton(Low);
        Low->setObjectName(QStringLiteral("Low"));
        sizePolicy5.setHeightForWidth(Low->sizePolicy().hasHeightForWidth());
        Low->setSizePolicy(sizePolicy5);

        verticalLayout->addWidget(Low);

        VeryLow = new QPushButton(layoutWidget2);
        feedbackGroup->addButton(VeryLow);
        VeryLow->setObjectName(QStringLiteral("VeryLow"));
        sizePolicy5.setHeightForWidth(VeryLow->sizePolicy().hasHeightForWidth());
        VeryLow->setSizePolicy(sizePolicy5);

        verticalLayout->addWidget(VeryLow);


        retranslateUi(sentryguiClass);

        QMetaObject::connectSlotsByName(sentryguiClass);
    } // setupUi

    void retranslateUi(QWidget *sentryguiClass)
    {
        sentryguiClass->setWindowTitle(QApplication::translate("sentryguiClass", "sentrygui", 0));
        camVLabel->setText(QApplication::translate("sentryguiClass", "Pitch", 0));
        turretHLabel->setText(QApplication::translate("sentryguiClass", "Turret Pan", 0));
        camHLabel->setText(QApplication::translate("sentryguiClass", "Camera Pan", 0));
        rangeLabel->setText(QApplication::translate("sentryguiClass", "Range", 0));
        statusLabel->setText(QApplication::translate("sentryguiClass", "Status:", 0));
        statusDisplay->setText(QApplication::translate("sentryguiClass", "Initializing", 0));
        modeLabel->setText(QApplication::translate("sentryguiClass", "Mode:", 0));
        modeDisplay->setText(QApplication::translate("sentryguiClass", "AUTO", 0));
        resetButton->setText(QApplication::translate("sentryguiClass", "RESET", 0));
        calibrateButton->setText(QApplication::translate("sentryguiClass", "CALIBRATE", 0));
        stopButton->setText(QApplication::translate("sentryguiClass", "STOP", 0));
        feedbackLabel->setText(QApplication::translate("sentryguiClass", "Shot Feedback", 0));
        VeryHigh->setText(QApplication::translate("sentryguiClass", "VERY HIGH", 0));
        High->setText(QApplication::translate("sentryguiClass", "TOO HIGH", 0));
        Hit->setText(QApplication::translate("sentryguiClass", "HIT", 0));
        Low->setText(QApplication::translate("sentryguiClass", "TOO LOW", 0));
        VeryLow->setText(QApplication::translate("sentryguiClass", "VERY LOW", 0));
    } // retranslateUi

};

namespace Ui {
    class sentryguiClass: public Ui_sentryguiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SENTRYGUI_H
