/********************************************************************************
** Form generated from reading UI file 'mysearchdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYSEARCHDIALOG_H
#define UI_MYSEARCHDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MySearchDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QHBoxLayout *horizontalLayout;
    QLabel *pathLabel;
    QLineEdit *pathEdit;
    QPushButton *browseButton;
    QSpacerItem *verticalSpacer;
    QLabel *resultLabel;
    QListWidget *result;
    QHBoxLayout *horizontalLayout_3;
    QLabel *foundLabel;
    QPushButton *searchButton;

    void setupUi(QDialog *MySearchDialog)
    {
        if (MySearchDialog->objectName().isEmpty())
            MySearchDialog->setObjectName("MySearchDialog");
        MySearchDialog->resize(500, 300);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/search_file_512.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        MySearchDialog->setWindowIcon(icon);
        MySearchDialog->setModal(false);
        verticalLayout = new QVBoxLayout(MySearchDialog);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        nameLabel = new QLabel(MySearchDialog);
        nameLabel->setObjectName("nameLabel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(nameLabel->sizePolicy().hasHeightForWidth());
        nameLabel->setSizePolicy(sizePolicy);
        nameLabel->setMinimumSize(QSize(75, 0));
        nameLabel->setMaximumSize(QSize(75, 16777215));

        horizontalLayout_2->addWidget(nameLabel);

        nameEdit = new QLineEdit(MySearchDialog);
        nameEdit->setObjectName("nameEdit");
        nameEdit->setMinimumSize(QSize(0, 25));
        nameEdit->setMaximumSize(QSize(16777215, 30));
        nameEdit->setContextMenuPolicy(Qt::NoContextMenu);

        horizontalLayout_2->addWidget(nameEdit);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pathLabel = new QLabel(MySearchDialog);
        pathLabel->setObjectName("pathLabel");
        sizePolicy.setHeightForWidth(pathLabel->sizePolicy().hasHeightForWidth());
        pathLabel->setSizePolicy(sizePolicy);
        pathLabel->setMinimumSize(QSize(75, 0));
        pathLabel->setMaximumSize(QSize(75, 16777215));

        horizontalLayout->addWidget(pathLabel);

        pathEdit = new QLineEdit(MySearchDialog);
        pathEdit->setObjectName("pathEdit");
        pathEdit->setMinimumSize(QSize(0, 25));
        pathEdit->setMaximumSize(QSize(16777215, 30));
        pathEdit->setContextMenuPolicy(Qt::NoContextMenu);

        horizontalLayout->addWidget(pathEdit);

        browseButton = new QPushButton(MySearchDialog);
        browseButton->setObjectName("browseButton");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(browseButton->sizePolicy().hasHeightForWidth());
        browseButton->setSizePolicy(sizePolicy1);
        browseButton->setMinimumSize(QSize(80, 25));
        browseButton->setMaximumSize(QSize(80, 16777215));
        browseButton->setContextMenuPolicy(Qt::NoContextMenu);

        horizontalLayout->addWidget(browseButton);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 60, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        resultLabel = new QLabel(MySearchDialog);
        resultLabel->setObjectName("resultLabel");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(resultLabel->sizePolicy().hasHeightForWidth());
        resultLabel->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(resultLabel);

        result = new QListWidget(MySearchDialog);
        result->setObjectName("result");

        verticalLayout->addWidget(result);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        foundLabel = new QLabel(MySearchDialog);
        foundLabel->setObjectName("foundLabel");

        horizontalLayout_3->addWidget(foundLabel);

        searchButton = new QPushButton(MySearchDialog);
        searchButton->setObjectName("searchButton");
        sizePolicy1.setHeightForWidth(searchButton->sizePolicy().hasHeightForWidth());
        searchButton->setSizePolicy(sizePolicy1);
        searchButton->setMinimumSize(QSize(0, 25));
        searchButton->setMaximumSize(QSize(80, 16777215));
        searchButton->setContextMenuPolicy(Qt::NoContextMenu);

        horizontalLayout_3->addWidget(searchButton);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(MySearchDialog);

        searchButton->setDefault(true);


        QMetaObject::connectSlotsByName(MySearchDialog);
    } // setupUi

    void retranslateUi(QDialog *MySearchDialog)
    {
        MySearchDialog->setWindowTitle(QCoreApplication::translate("MySearchDialog", "Search Files", nullptr));
        nameLabel->setText(QCoreApplication::translate("MySearchDialog", "Search Files:", nullptr));
        pathLabel->setText(QCoreApplication::translate("MySearchDialog", "Where:", nullptr));
        browseButton->setText(QCoreApplication::translate("MySearchDialog", "Browse", nullptr));
        resultLabel->setText(QCoreApplication::translate("MySearchDialog", "Result:", nullptr));
        foundLabel->setText(QCoreApplication::translate("MySearchDialog", "Found", nullptr));
        searchButton->setText(QCoreApplication::translate("MySearchDialog", "Search", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MySearchDialog: public Ui_MySearchDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYSEARCHDIALOG_H
