/*-
 * Copyright (c) 2014-2016 Hans Petter Selasky. All rights reserved.
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

int qcc_year_start;
int qcc_year_stop;
QString qcc_user[QCC_USER_NUM];

static int
qcc_read_config(void)
{
	QTcpSocket *tcp;
	char buf[64];
	char *token;
	char *string;
	char buffer[1024];
	int len;
	int x;

	tcp = new QTcpSocket();

	tcp->connectToHost(QCC_HOST_ADDR, QCC_HOST_PORT);

	if (tcp->waitForConnected(4000) == 0) {
		delete tcp;
		return (1);
	}

	SNPRINTF(buf, sizeof(buf), "CONFIGURATION\n");
	tcp->write(buf, sizeof(buf));
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buffer)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buffer, sizeof(buffer));
	if (len != (int)sizeof(buffer))
		goto error;

	buffer[sizeof(buffer)-1] = 0;

	string = buffer;

	token = strsep(&string, "-");
	if (token == 0)
		goto error;
	qcc_year_start = atoi(token);

	token = strsep(&string, "-");
	if (token == 0)
		goto error;
	qcc_year_stop = atoi(token);

	for (x = 0; x != QCC_USER_NUM; x++) {
		token = strsep(&string, "-");
		if (token == 0)
			goto error;
		qcc_user[x] = QString::fromUtf8(token);
	}

	if ((qcc_year_start + QCC_YEAR_NUM - 1) != qcc_year_stop)
		goto error;

	tcp->close();
	delete tcp;
	return (0);

error:
	tcp->close();
	delete tcp;
	return (1);
}

Q_DECL_EXPORT
QString QccDatabasePath;

QccMainWindow :: QccMainWindow() :
  QTabWidget()
{
	curr.setDate(qcc_year_start, 1, 1);

	memset(usage, 0, sizeof(usage));

	connect(&timer, SIGNAL(timeout()), this, SLOT(handle_timeout()));

	day = new QccDay(this);
	month = new QccMonth(this);
	week = new QccWeek(this);
	year = new QccYear(this);
	db = new QccDatabase(this);

	addTab(year, tr("Year - %1").arg(qcc_year_start));
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

	if (qcc_read_config() != 0) {
		QMessageBox box;

		box.setText(QObject::tr("Cannot read configuration from server!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Information);
		box.setWindowIcon(QIcon(QString(":/qcclient_48x48.png")));
		box.setWindowTitle("QCalendarClient");
		box.exec();

		return (0);
	}
	
	QDir dir;

	QccDatabasePath = dir.homePath() + QString("/Documents/qcclient.db");

	dir.mkpath(QccDatabasePath);

	QccMainWindow *mw = new QccMainWindow();

	mw->show();

	return (app.exec());
}
