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

#include "qcclient_month.h"
#include "qcclient_button.h"

const char *months[QCC_MONTH_NUM_H][QCC_MONTH_NUM_W] = {
	{ "JAN", "FEB", "MAR", "APR" },
	{ "MAI", "JUN", "JUL", "AUG" },
	{ "SEP", "OKT", "NOV", "DES" },
};

QccMonth :: QccMonth(QccMainWindow *_parent) :
  QWidget()
{
	unsigned x;
	unsigned y;

	parent = _parent;
	gl = new QGridLayout(this);
	gl->setSpacing(0);

	for (x = 0; x != QCC_MONTH_NUM_W; x++) {
		for (y = 0; y != QCC_MONTH_NUM_H; y++) {
			button[x][y] = new QccButton(QString(months[y][x]), (y * QCC_MONTH_NUM_W) + x + 1);
			QFont fnt = button[x][y]->font();
			fnt.setBold(1);
			button[x][y]->setFont(fnt);
			connect(button[x][y], SIGNAL(released(int)), parent, SLOT(handle_select_month(int)));
			gl->addWidget(button[x][y], y, x, 1, 1);
		}
	}
}

QccMonth :: ~QccMonth()
{

}
