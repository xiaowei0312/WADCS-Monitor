#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <QString>
#include <QRegExp>
#include <QRegExpValidator>
#include <QByteArray>

class StringUtil:public QObject
{
public:
    StringUtil();
    static bool validHexStr(QString text);
    static QByteArray convertHexStringToByteArray(QString hexStr);
    static QString convertByteArrayToHexString(QByteArray byteArray);
};

#endif // STRINGUTIL_H
