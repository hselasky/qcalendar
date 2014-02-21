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

#ifndef _QCCLIENT_EDIT_H_
#define	_QCCLIENT_EDIT_H_

#include "qcclient.h"

class QccEdit : public QWidget
{
	Q_OBJECT;

	QccMainWindow *parent;

public:
	QccEditEntryT entry;

	QccEdit(QccMainWindow *);
	~QccEdit();

	QString tabString() const;
	int compare(const QccEdit &) const;
	void exportData(QSettings *) const;
	int validData(QSettings *) const;
	void importData(QSettings *);
	void populate();

	QGridLayout *gl;
	QTabWidget *tab_parent;

	QDate date;

	QLineEdit *time_from;
	QLineEdit *time_to;
	QTextEdit *event;
	QLineEdit *user;
	QLabel *lbl_date;

	int status;
#define	ST_OK 0
#define	ST_DIRTY 1
#define	ST_REMOVED 2
	int id;

public slots:
	void handle_date_time_changed(const QString &);
	void handle_dirty();
};

#endif			/* _QCCLIENT_EDIT_H_ */
