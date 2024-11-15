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

#include "configeditor.h"

#include "../global.h"

#include "keycodesdialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QPalette>
#include <QTextEdit>


ConfigEditor::ConfigEditor(QString configFile, QWidget *parent) : QDialog(parent)
{
    config.setFileName(configFile);

    setWindowTitle(tr("<ParentName> Config Editor").replace("<ParentName>",ParentName));
    setMinimumSize(600, 400);
    setGeometry(0, 0, 900, 700);

    editorLayout = new QGridLayout(this);
    editorLayout->setContentsMargins(5, 10, 5, 10);

    editorArea = new QTextEdit(this);
    editorArea->setWordWrapMode(QTextOption::NoWrap);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editorArea->setFont(font);

    highlighter = new ConfigHighlighter(editorArea->document());

    config.open(QIODevice::ReadOnly);
    editorArea->setPlainText(config.readAll());
    config.close();

    controls = new QWidget(this);
    controlsLayout = new QGridLayout(controls);
    controlsLayout->setContentsMargins(0, 0, 0, 0);

    codesButtonBox = new QDialogButtonBox(Qt::Horizontal, controls);
    codesButtonBox->addButton(tr("Key Codes Reference"), QDialogButtonBox::HelpRole);

    QString docsLinkString = "<a href=\"https://mupen64plus.org/docs/\">"
                           + tr("Mupen64Plus Documentation") + "</a>";

    docsLink = new QLabel(docsLinkString, controls);
    docsLink->setOpenExternalLinks(true);

    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);

    editorButtonBox = new QDialogButtonBox(Qt::Horizontal, controls);
    editorButtonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    editorButtonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

    controlsLayout->addWidget(codesButtonBox, 0, 0);
    controlsLayout->addWidget(docsLink, 0, 1);
    controlsLayout->addItem(spacer, 0, 2);
    controlsLayout->addWidget(editorButtonBox, 0, 3);
    controls->setLayout(controlsLayout);

    editorLayout->addWidget(editorArea, 0, 0);
    editorLayout->addWidget(controls, 1, 0);

    connect(codesButtonBox, SIGNAL(helpRequested()), this, SLOT(openKeyCodes()));
    connect(editorButtonBox, SIGNAL(accepted()), this, SLOT(saveConfig()));
    connect(editorButtonBox, SIGNAL(rejected()), this, SLOT(close()));

    setLayout(editorLayout);
}


bool isDarkTheme() {
    QPalette palette = QApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);

    return windowColor.lightness() < 128; // 128 is the midpoint of the lightness scale
}


void ConfigEditor::openKeyCodes()
{
    if (!keyCodes) {
        keyCodes = new KeyCodes(this);
    }

    keyCodes->show();
    keyCodes->raise();
    keyCodes->activateWindow();
}


void ConfigEditor::saveConfig()
{
    config.open(QIODevice::WriteOnly);
    config.write(editorArea->toPlainText().toUtf8().data());
    config.close();

    close();
}


ConfigHighlighter::ConfigHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    Rule rule;

    headerFormat.setFontWeight(QFont::Bold);
    if (isDarkTheme())
        headerFormat.setForeground(Qt::magenta);
    else
        headerFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\[[A-Za-z0-9\\(\\)\\-\\W]*\\]");
    rule.format = headerFormat;
    rules.append(rule);

    if (isDarkTheme())
        variableFormat.setForeground(Qt::white);
    else
        variableFormat.setForeground(Qt::black);
    rule.pattern = QRegularExpression("[A-Za-z0-9_\\-\\s]*=");
    rule.format = variableFormat;
    rules.append(rule);

    if (isDarkTheme())
        valueFormat.setForeground(Qt::cyan);
    else
        valueFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("=[^\\n]+");
    rule.format = valueFormat;
    rules.append(rule);

    separatorFormat.setFontWeight(QFont::Bold);
    if (isDarkTheme())
        separatorFormat.setForeground(Qt::white);
    else
        separatorFormat.setForeground(Qt::black);
    rule.pattern = QRegularExpression("=");
    rule.format = separatorFormat;
    rules.append(rule);

    if (isDarkTheme())
        quotationFormat.setForeground(Qt::green);
    else
        quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    rules.append(rule);

    if (isDarkTheme())
        commentFormat.setForeground(Qt::darkGray);
    else
        commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("#[^\\n]*");
    rule.format = commentFormat;
    rules.append(rule);
}


void ConfigHighlighter::highlightBlock(const QString &text)
{
    foreach (const Rule &rule, rules) {
        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatchIterator matches = expression.globalMatch(text);
        while(matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            if (match.hasMatch()) {
                int index = match.capturedStart();
                int length = match.capturedEnd() - index;
                setFormat(index, length, rule.format);
            }
        }
    }
}
