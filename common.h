#ifndef COMMON_H
#define COMMON_H

#include <QDateTime>

struct UartDataPackage{
    QByteArray head,tail;     //head and tail
    QByteArray data;             //data
    QDateTime timestamp;        //timestamp
    unsigned int length;    //length
    unsigned int totalLength;   //totalPackageLength
    unsigned int checksum;  //checksum
    bool isValid;           //包是否有效? 根据包头 + 校验和确定包是否有效
};

struct UartProtoConfig{ //head + length + data + checksum + tail
    bool needParsed;        //是否需要解析
    bool isFixedLength;     //是否为固定长度协议
    int fixedLength;       //>0表示固定长度类型协议 

    QByteArray fixedHead,fixedTail; //固定头尾
    int lengthBytes;        //协议长度字节个数（数据字节），固定为1个字节
    int checksumBytes;      //协议校验和字节个数
    int checksum;           //校验和数组下标
};

#endif // COMMON_H
