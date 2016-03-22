#include "misc.h"
#include <vector>

QString operator / (QString const &left, QString const &right)
{
	ushort const *ll = left.utf16();
	ushort const *lr = ll + left.size();
	ushort const *rl = right.utf16();
	ushort const *rr = rl + right.size();
	while (ll < lr && QChar(*ll).isSpace()) ll++;
	while (ll < lr && QChar(lr[-1]).isSpace()) lr--;
	while (ll < lr && (lr[-1] == '/' || lr[-1] == '\\')) lr--;
	while (rl < rr && QChar(*rl).isSpace()) rl++;
	while (rl < rr && QChar(rr[-1]).isSpace()) rr--;
	while (rl < rr && (*rl == '/' || *rl == '\\')) rl++;
	size_t nl = lr - ll;
	size_t nr = rr - rl;
	size_t len = nl + 1 + nr;
	std::vector<ushort> tmp(len);
	ushort *p = &tmp[0];
	std::copy(ll, lr, p);
	p[nl] = '/';
	std::copy(rl, rr, p + nl + 1);
	return QString::fromUtf16(p, len);
}

