#include "LinkToGameServer.h"
#include "EpollClientLoginSlave.h"
#include <iostream>
#include "EpollServerLoginSlave.h"
#include "../../general/base/CommonSettingsCommon.h"

using namespace CatchChallenger;

void LinkToGameServer::parseInputBeforeLogin(const quint8 &mainCodeType, const quint8 &queryNumber, const char *data, const unsigned int &size)
{
    Q_UNUSED(queryNumber);
    Q_UNUSED(size);
    Q_UNUSED(data);
    switch(mainCodeType)
    {
        case 0x03:
        {
            //Protocol initialization
            if(size<1)
            {
                parseNetworkReadError(QStringLiteral("wrong size with main ident: %1 and queryNumber: %2, type: query_type_protocol").arg(mainCodeType).arg(queryNumber));
                return;
            }
            quint8 returnCode=data[0x00];
            if(returnCode>=0x04 && returnCode<=0x06)
            {
                switch(returnCode)
                {
                    case 0x04:
                        ProtocolParsing::compressionTypeClient=ProtocolParsing::CompressionType::None;
                    break;
                    case 0x05:
                        ProtocolParsing::compressionTypeClient=ProtocolParsing::CompressionType::Zlib;
                    break;
                    case 0x06:
                        ProtocolParsing::compressionTypeClient=ProtocolParsing::CompressionType::Xz;
                    break;
                    default:
                        parseNetworkReadError(QStringLiteral("compression type wrong with main ident: %1 and queryNumber: %2, type: query_type_protocol").arg(mainCodeType).arg(queryNumber));
                    return;
                }
                if(size!=(sizeof(quint8)))
                {
                    parseNetworkReadError(QStringLiteral("compression type wrong size (stage 3) with main ident: %1 and queryNumber: %2, type: query_type_protocol").arg(mainCodeType).arg(queryNumber));
                    return;
                }
                //send token to game server
                packFullOutcommingQuery(0x02,0x06,queryIdToLog/*query number*/,tokenForGameServer,sizeof(tokenForGameServer));
                stat=ProtocolGood;
                return;
            }
            else
            {
                if(returnCode==0x02)
                    parseNetworkReadError("Protocol not supported");
                else if(returnCode==0x03)
                    parseNetworkReadError("Server full");
                else
                    parseNetworkReadError(QStringLiteral("Unknown error %1").arg(returnCode));
                return;
            }
        }
        break;
        default:
            parseNetworkReadError(QStringLiteral("unknown sort ident reply code: %1, line: %2").arg(mainCodeType).arg(QStringLiteral("%1:%2").arg(__FILE__).arg(__LINE__)));
            return;
        break;
    }
}

void LinkToGameServer::parseMessage(const quint8 &mainCodeType,const char *data,const unsigned int &size)
{
    if(stat!=Stat::Logged)
    {
        parseNetworkReadError("parseFullMessage() not logged to send: "+QString::number(mainCodeType));
        return;
    }
    (void)data;
    (void)size;
    if(client!=NULL)
        client->packOutcommingData(mainCodeType,data,size);
}

void LinkToGameServer::parseFullMessage(const quint8 &mainCodeType,const quint8 &subCodeType,const char *rawData,const unsigned int &size)
{
    if(stat!=Stat::Logged)
    {
        parseNetworkReadError("parseFullMessage() not logged to send: "+QString::number(mainCodeType)+" "+QString::number(subCodeType));
        return;
    }
    (void)rawData;
    (void)size;
    if(client!=NULL)
        client->packFullOutcommingData(mainCodeType,subCodeType,rawData,size);
}

//have query with reply
void LinkToGameServer::parseQuery(const quint8 &mainCodeType,const quint8 &queryNumber,const char *data,const unsigned int &size)
{
    Q_UNUSED(data);
    if(stat!=Stat::Logged)
    {
        parseInputBeforeLogin(mainCodeType,queryNumber,data,size);
        return;
    }
    if(client!=NULL)
        client->packOutcommingQuery(mainCodeType,queryNumber,data,size);
}

void LinkToGameServer::parseFullQuery(const quint8 &mainCodeType,const quint8 &subCodeType,const quint8 &queryNumber,const char *rawData,const unsigned int &size)
{
    (void)subCodeType;
    (void)queryNumber;
    (void)rawData;
    (void)size;
    if(stat!=Stat::Logged)
    {
        parseNetworkReadError(QStringLiteral("is not logged, parseQuery(%1,%2)").arg(mainCodeType).arg(queryNumber));
        return;
    }
    //do the work here
    if(client!=NULL)
        client->packFullOutcommingQuery(mainCodeType,subCodeType,queryNumber,rawData,size);
}

//send reply
void LinkToGameServer::parseReplyData(const quint8 &mainCodeType,const quint8 &queryNumber,const char *data,const unsigned int &size)
{
    if(stat!=Stat::Logged)
    {
        if(mainCodeType==0x03 && queryNumber==0x01 && stat==Stat::Connected)
        {
            parseInputBeforeLogin(mainCodeType,queryNumber,data,size);
            return;
        }
        else
        {
            parseNetworkReadError(QStringLiteral("is not logged, parseReplyData(%1,%2)").arg(mainCodeType).arg(queryNumber));
            return;
        }
    }
    Q_UNUSED(data);
    Q_UNUSED(size);
    //do the work here

    if(client!=NULL)
        client->postReply(queryNumber,data,size);
}

void LinkToGameServer::parseFullReplyData(const quint8 &mainCodeType,const quint8 &subCodeType,const quint8 &queryNumber,const char *data,const unsigned int &size)
{
    if(stat!=Stat::Logged)
    {
        if(stat==Stat::ProtocolGood && mainCodeType==0x02 && subCodeType==0x06)
        {}
        else
        {
            parseNetworkReadError(QStringLiteral("is not logged, parseReplyData(%1,%2)").arg(mainCodeType).arg(queryNumber));
            return;
        }
    }
    (void)data;
    (void)size;
    //do the work here
        if(client!=NULL)
            client->postReply(queryNumber,data,size);
}

void LinkToGameServer::parseNetworkReadError(const QString &errorString)
{
    errorParsingLayer(errorString);
}