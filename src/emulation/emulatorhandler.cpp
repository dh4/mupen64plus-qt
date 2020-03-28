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

#include "emulatorhandler.h"

#include "../global.h"
#include "../common.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>

#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>


EmulatorHandler::EmulatorHandler(QWidget *parent) : QObject(parent)
{
    this->parent = parent;

    lastOutput = "";
}

void EmulatorHandler::checkStatus(int status, QProcess::ExitStatus exitStatus)
{
    if (status != 0 || exitStatus != QProcess::NormalExit) {
        QMessageBox exitDialog(parent);
        exitDialog.setWindowTitle(tr("Warning"));
        exitDialog.setText(tr("<ParentName> quit unexpectedly. Check the log for more information.")
                           .replace("<ParentName>",ParentName));
        exitDialog.setIcon(QMessageBox::Warning);
        exitDialog.addButton(QMessageBox::Ok);
        exitDialog.addButton(tr("View Log"), QMessageBox::HelpRole);

        int ret = exitDialog.exec();
        if (ret == 0) emit showLog();
    }
}


void EmulatorHandler::cleanTemp()
{
    QFile::remove(QDir::tempPath() + "/" + AppNameLower + "-" + qgetenv("USER") + "/temp.z64");
}


void EmulatorHandler::emitFinished()
{
    emit finished();
}


QStringList EmulatorHandler::parseArgString(QString argString)
{
    QStringList result;
    QString arg;
    bool inQuote = false;
    bool inApos = false;

    for (int i = 0; i < argString.size(); i++)
    {
        // Check if inside of a quote
        if (argString.at(i) == QLatin1Char('"')) {
            inQuote = !inQuote;

            // Only continue if outside of both quotes and apostrophes
            if (arg.isEmpty() || (!inQuote && !inApos)) continue;
        }

        // Same check for apostrophes
        if (argString.at(i) == QLatin1Char('\'')) {
            inApos = !inApos;
            if (arg.isEmpty() || (!inQuote && !inApos)) continue;
        }

        if (!inQuote && !inApos && argString.at(i).isSpace()) {
            if (!arg.isEmpty()) {
                result += arg;
                arg.clear();
            }
        } else
            arg += argString.at(i);
    }

    if (!arg.isEmpty())
        result += arg;

    return result;
}


void EmulatorHandler::readOutput()
{
    QString output = emulatorProc->readAllStandardOutput();
    if (output != "")
        lastOutput.append(output);
    else
        lastOutput.append("\n" + tr("There was no output. If you were expecting Mupen64Plus to run, try copying")
                          + "\n" + tr("the above command into a command prompt to see if you get an error."));
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

        QString tempDir = QDir::tempPath() + "/" + AppNameLower + "-" + qgetenv("USER");
        QDir().mkpath(tempDir);
        completeRomPath = tempDir + "/temp.n64";

        QFile tempRom(completeRomPath);
        tempRom.open(QIODevice::WriteOnly);
        tempRom.write(romData);
        tempRom.close();
    } else
        completeRomPath = romDir.absoluteFilePath(romFileName);

    QString emulatorPath = SETTINGS.value("Paths/mupen64plus", "").toString();
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

    QFile emulatorFile(emulatorPath);
    QFile romFile(completeRomPath);

    QString gameVideoPlugin = SETTINGS.value(romFileName+"/video", "").toString();
    QString gameAudioPlugin = SETTINGS.value(romFileName+"/audio", "").toString();
    QString gameInputPlugin = SETTINGS.value(romFileName+"/input", "").toString();
    QString gameRSPPlugin = SETTINGS.value(romFileName+"/rsp", "").toString();

    QString gameConfigPath = SETTINGS.value(romFileName+"/config", "").toString();
    QDir gameConfigDir(gameConfigPath);


    //Sanity checks
    if(!emulatorFile.exists() || QFileInfo(emulatorFile).isDir() || !QFileInfo(emulatorFile).isExecutable()) {
        QMessageBox::warning(parent, tr("Warning"),
                             tr("<ParentName> executable not found.").replace("<ParentName>",ParentName));
        if (zip) cleanTemp();
        return;
    }

    if(!romFile.exists() || QFileInfo(romFile).isDir()) {
        QMessageBox::warning(parent, tr("Warning"), tr("ROM file not found."));
        if (zip) cleanTemp();
        return;
    }

    romFile.open(QIODevice::ReadOnly);
    QByteArray romCheck = romFile.read(4);
    romFile.close();

    if (romCheck.toHex() != "80371240" && romCheck.toHex() != "37804012") {
        QMessageBox::warning(parent, tr("Warning"), tr("Not a valid ROM File."));
        if (zip) cleanTemp();
        return;
    }


    QStringList args;

    if (SETTINGS.value("saveoptions", "").toString() != "true")
        args << "--nosaveoptions";

    if (dataPath != "" && dataDir.exists())
        args << "--datadir" << dataPath;
    if (gameConfigPath != "" && gameConfigDir.exists())
        args << "--configdir" << gameConfigPath;
    else if (configPath != "" && configDir.exists())
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

    if (gameVideoPlugin != "")
        args << "--gfx" << gameVideoPlugin;
    else if (videoPlugin != "")
        args << "--gfx" << videoPlugin;
    if (gameAudioPlugin != "")
        args << "--audio" << gameAudioPlugin;
    else if (audioPlugin != "")
        args << "--audio" << audioPlugin;
    if (gameInputPlugin != "")
        args << "--input" << gameInputPlugin;
    else if (inputPlugin != "")
        args << "--input" << inputPlugin;
    if (gameRSPPlugin != "")
        args << "--rsp" << gameRSPPlugin;
    else if (rspPlugin != "")
        args << "--rsp" << rspPlugin;

    QString otherParameters = SETTINGS.value("Other/parameters", "").toString();
    if (otherParameters != "")
        args.append(parseArgString(otherParameters));

    QString gameParameters = SETTINGS.value(romFileName+"/parameters").toString();
    if (gameParameters != "")
        args.append(parseArgString(gameParameters));

    args << completeRomPath;

    emulatorProc = new QProcess(this);
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(readOutput()));
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(emitFinished()));
    connect(emulatorProc, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(checkStatus(int, QProcess::ExitStatus)));

    if (zip)
        connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(cleanTemp()));

    // GLideN64 workaround. Can be removed if workaround is no longer needed
    // See: https://github.com/gonetz/GLideN64/issues/454#issuecomment-126853972
    if (SETTINGS.value("Other/forcegl33", "").toString() == "true") {
        QProcessEnvironment emulatorEnv = QProcessEnvironment::systemEnvironment();
        emulatorEnv.insert("MESA_GL_VERSION_OVERRIDE", "3.3COMPAT");
        emulatorProc->setProcessEnvironment(emulatorEnv);
    }

    emulatorProc->setWorkingDirectory(QFileInfo(emulatorFile).dir().canonicalPath());
    emulatorProc->setProcessChannelMode(QProcess::MergedChannels);
    emulatorProc->start(emulatorPath, args);

    //Add command to log
    QString executable = emulatorPath;
    if (executable.contains(" "))
        executable = '"' + executable + '"';

    QString argString;
    foreach(QString arg, args)
    {
        if (arg.contains(" "))
            argString += " \"" + arg + "\"";
        else
            argString += " " + arg;
    }

    lastOutput = executable + argString + "\n\n";


    emit started();
}


void EmulatorHandler::stopEmulator()
{
    emulatorProc->terminate();
}
