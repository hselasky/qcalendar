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

#include "qcserver.h"

#define	SNPRINTF(buf, size, ...) do {		\
	memset((buf), 0, (size));		\
	snprintf((buf), (size), __VA_ARGS__);	\
} while (0)

QcsMain :: QcsMain()
{
	char buf[64];
	int x;

	tcp = new QTcpServer(this);

	if (!tcp->listen(QHostAddress(QString(QCS_HOST_ADDR)), QCS_HOST_PORT))
		err(EX_SOFTWARE, "Cannot listen");

	connect(tcp, SIGNAL(newConnection()), this, SLOT(handle_connection()));

	for (x = 0; x != QCS_YEAR_NUM; x++) {
		max_event_id[x] = 0;

		while (1) {
			SNPRINTF(buf, sizeof(buf), "qcserver_event_%d_%d", x + QCS_YEAR_START, max_event_id[x]);
			QFile file(QString(QCS_ROOT_DIR "/") + QString(buf));

			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				break;
			file.close();

			max_event_id[x]++;
		}
	}
}

void
QcsMain :: handle_connection()
{
	QTcpSocket *conn = tcp->nextPendingConnection();

	if (conn == 0)
		return;

	char buf[64];
	int len;
	int year;
	int event;
	int size;

	while (1) {
		while (conn->bytesAvailable() < (int)sizeof(buf)) {
			if (!conn->waitForReadyRead(4000))
				break;
		}

		len = conn->read(buf, sizeof(buf));
		if (len != (int)sizeof(buf))
			break;
		buf[sizeof(buf)-1] = 0;

		if (sscanf(buf, "PULL_%d_%d", &year, &event) == 2) {
			QByteArray ba;

			if (year < QCS_YEAR_START || year > QCS_YEAR_STOP)
				break;
			if (event < 0)
				break;

			SNPRINTF(buf, sizeof(buf), "qcserver_event_%d_%d", year, event);
			QFile file(QString(QCS_ROOT_DIR "/") + QString(buf));

			if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				SNPRINTF(buf, sizeof(buf), "BYTES_0\n");
				conn->write(buf, sizeof(buf));
			} else {
				ba = file.readAll();
				SNPRINTF(buf, sizeof(buf), "BYTES_%d\n", (int)ba.size());
				conn->write(buf, sizeof(buf));
				conn->write(ba);
			}
		} else if (sscanf(buf, "PUSH_%d_%d_%d",
		    &year, &event, &size) == 3) {
			char *ptr;

			if (year < QCS_YEAR_START || year > QCS_YEAR_STOP)
				break;
			if (event < 0)
				break;
			if (size < 0 || size > QCS_EVENT_SIZE_MAX)
				break;

			ptr = (char *)malloc(size);
			if (ptr == 0)
				break;

			while (conn->bytesAvailable() < size) {
				if (!conn->waitForReadyRead(4000))
					break;
			}
			if (conn->read(ptr, size) != size) {
				free(ptr);
				break;
			}

			SNPRINTF(buf, sizeof(buf), "qcserver_event_%d_%d", year, event);
			QFile file(QString(QCS_ROOT_DIR "/") + QString(buf));

			if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
				SNPRINTF(buf, sizeof(buf), "ERROR\n");
				conn->write(buf, sizeof(buf));
			} else {
				if (file.write(ptr, size) != size) {
					SNPRINTF(buf, sizeof(buf), "ERROR\n");
					conn->write(buf, sizeof(buf));
				} else {
					SNPRINTF(buf, sizeof(buf), "OK\n");
					conn->write(buf, sizeof(buf));
				}
				file.close();
			}
			free(ptr);
		} else if (sscanf(buf, "MAX_EVENT_ID_%d", &year) == 1) {
			if (year < QCS_YEAR_START || year > QCS_YEAR_STOP)
				break;
			SNPRINTF(buf, sizeof(buf), "ID_%d\n", max_event_id[year - QCS_YEAR_START]);
			conn->write(buf, sizeof(buf));
		} else if (sscanf(buf, "NEW_EVENT_ID_%d", &year) == 1) {
			if (year < QCS_YEAR_START || year > QCS_YEAR_STOP)
				break;
			if (max_event_id[year - QCS_YEAR_START] >= QCS_EVENT_DELTA_MAX)
				break;

			SNPRINTF(buf, sizeof(buf), "qcserver_event_%d_%d",
			    year, max_event_id[year - QCS_YEAR_START]);

			QFile file(QString(QCS_ROOT_DIR "/") + QString(buf));

			if (!file.open(QIODevice::ReadWrite |
			    QIODevice::Text | QIODevice::Truncate)) {
				SNPRINTF(buf, sizeof(buf), "ID_-1\n");
				conn->write(buf, sizeof(buf));
			} else {
				char buffer[128];
				SNPRINTF(buffer, sizeof(buffer), 
				   "[General]\n"
				   "date=\n"
				   "from=\n"
				   "to=\n"
				   "event=\n"
				   "user=0\n"
				   "status=2\n" /* deleted */
				   "id=%d\n",
				   max_event_id[year - QCS_YEAR_START]);

				file.write(buffer);
				file.close();

				SNPRINTF(buf, sizeof(buf), "ID_%d\n",
				    max_event_id[year - QCS_YEAR_START]);
				max_event_id[year - QCS_YEAR_START]++;
				conn->write(buf, sizeof(buf));
			}
		} else {
			break;
		}
		conn->flush();
	}
	conn->close();
	delete conn;
}

QcsMain :: ~QcsMain()
{
	delete tcp;
}

Q_DECL_EXPORT int
main(int argc, char **argv)
{
  	QApplication app(argc, argv, 0);

	new QcsMain();

	return (app.exec());
}
