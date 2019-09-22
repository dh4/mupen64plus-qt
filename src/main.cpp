/***
 * Copyright (c) 2013, Dan Hasting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the organization nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

#include "global.h"
#include "common.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QScreen>
#include <QTranslator>


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator translator;
    QString language = SETTINGS.value("language", getDefaultLanguage()).toString();

    if (language != "EN") {
        QString resource = ":/locale/"+AppNameLower+"_"+language.toLower()+".qm";
        if (QFileInfo(resource).exists()) {
            translator.load(resource);
            application.installTranslator(&translator);
        }
    }

    QCoreApplication::setOrganizationName(ParentName);
    QCoreApplication::setApplicationName(AppName);

    MainWindow window;


    QString maximized = SETTINGS.value("Geometry/maximized", "").toString();
    QString windowx = SETTINGS.value("Geometry/windowx", "").toString();
    QString windowy = SETTINGS.value("Geometry/windowy", "").toString();

    if (maximized == "true") {
        window.showMaximized();
    } else {
        window.show();
    }

    if (windowx == "" && windowy == "") {
        window.move(QGuiApplication::primaryScreen()->geometry().center() - window.rect().center());
    }

    return application.exec();
}
