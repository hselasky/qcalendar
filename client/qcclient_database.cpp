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

#include "qcclient_database.h"
#include "qcclient_edit.h"
#include "qcclient_day.h"

QccDatabase :: QccDatabase(QccMainWindow *_parent)
{
	uint32_t x;

	parent = _parent;
	tcp = 0;

	memset(event_spare, 0xff, sizeof(event_spare));

	for (x = 0; x != QCC_YEAR_NUM; x++) {
		TAILQ_INIT(&head[x]);
		event_no[x] = 0;
	}
}

QccDatabase :: ~QccDatabase()
{
	QccEdit *pe;
	uint32_t x;

	for (x = 0; x != QCC_YEAR_NUM; x++) {
		while ((pe = TAILQ_FIRST(&head[x])) != 0) {
			TAILQ_REMOVE(&head[x], pe, entry);
			delete pe;
		}
	}
}

QccEdit *
QccDatabase :: pullEventById(uint32_t year, uint32_t event, uint32_t refresh)
{
	QFile *file = 0;

	char *ptr = 0;
	char buf[64];
	int len;

	SNPRINTF(buf, sizeof(buf), "qcclient_event_%d_%d", (int)year, (int)event);
	QSettings setting(QccDatabasePath + QChar('/') + QString(buf), QSettings::IniFormat);

	if (refresh == 0) {
		if (((QccEdit *)0)->validData(&setting))
			goto parse;

		setting.sync();
	}

	SNPRINTF(buf, sizeof(buf), "PULL_%d_%d\n", (int)year, (int)event);
	tcp->write(buf, sizeof(buf));
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buf)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buf, sizeof(buf));
	if (len != (int)sizeof(buf))
		goto error;

	buf[sizeof(buf)-1] = 0;

	if (sscanf(buf, "BYTES_%d", &len) != 1)
		goto error;

	if (len <= 0 || len > QCC_EVENT_SIZE_MAX)
		goto error;

	ptr = (char *)malloc(len);
	if (ptr == 0)
		goto error;

	while (tcp->bytesAvailable() < len) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}
	if (tcp->read(ptr, len) != len)
		goto error;

	file = new QFile(setting.fileName());

	if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		goto error;

	file->write(ptr, len);
	file->close();

	setting.sync();

	if (((QccEdit *)0)->validData(&setting))
		goto parse;

error:
	free(ptr);
	delete(file);
	return (0);

parse:
	QccEdit *pe = new QccEdit(parent);
	pe->tab_parent = parent->day->tab;
	pe->importData(&setting);
	free(ptr);
	delete(file);
	return (pe);
}

void
QccDatabase :: pushEventById(uint32_t year, QccEdit *pe)
{
	QFile *file = 0;
	char buf[64];
	int len;
	QByteArray ba;

	if (!(pe->status & ST_DIRTY))
		return;

	/* get new ID, if any */
	if (pe->id < 0) {
		if (pe->status & ST_REMOVED)
			return;
		pe->id = getNewEventId(year);
		if (pe->id < 0)
			return;
	}

	pe->status &= ~ST_DIRTY;

	SNPRINTF(buf, sizeof(buf), "qcclient_event_%d_%d", (int)year, (int)pe->id);
	QSettings setting(QccDatabasePath + QChar('/') + QString(buf), QSettings::IniFormat);

	pe->exportData(&setting);

	setting.sync();

	file = new QFile(setting.fileName());

	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
		goto error;

	ba = file->readAll();

	SNPRINTF(buf, sizeof(buf), "PUSH_%d_%d_%d\n",
	    (int)year, (int)pe->id, (int)ba.size());
	tcp->write(buf, sizeof(buf));
	tcp->write(ba);
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buf)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buf, sizeof(buf));
	if (len != (int)sizeof(buf))
		goto error;

	buf[sizeof(buf)-1] = 0;

	if (strcmp(buf, "OK\n"))
		goto error;
	delete(file);
	return;

error:
	parent->handle_failure(parent->tr("Could not store event"));
	pe->status |= ST_DIRTY;
	delete(file);
}

uint32_t
QccDatabase :: getMaxEventId(uint32_t year)
{
	char buf[64];
	int len;

	SNPRINTF(buf, sizeof(buf), "MAX_EVENT_ID_%d\n", (int)year);
	tcp->write(buf, sizeof(buf));
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buf)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buf, sizeof(buf));
	if (len != (int)sizeof(buf))
		goto error;

	buf[sizeof(buf)-1] = 0;

	if (sscanf(buf, "ID_%d", &len) != 1)
		goto error;

	if (len < 0 || len > QCC_EVENT_DELTA_MAX)
		goto error;

	return (len);
