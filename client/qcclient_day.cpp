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

#include "qcclient_day.h"
#include "qcclient_button.h"
#include "qcclient_edit.h"
#include "qcclient_database.h"
#include "qcclient_week.h"

const char *users[QCC_USER_NUM] = { QCC_USER_LIST };

QccDay :: QccDay(QccMainWindow *_parent) :
  QWidget()
{
	unsigned x;

	parent = _parent;

	event_pasteboard = -1;
	event_year = -1;

	watchdog = new QTimer();
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_remove_timeout()));

	tab = new QTabWidget();

	gl = new QGridLayout(this);
	gl->setSpacing(0);

	gl->addWidget(tab,0,0,1,QCC_USER_NUM+2);

	status = new QLabel(tr("OK"));
	gl->addWidget(status, 1,0,1,1);

	for (x = 0; x != QCC_USER_NUM; x++) {
		but_new[x] = new QccButton(tr("New %1 event").arg(QString(users[x])), x);
		connect(but_new[x], SIGNAL(released(int)), this, SLOT(handle_new(int)));
		gl->addWidget(but_new[x],1,x+1,1,1);
	}

	sync = new QccButton("Sync events", 0);
	gl->addWidget(sync, 2,0,1,1);
	connect(sync, SIGNAL(pressed(int)), this, SLOT(handle_sync()));

	remove = new QccButton("Remove event", 0);
	gl->addWidget(remove, 2,1,1,1);
	connect(remove, SIGNAL(pressed(int)), this, SLOT(handle_remove()));

	today = new QccButton("Show today", 0);
	gl->addWidget(today, 2,2,1,1);
	connect(today, SIGNAL(pressed(int)), this, SLOT(handle_today(int)));

	tomorrow = new QccButton("Show tomorrow", 1);
	gl->addWidget(tomorrow, 2,3,1,1);
	connect(tomorrow, SIGNAL(pressed(int)), this, SLOT(handle_today(int)));

	cut = new QccButton("Event Cut", 0);
	gl->addWidget(cut, 1,4,1,1);
	connect(cut, SIGNAL(pressed(int)), this, SLOT(handle_cut()));

	paste = new QccButton("Event Paste", 0);
	gl->addWidget(paste, 2,4,1,1);
	connect(paste, SIGNAL(pressed(int)), this, SLOT(handle_paste()));

	paste->setEnabled(0);
}

QccDay :: ~QccDay()
{
}

void
QccDay :: handle_new(int id)
{
	QccEdit *pe = new QccEdit(parent);

	pe->user->setCurrentRow(id);

	if (parent->db->add(pe) == 0)
		return;

	tab->addTab(pe, pe->tabString());

	tab->setCurrentWidget(pe);
}

void
QccDay :: handle_remove()
{
	watchdog->start(4000);
}

void
QccDay :: handle_remove_timeout()
{
	if (remove->isDown() == 0)
		return;

	QccEdit *pe = (QccEdit *)tab->currentWidget();
	if (pe == 0)
		return;
	tab->removeTab(tab->currentIndex());

	pe->status |= ST_DIRTY | ST_REMOVED;

	parent->db->sync();
	parent->week->syncText();
	parent->db->show();
}

void
QccDay :: handle_sync()
{
	parent->db->sync();
	parent->week->syncText();
	parent->db->show();
}

void
QccDay :: handle_today(int offset)
{
	parent->curr =  QDate::currentDate().addDays(offset);

	parent->tabRefresh();

	parent->week->syncText();
	parent->db->show();
}

void
QccDay :: handle_cut()
{
	QccEdit *pe = (QccEdit *)tab->currentWidget();

	if (pe == 0) {
		event_pasteboard = -1;
	} else {
		event_pasteboard = pe->id;
		event_year = parent->curr.year();
	}

	paste->setEnabled(1);
}

void
QccDay :: handle_paste()
{
	QccEdit *pe;

	if (event_pasteboard < 0)
		return;
	if (event_year != parent->curr.year())
		return;

	TAILQ_FOREACH(pe, &parent->db->head[event_year - QCC_YEAR_START], entry) {
		if (pe->id == event_pasteboard) {
			pe->date = parent->curr;
			pe->status |= ST_DIRTY;
			break;
		}
	}

	event_pasteboard = -1;
	event_year = -1;

	parent->tabRefresh();
 	parent->week->syncText();
	parent->db->show();

	paste->setEnabled(0);
}
