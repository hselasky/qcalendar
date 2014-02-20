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

#include "qcclient.h"
#include "qcclient_day.h"
#include "qcclient_month.h"
#include "qcclient_year.h"
#include "qcclient_week.h"
#include "qcclient_database.h"

Q_DECL_EXPORT
QString QccDatabasePath;

QccMainWindow :: QccMainWindow() :
  QTabWidget()
{
	curr.setDate(QCC_YEAR_START, 1, 1);

	memset(usage, 0, sizeof(usage));

	connect(&timer, SIGNAL(timeout()), this, SLOT(handle_timeout()));

	day = new QccDay(this);
	month = new QccMonth(this);
	week = new QccWeek(this);
	year = new QccYear(this);
	db = new QccDatabase(this);

	addTab(year, tr("Year - %1").arg(QCC_YEAR_START));
	addTab(month, tr("Month - JAN"));
	addTab(week, tr("Week - %1").arg(curr.weekNumber()));
	addTab(day, tr("Day - 1 - %1").arg(QString(weeks[curr.dayOfWeek()])));

	db->sort();

	day->handle_today(0);
	day->handle_sync();

	setWindowIcon(QIcon(QString(":/qcclient_48x48.png")));
}

Q_DECL_EXPORT
QccMainWindow :: ~QccMainWindow()
{
}

Q_DECL_EXPORT void
QccMainWindow :: handle_select_month(int mnew)
{
	int y,m,d;

	if (mnew < 1)
		return;

	curr.getDate(&y,&m,&d);
	curr.setDate(y,mnew,1);

	tabRefresh();

	week->syncText();

	if (currentIndex() == INDEX_MONTH)
		setCurrentIndex(INDEX_WEEK);
}

Q_DECL_EXPORT void
QccMainWindow :: tabRefresh()
{
	setTabText(INDEX_YEAR, tr("Year - %1").arg(curr.year()));
	setTabText(INDEX_MONTH, tr("Month - %1")
	    .arg(QString(months[(curr.month() - 1) /
	    QCC_MONTH_NUM_W][(curr.month() - 1) % QCC_MONTH_NUM_W])));
	setTabText(INDEX_WEEK, tr("Week - %1").arg(curr.weekNumber()));
	setTabText(INDEX_DAY, tr("Day - %1 - %2").arg(curr.day())
	    .arg(QString(weeks[curr.dayOfWeek()])));
}

Q_DECL_EXPORT void
QccMainWindow :: handle_select_day(int dnew)
{
	int y,m,d;

	if (dnew < 1)
		return;

	curr.getDate(&y,&m,&d);
	curr.setDate(y,m,dnew);

	tabRefresh();

	week->syncText();

	if (currentIndex() == INDEX_WEEK) {
		db->show();
		setCurrentIndex(INDEX_DAY);
	}
}

Q_DECL_EXPORT void
QccMainWindow :: handle_select_year(int ynew)
{
	int y,m,d;

	if (ynew < 1)
		return;

	curr.getDate(&y,&m,&d);
	curr.setDate(ynew,1,1);

	tabRefresh();

	week->syncText();

	if (currentIndex() == INDEX_YEAR)
		setCurrentIndex(INDEX_MONTH);
}

Q_DECL_EXPORT void
QccMainWindow :: handle_failure(const QString &desc)
{
	day->status->setText(desc);
	timer.start(4000);
}

Q_DECL_EXPORT void
QccMainWindow :: handle_timeout()
{
	day->status->setText(QString());
}

Q_DECL_EXPORT int
main(int argc, char **argv)
{
  	QApplication app(argc, argv);

	QDir dir;

	QccDatabasePath = dir.homePath() + QString("/Documents/qcclient.db");

	dir.mkpath(QccDatabasePath);

	QccMainWindow *mw = new QccMainWindow();

	mw->show();

	return (app.exec());
}
