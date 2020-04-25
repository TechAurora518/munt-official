// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#ifndef GULDEN_CLICKABLE_LABEL_H
#define GULDEN_CLICKABLE_LABEL_H

#include <QObject>
#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    ClickableLabel( QWidget* parent );
    ~ClickableLabel(){}
    void setChecked(bool checked);
    bool isChecked();

    void forceStyleRefresh();

Q_SIGNALS:
    void clicked();

public Q_SLOTS:

protected:
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
    bool mouseIn;
};

#endif
