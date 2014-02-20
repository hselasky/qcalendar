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

#ifndef _QCCLIENT_H_
#define	_QCCLIENT_H_

#include <QApplication>
#include <QDate>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <QObject>
#include <QSizePolicy>
#include <QScrollArea>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QTimer>
#include <QSettings>
#include <QTcpSocket>
#include <QFile>
#include <QDir>

#include <sys/queue.h>

#define	QCC_YEAR_START 2014
#define	QCC_YEAR_STOP 2015
#define	QCC_YEAR_NUM (QCC_YEAR_STOP + 1 - QCC_YEAR_START)
#define	QCC_MONTH_NUM_W 4
#define	QCC_MONTH_NUM_H 3
#define	QCC_WEEK_NUM_W 8
#define	QCC_WEEK_NUM_H 7
#ifndef QCC_USER_NUM
#define	QCC_USER_NUM 3
#endif
#ifndef QCC_USER_LIST
#define	QCC_USER_LIST "A", "B", "C"
#endif
#define	QCC_EVENT_NUM 24
#define	QCC_EVENT_DELTA_MAX (24 * 365)
#define	QCC_EVENT_SIZE_MAX (16 * 1024)
#define	QCC_HOST_PORT 8200
#ifndef QCC_HOST_ADDR
#define QCC_HOST_ADDR "127.0.0.1"
#endif
#ifndef QCC_EVENT_SPARE_NUM
#define	QCC_EVENT_SPARE_NUM 32
#endif

class QccMainWindow;
class QccButton;
class QccDay;
class QccEdit;
class QccMonth;
class QccWeek;
class QccYear;
class QccDatabase;

class QccEdit;
typedef struct
#define struct 
TAILQ_ENTRY(QccEdit) QccEditEntryT;
#undef struct
typedef struct
#define struct
TAILQ_HEAD(QccEditHead, QccEdit) QccEditHeadT;
#undef struct

class QccMainWindow : public QTabWidget
{
	Q_OBJECT;

	enum {
		INDEX_YEAR,
		INDEX_MONTH,
		INDEX_WEEK,
		INDEX_DAY,
		INDEX_MAX,
	};
public:
	QccMainWindow();
	~QccMainWindow();

	void tabRefresh();

	QccYear *year;
	QccMonth *month;
	QccWeek *week;
	QccDay *day;
	QccDatabase *db;

	uint8_t usage[QCC_YEAR_NUM][16][32];

	QDate curr;

public slots:
	void handle_select_month(int);
	void handle_select_day(int);
	void handle_select_year(int);
	void handle_failure(const QString &);
};

extern const char *months[QCC_MONTH_NUM_H][QCC_MONTH_NUM_W];
extern const char *weeks[QCC_WEEK_NUM_W];
extern const char *users[QCC_USER_NUM];
extern QString QccDatabasePath;

#endif			/* _QCCLIENT_H_ */
