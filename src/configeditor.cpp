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

#include "configeditor.h"
#include "global.h"


ConfigEditor::ConfigEditor(QString configFile, QWidget *parent) : QDialog(parent)
{
    config.setFileName(configFile);

    setWindowTitle(tr("Mupen64Plus Config Editor"));
    setMinimumSize(600, 400);

    editorLayout = new QGridLayout(this);
    editorLayout->setContentsMargins(5, 10, 5, 10);

    editorArea = new QTextEdit(this);
    editorArea->setWordWrapMode(QTextOption::NoWrap);

    QFont font;
#ifdef Q_OS_LINUX
    font.setFamily("Monospace");
    font.setPointSize(9);
#else
    font.setFamily("Courier");
    font.setPointSize(10);
#endif
    font.setFixedPitch(true);
    editorArea->setFont(font);

    highlighter = new ConfigHighlighter(editorArea->document());

    config.open(QIODevice::ReadOnly);
    editorArea->setPlainText(config.readAll());
    config.close();

    editorButtonBox = new QDialogButtonBox(Qt::Horizontal, this);
    editorButtonBox->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    editorButtonBox->addButton(QDialogButtonBox::Cancel);

    editorLayout->addWidget(editorArea, 0, 0);
    editorLayout->addWidget(editorButtonBox, 1, 0);

    connect(editorButtonBox, SIGNAL(accepted()), this, SLOT(saveConfig()));
    connect(editorButtonBox, SIGNAL(rejected()), this, SLOT(close()));

    setLayout(editorLayout);
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
    headerFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\[[A-Za-z0-9\\(\\)-\\W]*\\]");
    rule.format = headerFormat;
    rules.append(rule);

    variableFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("[A-Za-z0-9_- ]*=");
    rule.format = variableFormat;
    rules.append(rule);

    valueFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("=[^\\n]+");
    rule.format = valueFormat;
    rules.append(rule);

    separatorFormat.setFontWeight(QFont::Bold);
    separatorFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("=");
    rule.format = separatorFormat;
    rules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    rules.append(rule);

    commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegExp("#[^\\n]*");
    rule.format = commentFormat;
    rules.append(rule);
}


void ConfigHighlighter::highlightBlock(const QString &text)
{
    foreach (const Rule &rule, rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}