error:
	parent->handle_failure(parent->tr("Could not read max ID"));

	QSettings setting("qcclient");
	QString key = QString("year_%1_event").arg(year);
	if (setting.contains(key)) {
		len = setting.value(key).toInt();
		if (len > QCC_EVENT_DELTA_MAX)
			len = 0;
		return (len);
	}
	return (0);
}

int
QccDatabase :: getNewEventId(uint32_t year, uint32_t no_spares)
{
	char buf[64];
	int len;
	int x;

	SNPRINTF(buf, sizeof(buf), "NEW_EVENT_ID_%d\n", (int)year);
	tcp->write(buf, sizeof(buf));
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buf)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buf, sizeof(buf));
	if (len != (int)sizeof(buf))
		goto error;

	buf[sizeof(buf)-1] = 0;

	if (sscanf(buf, "ID_%d", &len) != 1)
		goto error;

	if (len < 0 || len >= QCC_EVENT_DELTA_MAX)
		goto error;
	return (len);

error:
	parent->handle_failure(parent->tr("Could not read current ID"));

	if (no_spares != 0)
		return (-1);

	QSettings setting("qcclient");

	for (x = 0; x != QCC_EVENT_SPARE_NUM; x++) {
		QString key = QString("year_%1_spare_%2").arg(year).arg(x);
		if (setting.contains(key) == 0)
			continue;
		len = setting.value(key).toInt();
		if (len < 0 || len >= QCC_EVENT_DELTA_MAX)
			continue;
		setting.setValue(key, -1);
		setting.sync();
		return (len);
	}
	return (-1);
}

int
QccDatabase :: getHashes(uint32_t year, int event, QByteArray &curr)
{
	char buf[64];
	int len;
	int x;
	int y = year - qcc_year_start;
	int retval = 1;
	char *ptr_curr;
	char *ptr_temp;

	if (event == 0)
		return (retval);

	/* simple case, pull new events */
	for (x = event_no[y]; x < event; x++) {
		QccEdit *pe = pullEventById(year, x, 0);
		if (pe == 0)
			break;
		TAILQ_INSERT_TAIL(&head[y], pe, entry);
	}
	event_no[y] = event;

	/* look for updates, only */
	QByteArray temp;

	SNPRINTF(buf, sizeof(buf), "HASHES_%d_%d\n", year, event);
	tcp->write(buf, sizeof(buf));
	tcp->flush();

	while (tcp->bytesAvailable() < (int)sizeof(buf)) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}

	len = tcp->read(buf, sizeof(buf));
	if (len != (int)sizeof(buf))
		goto error;

	buf[sizeof(buf)-1] = 0;

	if (sscanf(buf, "BYTES_%d", &len) != 1)
		goto error;
	if (len != (event * QCC_HASH_SIZE))
		goto error;
	while (tcp->bytesAvailable() < len) {
		if (!tcp->waitForReadyRead(4000))
			goto error;
	}
	temp = tcp->read(len);

	if (temp.size() != len)
		goto error;

	ptr_temp = temp.data();
	ptr_curr = curr.data();

	for (x = 0; x != len; x += QCC_HASH_SIZE) {
		if ((x + QCC_HASH_SIZE) > curr.size()) {
			retval = 0;
			break;
		}
		if (memcmp(ptr_curr + x, ptr_temp + x, QCC_HASH_SIZE) != 0) {
			QccEdit *pe;
			QccEdit *pe_temp;

			TAILQ_FOREACH_SAFE(pe, &head[y], entry, pe_temp) {
				if (pe->id == (x / QCC_HASH_SIZE)) {
					TAILQ_REMOVE(&head[y], pe, entry);
					delete (pe);
				}
			}

			pe = pullEventById(year, x / QCC_HASH_SIZE, 1);
			if (pe != 0)
				TAILQ_INSERT_TAIL(&head[y], pe, entry);

			retval = 0;
		}
	}

	curr = temp;
	return (retval);

error:
	parent->handle_failure(parent->tr("Could not read hashes"));
	return (1);
}

static int
compare(const void *a, const void *b)
{
	const QccEdit *pa = *(const QccEdit **)a;
	const QccEdit *pb = *(const QccEdit **)b;

	return (pa->compare(*pb));
}

