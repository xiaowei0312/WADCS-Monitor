#ifndef COMMON_H
#define COMMON_H

#include <QDateTime>

struct UartDataPackage{
    QByteArray head,tail;     //head and tail
    QByteArray data;             //data
    QDateTime timestamp;        //timestamp
    unsigned int length;    //length
    unsigned int checksum;  //checksum
};

#endif // COMMON_H
