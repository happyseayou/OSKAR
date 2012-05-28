/*
 * Copyright (c) 2012, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
 */

#ifndef OSKAR_RUN_DIALOG_H_
#define OSKAR_RUN_DIALOG_H_

/**
 * @file oskar_RunDialog.h
 */

#include <QtCore/QStringList>
#include <QtGui/QDialog>

class QAbstractButton;
class QDialogButtonBox;
class QCheckBox;
class QLabel;
class QTextEdit;
class oskar_SettingsModel;
class oskar_RunThread;

class oskar_RunDialog : public QDialog
{
    Q_OBJECT

public:
    oskar_RunDialog(oskar_SettingsModel* model, QWidget *parent);
    ~oskar_RunDialog();

    void start(const QString& binary_name,
            const QString& settings_file, QStringList outputs);

private slots:
    void appendOutput(QString output);
    void runFinished();
    void buttonClicked(QAbstractButton* button);

private:
    void run(int depth, QStringList outputFiles);

private:
    bool aborted_;
    QCheckBox* autoClose_;
    QTextEdit* display_;
    QLabel* labelText_;
    QLabel* labelCommand_;
    QLabel* labelSettingsFile_;
    QDialogButtonBox* buttons_;
    QAbstractButton* closeButton_;
    QAbstractButton* cancelButton_;
    QString binaryName_;
    QString settingsFile_;
    QStringList outputFiles_;
    oskar_RunThread* thread_;
};

#endif /* OSKAR_RUN_DIALOG_H_ */
