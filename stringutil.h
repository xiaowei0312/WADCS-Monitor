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
    static bool validHexStrWithSpace(QString text);
    static bool validHexStrWithNoSpace(QString text);
    static QByteArray convertHexStringToByteArray(QString hexStr);
    static QString convertByteArrayToHexString(const QByteArray &byteArray);
    static unsigned int convertByteArrayToInteger(const QByteArray &byteArray);
};

#endif // STRINGUTIL_H
