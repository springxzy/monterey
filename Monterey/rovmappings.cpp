/*
    Copyright (C) 2012  Chris Konstad (chriskon149@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "rovmappings.h"
#include "ui_rovmappings.h"
#include "mainwindow.h"

#include <QLayout>
#include <QtDebug>
#include <QMessageBox>

ROVMappings::ROVMappings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ROVMappings)
{
    ui->setupUi(this);

    //Load the existing mappings
    MainWindow *p = dynamic_cast<MainWindow *> (this->parentWidget());
    if(p->controller->isJoyAttached())
    {
        numAxes = p->controller->getJoystickNumberAxes();
        //Load maximum axes
        QList<QSpinBox*> spinBoxes = this->findChildren<QSpinBox*>();
        foreach(QSpinBox* sb, spinBoxes)
        {
            sb->setMaximum(p->controller->getJoystickNumberAxes() - 1);
        }

        //Load maximum buttons
        QList<QComboBox*> comboButtons;
        comboButtons.append(ui->cbS0DBut);
        comboButtons.append(ui->cbS0IBut);
        comboButtons.append(ui->cbS1DBut);
        comboButtons.append(ui->cbS1IBut);
        foreach(QComboBox* cb, comboButtons)    //for each combo box
        {
            for(int b=0; b < p->controller->getJoystickNumberButtons(); b++)    //add each button
            {
                cb->addItem(QString::number(b));
            }
            cb->addItem("Disable"); //add option to disable
        }

        //Load axes
        ui->sbVX->setValue(p->controller->getAxisX());
        ui->sbVY->setValue(p->controller->getAxisY());
        ui->sbVZ->setValue(p->controller->getAxisZ());
        ui->sbV->setValue(p->controller->getAxisV());
        ui->sbTL->setValue(p->controller->getAxisL());
        ui->sbTR->setValue(p->controller->getAxisR());

        //Load relay settings
        QVBoxLayout *buttonLayout = ui->verticalLayoutButtons;
        QVBoxLayout *hatLayout = ui->verticalLayoutHats;
        for(int i=0; i<p->controller->relayMappings.count(); i++)
        {
            //Load buttons
            QHBoxLayout *newLayout = new QHBoxLayout(this);
            QLabel *le = new QLabel(this);
            QComboBox *cb = new QComboBox(this);
            relayButtons.append(cb);

            for(int b=0; b < p->controller->getJoystickNumberButtons(); b++)    //add each button
            {
                cb->addItem(QString::number(b));
            }
            cb->addItem("Disable"); //add option to disable

            le->setText(p->controller->rov->listRelays.at(i)->getName());
            cb->setCurrentIndex(p->controller->relayMappings[i].button);

            newLayout->addWidget(le);
            newLayout->addWidget(cb);
            buttonLayout->addLayout(newLayout);

            //Load hats
            QHBoxLayout *newHat = new QHBoxLayout(this);
            QLabel *label = new QLabel(this);
            QLineEdit *lineEdit = new QLineEdit(this);
            relayHats.append(lineEdit);

            label->setText(p->controller->rov->listRelays.at(i)->getName());
            label->setAlignment(Qt::AlignLeft);
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            lineEdit->setFixedWidth(100);
            lineEdit->setAlignment(Qt::AlignHCenter);
            lineEdit->setText(QString::number(p->controller->relayMappings[i].hat));
            newHat->addWidget(label);
            newHat->addWidget(lineEdit);
            hatLayout->addLayout(newHat);
        }

        //TODO: Handle like relays when the rest of the code is ready
        //Load servo settings
        ui->cbS0DBut->setCurrentIndex(p->controller->rov->listServos[0]->getButtonDown());
        ui->cbS0IBut->setCurrentIndex(p->controller->rov->listServos[0]->getButtonUp());
        ui->cbS1DBut->setCurrentIndex(p->controller->rov->listServos[1]->getButtonDown());
        ui->cbS1IBut->setCurrentIndex(p->controller->rov->listServos[1]->getButtonUp());

        ui->leS0DHat->setText(QString::number(p->controller->rov->listServos[0]->getHatDown()));
        ui->leS0IHat->setText(QString::number(p->controller->rov->listServos[0]->getHatUp()));
        ui->leS1DHat->setText(QString::number(p->controller->rov->listServos[1]->getHatDown()));
        ui->leS1IHat->setText(QString::number(p->controller->rov->listServos[1]->getHatUp()));
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Mappings dialog is not fully functional.");
        msgBox.setInformativeText("Please attach a joystick for full functionality.");
        msgBox.exec();
    }

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    updateTimer->start(50);
    qDebug() << "Setup the mappings dialog!";
}

ROVMappings::~ROVMappings()
{
    qDebug() << "Mappings dialog closed";
    delete ui;
    this->deleteLater();
}

void ROVMappings::on_pbSave_clicked()
{
    MainWindow *p = dynamic_cast<MainWindow *> (this->parentWidget());
    if(p->controller->isJoyAttached())
    {
        //Set the axes
        p->controller->setAxisL(ui->sbTL->value());
        p->controller->setAxisR(ui->sbTR->value());
        p->controller->setAxisV(ui->sbV->value());
        p->controller->setAxisX(ui->sbVX->value());
        p->controller->setAxisY(ui->sbVY->value());
        p->controller->setAxisZ(ui->sbVZ->value());

        //Set the relays
        for(int i=0; i<p->controller->relayMappings.count(); i++)
        {
            p->controller->relayMappings[i].button = relayButtons.at(i)->currentIndex();
            p->controller->relayMappings[i].hat = relayHats.at(i)->text().toInt();
        }

        //Set the servos (buttons)
        p->controller->rov->listServos[0]->setButtonDown(ui->cbS0DBut->currentIndex());
        p->controller->rov->listServos[0]->setButtonUp(ui->cbS0IBut->currentIndex());
        p->controller->rov->listServos[1]->setButtonDown(ui->cbS1DBut->currentIndex());
        p->controller->rov->listServos[1]->setButtonUp(ui->cbS1IBut->currentIndex());

        //Set the servos (hats)
        p->controller->rov->listServos[0]->setHatDown(ui->leS0DHat->text().toInt());
        p->controller->rov->listServos[0]->setHatUp(ui->leS0IHat->text().toInt());
        p->controller->rov->listServos[1]->setHatDown(ui->leS1DHat->text().toInt());
        p->controller->rov->listServos[1]->setHatUp(ui->leS1IHat->text().toInt());

        p->controller->saveSettings();  //save the settings
    }

    delete updateTimer;
    this->close();
}

void ROVMappings::on_pbCancel_clicked()
{
    delete updateTimer;
    this->close();
}

void ROVMappings::updateDisplay()
{
    MainWindow *p = dynamic_cast<MainWindow *> (this->parentWidget());
    if(p->controller->isJoyAttached())
    {
        //Display axes values
        qDebug() << "Displaying 1 axis values";
        Q_ASSERT(ui->sbTL->value() < numAxes && ui->sbTL->value() >= 0);
        ui->pbTL->setValue(p->controller->getJoystickAxesValues(ui->sbTL->value()));
        qDebug() << ui->pbTL->value();
        qDebug() << "Displaying 2 axis values";
        Q_ASSERT(ui->sbTR->value() < numAxes && ui->sbTR->value() >= 0);
        ui->pbTR->setValue(p->controller->getJoystickAxesValues(ui->sbTR->value()));
        qDebug() << ui->pbTR->value();
        qDebug() << "Displaying 3 axis values";
        Q_ASSERT(ui->sbV->value() < numAxes && ui->sbV->value() >= 0);
        ui->pbV->setValue(p->controller->getJoystickAxesValues(ui->sbV->value()));
        qDebug() << ui->pbV->value();
        qDebug() << "Displaying 4 axis values";
        Q_ASSERT(ui->sbVX->value() < numAxes && ui->sbVX->value() >= 0);
        ui->pbVX->setValue(p->controller->getJoystickAxesValues(ui->sbVX->value()));
        qDebug() << ui->pbVX->value();
        qDebug() << "Displaying 5 axis values";
        Q_ASSERT(ui->sbVY->value() < numAxes && ui->sbVY->value() >= 0);
        ui->pbVY->setValue(p->controller->getJoystickAxesValues(ui->sbVY->value()));
        qDebug() << ui->pbVY->value();
        qDebug() << "Displaying 6 axis values";
        Q_ASSERT(ui->sbVZ->value() < numAxes && ui->sbVZ->value() >= 0);
        ui->pbVZ->setValue(p->controller->getJoystickAxesValues(ui->sbVZ->value()));
        qDebug() << ui->pbVZ->value();

        //Display hat value
        qDebug() << "Displaying hat value!";
        ui->leCurrentHat->setText(QString::number(p->controller->getJoystickCurrentHatValue()));

        //Display currently pressed buttons
        qDebug() << "Displaying button values!";
        QString temp;
        QList<int> tempList = p->controller->getJoystickCurrentButtonValue();
        if(tempList.count())
        {
            if(tempList[0] == -1)   //if no button pressed
            {
                temp.append(" ");
            }
            else if(tempList.count() == 1)   //if only one item
            {
                temp.append(QString::number(tempList[0]));
            }
            else    //if multiple buttons are pressed
            {
                for(int i=0;i<tempList.count();i++) //should fix crash as of 8/1/2012
                {
                    temp.append(QString::number(i));
                    temp.append(", ");
                }
                if(temp.endsWith(QChar(','), Qt::CaseInsensitive))
                    temp.chop(temp.count()-1);  //remove last comma
            }
            ui->leCurrentButton->setText(temp);
        }
        else
            qWarning() << "tempList has no values" << tempList;

    }
}
