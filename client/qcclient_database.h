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

#ifndef _QCCDATABASE_H_
#define	_QCCDATABASE_H_

#include "qcclient.h"

class QccDatabase {
public:
	QccDatabase(QccMainWindow *);
	~QccDatabase();

	QTcpSocket *tcp;

	QccEditHeadT head[QCC_YEAR_NUM];

	QccMainWindow *parent;

	QccEdit *pullEventById(uint32_t, uint32_t, uint32_t = 0);
	void pushEventById(uint32_t, QccEdit *);
	int getHashes(uint32_t, int, QByteArray &);

	uint32_t getMaxEventId(uint32_t);
	int getNewEventId(uint32_t, uint32_t = 0);
	int add(QccEdit *pe);

	void sort();
	void sync();
	void show();

	uint32_t event_no[QCC_YEAR_NUM];
	int event_spare[QCC_YEAR_NUM][QCC_EVENT_SPARE_NUM];
};

#endif			/* _QCCDATABASE_H_ */
