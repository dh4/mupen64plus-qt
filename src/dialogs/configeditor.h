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

#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QDialog>
#include <QFile>
#include <QRegularExpression>
#include <QSyntaxHighlighter>

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QTextCharFormat;
class QTextEdit;
class KeyCodes;


class ConfigHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ConfigHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<Rule> rules;

    QTextCharFormat headerFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat separatorFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat commentFormat;
};


class ConfigEditor : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigEditor(QString configFile, QWidget *parent = 0);

private:
    QDialogButtonBox *codesButtonBox;
    QDialogButtonBox *editorButtonBox;
    QFile config;
    QGridLayout *controlsLayout;
    QGridLayout *editorLayout;
    QLabel *docsLink;
    QTextEdit *editorArea;
    QWidget *controls;

    ConfigHighlighter *highlighter;

    KeyCodes *keyCodes = nullptr;

private slots:
    void openKeyCodes();
    void saveConfig();

};

#endif // CONFIGEDITOR_H
