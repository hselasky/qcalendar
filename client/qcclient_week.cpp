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

#include "qcclient_week.h"
#include "qcclient_button.h"

const char *weeks[QCC_WEEK_NUM_W] = {
	"WEEK", "MO", "TU", "WE", "TH", "FR", "SA", "SU"
};

QccWeek :: QccWeek(QccMainWindow *_parent) :
  QWidget()
{
	unsigned x;
	unsigned y;

	parent = _parent;
	gl = new QGridLayout(this);
	gl->setSpacing(0);

	for (x = 0; x != QCC_WEEK_NUM_H; x++) {
		for (y = 0; y != QCC_WEEK_NUM_W; y++) {
			if (x != 0 && y != 0) {
				button[x][y] = new QccButton(QString(), -1);
				QFont fnt = button[x][y]->font();
				fnt.setBold(1);
				button[x][y]->setFont(fnt);
				connect(button[x][y], SIGNAL(released(int)), parent, SLOT(handle_select_day(int)));
			} else if (x == 0) {
				button[0][y] = new QccButton(QString(weeks[y]), -1);
			} else if (y == 0) {
				button[x][0] = new QccButton(QString(), -1);
			}
			gl->addWidget(button[x][y],x,y,1,1);
		}
	}
	syncText();
}

QccWeek :: ~QccWeek()
{

}

void
QccWeek :: syncText(void)
{
	unsigned x;
	unsigned y;
	int yr,m,d,wn;
	QDate today = QDate::currentDate();
	QDate temp = parent->curr;

	temp.getDate(&yr,&m,&d);
	temp.setDate(yr,m,1);

	wn = temp.weekNumber();

	for (x = 1; x != QCC_WEEK_NUM_H; x++) {
		for (y = 1; y != QCC_WEEK_NUM_W; y++) {
			button[x][y]->setText(QString());
			button[x][y]->setId(-1);
		}
	}

	while (1) {
		int id;
		int num;

		y = temp.dayOfWeek();
		x = temp.weekNumber() - wn + 1;

		if (y == 7 || temp.month() != m) {
			id = temp.weekNumber();

			button[x][0]->setText(QString("%1").arg(id));
			button[x][0]->setId(id);

			if (temp.month() != m)
				break;
		}
		id = temp.day();

		button[x][y]->setFlat(temp == today);

		if ((yr >= QCC_YEAR_START && yr <= QCC_YEAR_STOP) &&
		    ((num = parent->usage[yr - QCC_YEAR_START][m - 1][id - 1]) != 0)) {
			switch (num) {
			case 1:
				num = '-';
				break;
			case 2:
				num = '+';
				break;
			default:
				num = '*';
				break;
			}
			button[x][y]->setText(QString("%1%2").arg(id).arg(QChar(num)));
		} else {
			button[x][y]->setText(QString("%1").arg(id));
		}
		button[x][y]->setId(id);	  

		temp = temp.addDays(1);
	}
}
