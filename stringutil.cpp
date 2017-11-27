#include "stringutil.h"

StringUtil::StringUtil()
{
}

bool StringUtil::validHexStr(QString text){
    int pos = 0;
    text += " ";
    //QRegExpValidator hexReg(QRegExp("([0-9A-Fa-f]{2}[ ]+)+([0-9A-Fa-f]{2}[ ]*)?"),NULL);
    QRegExpValidator hexReg(QRegExp("([0-9A-Fa-f]{2}[ ]+)+"),NULL);
    if(hexReg.validate(text,pos) == QValidator::Acceptable) 
        return true;
    return false;
}

//Params: FF 1A 01 02 03
//Return: QByteArray(0xFF 0x1A 0x01 0x02 0x03)
QByteArray StringUtil::convertHexStringToByteArray(QString hexStr)
{
    return QByteArray::fromHex(hexStr.remove(QRegExp("\\s")).toLatin1());
}

//Params: QByteArray(0xFF 0x1A 0x01 0x02 0x03)
//Return: FF 1A 01 02 03
QString StringUtil::convertByteArrayToHexString(const QByteArray &byteArray)
{
    QString str;
    QByteArray hexByteArray = byteArray.toHex();
    for(int i=0;i<hexByteArray.length();i+=2){
        str.append(hexByteArray.at(i));
        str.append(hexByteArray.at(i+1));
        str.append(' ');
    }
    //return str.left(str.length()-1);
    return str;
}
