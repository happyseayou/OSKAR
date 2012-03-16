/*
 * Copyright (c) 2011, The University of Oxford
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

#include "widgets/oskar_SettingsView.h"
#include "widgets/oskar_SettingsModel.h"
#include <QtGui/QScrollBar>

oskar_SettingsView::oskar_SettingsView(QWidget* parent)
: QTreeView(parent)
{
    connect(this, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(resizeAfterExpand(const QModelIndex&)));
    setAlternatingRowColors(false);
    setUniformRowHeights(true);
}

void oskar_SettingsView::restoreExpanded()
{
    QSettings settings;
    QStringList expanded = settings.value("settings_view/expanded_items").
            toStringList();
    saveRestoreExpanded(QModelIndex(), expanded, 1);
}

void oskar_SettingsView::restorePosition()
{
    QSettings settings;
    QScrollBar* verticalScroll = verticalScrollBar();
    verticalScroll->setValue(settings.value("settings_view/position").toInt());
}

void oskar_SettingsView::saveExpanded()
{
    QSettings settings;
    QStringList expanded;
    saveRestoreExpanded(QModelIndex(), expanded, 0);
    settings.setValue("settings_view/expanded_items", expanded);
}

void oskar_SettingsView::savePosition()
{
    QSettings settings;
    settings.setValue("settings_view/position", verticalScrollBar()->value());
}

void oskar_SettingsView::showFirstLevel()
{
    expandToDepth(0);
}

void oskar_SettingsView::resizeAfterExpand(const QModelIndex& /*index*/)
{
    resizeColumnToContents(0);
    update();
}

void oskar_SettingsView::saveRestoreExpanded(const QModelIndex& parent,
        QStringList& list, int restore)
{
    for (int i = 0; i < model()->rowCount(parent); ++i)
    {
        QModelIndex idx = model()->index(i, 0, parent);
        QString key = idx.data(oskar_SettingsModel::KeyRole).toString();
        if (restore)
        {
            if (list.contains(key)) expand(idx);
        }
        else
        {
            if (isExpanded(idx)) list.append(key);
        }

        // Recursion.
        if (model()->rowCount(idx) > 0)
            saveRestoreExpanded(idx, list, restore);
    }
}
