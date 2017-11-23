#ifndef COMMON_H
#define COMMON_H

#include <QDateTime>

struct UartDataPackage{
    QByteArray head,tail;     //head and tail
    QByteArray data;             //data
    QDateTime timestamp;        //timestamp
    unsigned int length;    //length
    unsigned int checksum;  //checksum
    bool isValid;           //包是否有效? 根据包头 + 校验和确定包是否有效
};

#endif // COMMON_H
