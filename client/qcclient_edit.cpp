/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "qcclient_edit.h"
#include "qcclient_button.h"

QccEdit :: QccEdit(QccMainWindow *_parent) :
  QWidget()
{
	int y,m,d;

	QLabel *lbl;

	parent = _parent;
	tab_parent = 0;

	gl = new QGridLayout(this);

	date = parent->curr;
	date.getDate(&y,&m,&d);

	lbl = new QLabel(tr("Event Date: "));
	gl->addWidget(lbl, 0,0,1,1);

	lbl_date = new QLabel(QString("%1/%2/%3").arg(d).arg(m).arg(y));
	gl->addWidget(lbl_date, 0,1,1,1);

	time_from = new QLineEdit();
	connect(time_from, SIGNAL(textChanged(const QString &)), this, SLOT(handle_date_time_changed(const QString &)));
	connect(time_from, SIGNAL(textChanged(const QString &)), this, SLOT(handle_dirty()));

	time_to = new QLineEdit();
	connect(time_to, SIGNAL(textChanged(const QString &)), this, SLOT(handle_dirty()));

	lbl = new QLabel(tr("Event start: "));
	gl->addWidget(lbl, 1,0,1,1);
	gl->addWidget(time_from, 1,1,1,1);

	lbl = new QLabel(tr("Event stop: "));
	gl->addWidget(lbl, 1,2,1,1);
	gl->addWidget(time_to, 1,3,1,1);

	lbl = new QLabel(tr("Attendees: "));
	gl->addWidget(lbl, 0,2,1,1);

	user = new QLineEdit();
	connect(user, SIGNAL(textChanged(const QString &)), this, SLOT(handle_dirty()));
	gl->addWidget(user, 0,3,1,1);

	event = new QTextEdit();
	connect(event, SIGNAL(textChanged()), this, SLOT(handle_dirty()));
	gl->addWidget(event, 2,0,1,4);

	status = 0;
	id = -1;
}

QccEdit :: ~QccEdit()
{

}

void
QccEdit :: handle_date_time_changed(const QString &desc)
{
	tab_parent->setTabText(tab_parent->indexOf(this), desc);
}

QString
QccEdit :: tabString() const
{
	return (time_from->text());
}

int
QccEdit :: compare(const QccEdit &other) const
{
	if (date > other.date)
		return (1);
	if (date < other.date)
		return (-1);
	if (time_from->text() > other.time_from->text())
		return (1);
	if (time_from->text() < other.time_from->text())
		return (-1);
	if (time_to->text() > other.time_to->text())
		return (1);
	if (time_to->text() < other.time_to->text())
		return (-1);
	if (user->text() > other.user->text())
		return (1);
	if (user->text() < other.user->text())
		return (-1);
	if (id > other.id)
		return (1);
	if (id < other.id)
		return (-1);
	return (0);
}

void
QccEdit :: exportData(QSettings *setting) const
{
	setting->setValue("date", date.toString("yyyyMMdd"));
	setting->setValue("from", time_from->text());
	setting->setValue("to", time_to->text());
	setting->setValue("event", event->toPlainText());
	setting->setValue("user", user->text());
	setting->setValue("status", status);
	setting->setValue("id", id);
}

int
QccEdit :: validData(QSettings *setting) const
{
	return (setting->contains("date") && setting->contains("from") &&
	    setting->contains("to") && setting->contains("event") &&
	    setting->contains("user") && setting->contains("status") &&
	    setting->contains("id"));
}

void
QccEdit :: importData(QSettings *setting)
{
	int d,m,y;

	date = QDate::fromString(setting->value("date").toString(), "yyyyMMdd");
	date.getDate(&y,&m,&d);

	lbl_date->setText(QString("%1/%2/%3").arg(d).arg(m).arg(y));
	time_from->setText(setting->value("from").toString());
	time_to->setText(setting->value("to").toString());
	event->setText(setting->value("event").toString());
	user->setText(setting->value("user").toString());
	status = setting->value("status").toInt();
	id = setting->value("id").toInt();
}

void
QccEdit :: handle_dirty()
{
	status |= ST_DIRTY;
}