void
QccDatabase :: sort()
{
	uint32_t x;
	uint32_t n;
	uint32_t y;
	QccEdit *pe;
	QccEdit **ppe;

	memset(parent->usage, 0, sizeof(parent->usage));

	for (x = 0; x != QCC_YEAR_NUM; x++) {
		n = 0;
		TAILQ_FOREACH(pe, &head[x], entry) {
			n++;
		}
		if (n == 0)
			continue;

		ppe = (QccEdit **)malloc(sizeof(void *) * n);
		if (ppe == 0)
			break;

		/* remove events from list */
		n = 0;
		while ((pe = TAILQ_FIRST(&head[x])) != 0) {
			 TAILQ_REMOVE(&head[x], pe, entry);
			 ppe[n] = pe;
			 n++;
		}

		/* sort events */
		mergesort(ppe, n, sizeof(void *), compare);

		/* remove duplicates */
		for (y = 1; y != n; y++) {
			if (ppe[y]->compare(*ppe[y - 1]) == 0) {
				delete (ppe[y - 1]);
				ppe[y - 1] = 0;
			}
		}

		/* remove deleted events */
		for (y = 0; y != n; y++) {
			if (ppe[y] == 0)
				continue;
			if ((ppe[y]->status & ST_REMOVED) && (ppe[y]->id == -1)) {
				delete (ppe[y]);
				ppe[y] = 0;
			}
		}

		/* put back in list */
		for (y = 0; y != n; y++) {
			pe = ppe[y];
			if (pe == 0)
				continue;
			if (!(pe->status & ST_REMOVED)) {
				int a,b,c;
				pe->date.getDate(&a,&b,&c);

				/* Do stats */
				if (a >= qcc_year_start && a <= qcc_year_stop)
					parent->usage[a - qcc_year_start][b - 1][c - 1]++;
			}
			TAILQ_INSERT_TAIL(&head[x], pe, entry);
		}
		free(ppe);
	}
}

void
QccDatabase :: sync()
{
	QccEdit *pe;
	uint32_t event_new;
	uint32_t x;
	uint32_t y;

	parent->handle_failure(parent->tr("OK"));

	QSettings setting("qcclient");

	/* unref any QccEdits */
	while (parent->day->tab->count())
		parent->day->tab->removeTab(0);

	tcp = new QTcpSocket();

	tcp->connectToHost(QCC_HOST_ADDR, QCC_HOST_PORT);
	if (tcp->waitForConnected(4000) == 0) {
		parent->handle_failure(parent->tr("Could not connect to HOST"));
		goto pull_only;
	}

	/* refresh spares, if any */
	for (y = 0; y != QCC_YEAR_NUM; y++) {
		for (x = 0; x != QCC_EVENT_SPARE_NUM; x++) {
			QString key = QString("year_%1_spare_%2")
			    .arg(qcc_year_start + y).arg(x);

			if (setting.contains(key) != 0 &&
			    setting.value(key).toInt() > -1)
				continue;

			int id = getNewEventId(qcc_year_start + y, 1);
			if (id < 0 || id >= QCC_EVENT_DELTA_MAX)
				break;
			setting.setValue(key, id);
		}
	}

	setting.sync();

	/* ensure all events are pushed */
	for (x = 0; x != QCC_YEAR_NUM; x++) {
		TAILQ_FOREACH(pe, &head[x], entry)
			pushEventById(qcc_year_start + x, pe);
     	}

pull_only:
	/* pull new events, if any */
	for (x = 0; x != QCC_YEAR_NUM; x++) {

		QString key;

		/* get latest counter */
		event_new = getMaxEventId(x + qcc_year_start);
		if (event_new > QCC_EVENT_DELTA_MAX) {
			parent->handle_failure(parent->tr("Event overflow"));
			event_new = QCC_EVENT_DELTA_MAX;
		}

		/* grab changes first */
		key = QString("hashes_%1").arg(x + qcc_year_start);

		QByteArray hashes_curr;

		if (setting.contains(key) != 0)
			hashes_curr = qUncompress(setting.value(key).toByteArray());

		if (getHashes(x + qcc_year_start, event_new, hashes_curr) == 0)
			setting.setValue(key, qCompress(hashes_curr, 9));

		key = QString("year_%1_event").arg(qcc_year_start + x);
		setting.setValue(key, event_no[x]);
	}
	setting.sync();
	tcp->close();

	delete tcp;
	tcp = 0;

	sort();
}

void
QccDatabase :: show()
{
	QccEdit *pe;
	int y,m,d;
	int x;

	parent->curr.getDate(&y,&m,&d);

	x = y - qcc_year_start;
	if (x < 0 || x >= QCC_YEAR_NUM)
		return;

	/* unref any QccEdits */
	while (parent->day->tab->count())
		parent->day->tab->removeTab(0);

	TAILQ_FOREACH(pe, &head[x], entry) {
		if (pe->status & ST_REMOVED)
			continue;
		int a,b,c;
		pe->date.getDate(&a,&b,&c);

		/* Add tab to view, if any */
		if (a == y && b == m && d == c)
			parent->day->tab->addTab(pe, pe->tabString());
	}
}

int
QccDatabase :: add(QccEdit *pe)
{
	int y,m,d;
	int x;

	pe->date.getDate(&y,&m,&d);

	x = y - qcc_year_start;
	if (x < 0 || x >= QCC_YEAR_NUM) {
		delete pe;
		return (0);
	}

	pe->status |= ST_DIRTY;
	pe->tab_parent = parent->day->tab;

	TAILQ_INSERT_TAIL(&head[x], pe, entry);
	return (1);
}
