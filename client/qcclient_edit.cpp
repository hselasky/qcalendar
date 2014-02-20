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
	unsigned x;
	int y,m,d;

	QLabel *lbl;

	parent = _parent;
	tab_parent = 0;

	gl = new QGridLayout(this);

	date = parent->curr;
	date.getDate(&y,&m,&d);

	lbl = new QLabel(tr("Event Date: "));
	gl->addWidget(lbl, 0,0,1,1);

	lbl = new QLabel(tr("%1/%2/%3").arg(d).arg(m).arg(y));
	gl->addWidget(lbl, 0,1,1,1);

	time_from = new QTimeEdit();
	connect(time_from, SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(handle_date_time_changed(const QDateTime &)));
	connect(time_from, SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(handle_dirty()));

	time_to = new QTimeEdit();
	connect(time_to, SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(handle_dirty()));

	lbl = new QLabel(tr("Event start: "));
	gl->addWidget(lbl, 1,0,1,1);
	gl->addWidget(time_from, 1,1,1,1);

	lbl = new QLabel(tr("Event stop: "));
	gl->addWidget(lbl, 2,0,1,1);
	gl->addWidget(time_to, 2,1,1,1);

	user = new QListWidget();
	connect(user, SIGNAL(currentRowChanged(int)), this, SLOT(handle_dirty()));

	for (x = 0; x != QCC_USER_NUM; x++) {
		new QListWidgetItem(QString(users[x]), user);
	}

	gl->addWidget(user, 1,2,2,1);

	event = new QTextEdit();
	connect(event, SIGNAL(textChanged()), this, SLOT(handle_dirty()));
	gl->addWidget(event, 3,0,1,3);

	status = 0;
	id = -1;
}

QccEdit :: ~QccEdit()
{

}

void
QccEdit :: handle_date_time_changed(const QDateTime &date)
{
	tab_parent->setTabText(tab_parent->indexOf(this), tabString());
}

QString
QccEdit :: tabString() const
{
	QTime tf = time_from->time();
	int fh = tf.hour();
	int fm = tf.minute();

	return (QString("%1%2:%3%4").arg(fh/10).arg(fh%10).arg(fm/10).arg(fm%10));
}

int
QccEdit :: compare(const QccEdit &other) const
{
	if (date > other.date)
		return (1);
	if (date < other.date)
		return (-1);
	if (time_from->time() > other.time_from->time())
		return (1);
	if (time_from->time() < other.time_from->time())
		return (-1);
	if (time_to->time() > other.time_to->time())
		return (1);
	if (time_to->time() < other.time_to->time())
		return (-1);
	if (user->currentRow() > other.user->currentRow())
		return (1);
	if (user->currentRow() < other.user->currentRow())
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
	setting->setValue("date", date.toString());
	setting->setValue("from", time_from->time().toString());
	setting->setValue("to", time_to->time().toString());
	setting->setValue("event", event->toPlainText());
	setting->setValue("user", user->currentRow());
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
	date = QDate::fromString(setting->value("date").toString());
	time_from->setTime(QTime::fromString(setting->value("from").toString()));
	time_to->setTime(QTime::fromString(setting->value("to").toString()));
	event->setText(setting->value("event").toString());
	user->setCurrentRow(setting->value("user").toInt());
	status = setting->value("status").toInt();
	id = setting->value("id").toInt();
}

void
QccEdit :: handle_dirty()
{
	status |= ST_DIRTY;
}
