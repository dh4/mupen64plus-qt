/***
 * Copyright (c) 2013, Presence
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

#include "emulatorhandler.h"


EmulatorHandler::EmulatorHandler(MainWindow *parent) : QObject(parent)
{
    main = parent;
    lastOutput = "";
}

void EmulatorHandler::checkStatus(int status)
{
    if (status > 0)
        QMessageBox::warning(main, tr("Warning"),
            tr("Mupen64Plus quit unexpectedly. Check to make sure you are using a valid ROM."));
}


void EmulatorHandler::cleanTemp()
{
    QFile::remove(QDir::tempPath() + "/cen64-qt/temp.z64");
}


void EmulatorHandler::emitFinished()
{
    emit finished();
}


void EmulatorHandler::readOutput()
{
    lastOutput = emulatorProc->readAllStandardOutput();
}


void EmulatorHandler::startEmulator(QDir romDir, QString romFileName, QString zipFileName)
{
    QString completeRomPath;
    bool zip = false;

    if (zipFileName != "") { //If zipped file, extract and write to temp location for loading
        zip = true;

        QString zipFile = romDir.absoluteFilePath(zipFileName);
        QuaZipFile zippedFile(zipFile, romFileName);

        zippedFile.open(QIODevice::ReadOnly);
        QByteArray romData;
        romData.append(zippedFile.readAll());
        zippedFile.close();

        QString tempDir = QDir::tempPath() + "/mupen64plus-qt";
        QDir().mkdir(tempDir);
        completeRomPath = tempDir + "/temp.n64";

        QFile tempRom(completeRomPath);
        tempRom.open(QIODevice::WriteOnly);
        tempRom.write(romData);
        tempRom.close();
    } else
        completeRomPath = romDir.absoluteFilePath(romFileName);

    QString mupen64Path = SETTINGS.value("Paths/mupen64plus", "").toString();
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QString pluginPath = SETTINGS.value("Paths/plugins", "").toString();
    QDir dataDir(dataPath);
    QDir configDir(configPath);
    QDir pluginDir(pluginPath);

    QString emuMode = SETTINGS.value("Emulation/mode", "").toString();

    QString resolution = SETTINGS.value("Graphics/resolution", "").toString();

    QString videoPlugin = SETTINGS.value("Plugins/video", "").toString();
    QString audioPlugin = SETTINGS.value("Plugins/audio", "").toString();
    QString inputPlugin = SETTINGS.value("Plugins/input", "").toString();
    QString rspPlugin = SETTINGS.value("Plugins/rsp", "").toString();

    QFile mupen64File(mupen64Path);
    QFile romFile(completeRomPath);


    //Sanity checks
    if(!mupen64File.exists() || QFileInfo(mupen64File).isDir() || !QFileInfo(mupen64File).isExecutable()) {
        QMessageBox::warning(main, tr("Warning"), tr("Mupen64Plus executable not found."));
        if (zip) cleanTemp();
        return;
    }

    if(!romFile.exists() || QFileInfo(romFile).isDir()) {
        QMessageBox::warning(main, tr("Warning"), tr("ROM file not found."));
        if (zip) cleanTemp();
        return;
    }

    romFile.open(QIODevice::ReadOnly);
    QByteArray romCheck = romFile.read(4);
    romFile.close();

    if (romCheck.toHex() != "80371240" && romCheck.toHex() != "37804012") {
        QMessageBox::warning(main, tr("Warning"), tr("Not a valid ROM File."));
        if (zip) cleanTemp();
        return;
    }


    QStringList args;

    if (SETTINGS.value("saveoptions", "").toString() == "true")
        args << "--saveoptions";
    else
        args << "--nosaveoptions";

    if (dataPath != "" && dataDir.exists())
        args << "--datadir" << dataPath;
    if (configPath != "" && configDir.exists())
        args << "--configdir" << configPath;
    if (pluginPath != "" && pluginDir.exists())
        args << "--plugindir" << pluginPath;

    if (emuMode != "")
        args << "--emumode" << emuMode;

    if (SETTINGS.value("Graphics/osd", "").toString() == "true")
        args << "--osd";
    else
        args << "--noosd";

    if (SETTINGS.value("Graphics/fullscreen", "").toString() == "true")
        args << "--fullscreen";
    else
        args << "--windowed";

    if (resolution != "")
        args << "--resolution" << resolution;

    if (videoPlugin != "")
        args << "--gfx" << videoPlugin;
    if (audioPlugin != "")
        args << "--audio" << audioPlugin;
    if (inputPlugin != "")
        args << "--input" << inputPlugin;
    if (rspPlugin != "")
        args << "--rsp" << rspPlugin;

    args << completeRomPath;

    main->toggleMenus(false);

    emulatorProc = new QProcess(this);
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(readOutput()));
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(emitFinished()));
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    if (zip)
        connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(cleanTemp()));

    emulatorProc->setWorkingDirectory(QFileInfo(mupen64File).dir().canonicalPath());
    emulatorProc->setProcessChannelMode(QProcess::MergedChannels);
    emulatorProc->start(mupen64Path, args);
}



void EmulatorHandler::stopEmulator()
{
    emulatorProc->terminate();
}
