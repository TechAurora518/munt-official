// Copyright (c) 2016-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "clickablelabel.h"

#include <QVariant>
#include <QStyle>
#include <QShortcut>
#include <QKeySequence>

ClickableLabel::ClickableLabel( QWidget * parent )
: QLabel( parent )
{
    setContentsMargins(0, 0, 0, 0);
    setIndent(0);

    setProperty("hover", QVariant(false));
    setProperty("checked", QVariant(false));
    setProperty("pressed", QVariant(false));

    mouseIn = false;

    //QShortcut mnemonicShortcut = new QShortcut(QKeySequence::mnemonic(), this);
    //connect(mnemonicShortcut, SIGNAL(activated()), this, SIGNAL(clicked()));
}

void ClickableLabel::mousePressEvent( QMouseEvent * event ) 
{
    setProperty("pressed", QVariant(true));
    style()->unpolish(this);
    style()->polish(this);
}

void ClickableLabel::mouseReleaseEvent ( QMouseEvent * event )
{
    setProperty("pressed", QVariant(false));
    style()->unpolish(this);
    style()->polish(this);
    if (mouseIn)
    {
        Q_EMIT clicked();
    }
}

void ClickableLabel::enterEvent(QEvent * event)
{
    mouseIn = true;
    setProperty("hover", QVariant(true));
    style()->unpolish(this);
    style()->polish(this);
}

void ClickableLabel::leaveEvent(QEvent * event)
{
    mouseIn = false;
    setProperty("hover", QVariant(false));
    style()->unpolish(this);
    style()->polish(this);
}

void ClickableLabel::setChecked(bool checked)
{
    if (isChecked() != checked)
    {
        setProperty("checked", QVariant(checked));
        style()->unpolish(this);
        style()->polish(this);
    }
}

bool ClickableLabel::isChecked()
{
    return property("checked").toBool();
}
