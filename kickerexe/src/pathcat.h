
#ifndef __PATHCAT_H
#define __PATHCAT_H

#include <QString>
#include <string>

std::string pathcat(char const *left, char const *right);
std::string pathcat(std::string const &left, std::string const &right);

QString qpathcat(ushort const *left, ushort const *right);

inline QString pathcat(QString const &left, QString const &right)
{
	return qpathcat(left.utf16(), right.utf16());
}

#endif
