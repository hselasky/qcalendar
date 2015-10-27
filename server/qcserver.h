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

#ifndef _QCSERVER_H_
#define	_QCSERVER_H_

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>

#define	QCS_YEAR_START 2015
#define	QCS_YEAR_STOP 2016
#define	QCS_YEAR_NUM (QCS_YEAR_STOP + 1 - QCS_YEAR_START)
#define	QCS_EVENT_DELTA_MAX (24 * 365)
#define	QCS_EVENT_SIZE_MAX (16 * 1024)
#ifndef QCS_HOST_PORT
#define	QCS_HOST_PORT 8200
#endif
#ifndef QCS_HOST_ADDR
#define	QCS_HOST_ADDR "0.0.0.0"
#endif
#ifndef QCS_ROOT_DIR
#define	QCS_ROOT_DIR "/qcserver.db"
#endif

class QcsMain : public QObject
{
	Q_OBJECT;

public:
	QcsMain();
	~QcsMain();

	int max_event_id[QCS_YEAR_NUM];

	uint64_t time_event[QCS_YEAR_NUM][QCS_EVENT_DELTA_MAX];

	QTcpServer *tcp;

public slots:
	void handle_connection();
};

#endif			/* _QCSERVER_H_ */
