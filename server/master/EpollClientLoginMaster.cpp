#include "EpollClientLoginMaster.h"

#include <iostream>
#include <QString>

using namespace CatchChallenger;

EpollClientLoginMaster::EpollClientLoginMaster(
        #ifdef SERVERSSL
            const int &infd, SSL_CTX *ctx
        #else
            const int &infd
        #endif
        ) :
        ProtocolParsingInputOutput(
            #ifdef SERVERSSL
                infd,ctx
            #else
                infd
            #endif
           #ifndef CATCHCHALLENGERSERVERDROPIFCLENT
            ,PacketModeTransmission_Server
            #endif
            ),
        stat(None),
        socketString(NULL),
        socketStringSize(0),
        charactersGroupForGameServer(NULL)
{
}

EpollClientLoginMaster::~EpollClientLoginMaster()
{
    if(socketString!=NULL)
        delete socketString;
    if(stat==EpollClientLoginMasterStat::LoginServer)
    {
        EpollClientLoginMaster::loginServers.removeOne(this);
        int index=0;
        while(index<EpollClientLoginMaster::gameServers.size())
        {
            int sub_index=0;
            while(sub_index<EpollClientLoginMaster::gameServers.at(index)->loginServerReturnForCharaterSelect.size())
            {
                const DataForSelectedCharacterReturn &dataForSelectedCharacterReturn=EpollClientLoginMaster::gameServers.at(index)->loginServerReturnForCharaterSelect.at(sub_index);
                if(dataForSelectedCharacterReturn.loginServer==this)
                    EpollClientLoginMaster::gameServers.at(index)->loginServerReturnForCharaterSelect[sub_index].loginServer=NULL;
                sub_index++;
            }
            index++;
        }
    }
    if(stat==EpollClientLoginMasterStat::GameServer)
    {
        EpollClientLoginMaster::gameServers.removeOne(this);
        int index=0;
        while(index<loginServerReturnForCharaterSelect.size())
        {
            const DataForSelectedCharacterReturn &dataForSelectedCharacterReturn=loginServerReturnForCharaterSelect.at(index);
            if(dataForSelectedCharacterReturn.loginServer!=NULL)
                dataForSelectedCharacterReturn.loginServer->selectCharacter_ReturnFailed(dataForSelectedCharacterReturn.client_query_id,0x04,dataForSelectedCharacterReturn.characterId);
            index++;
        }
    }
}

void EpollClientLoginMaster::disconnectClient()
{
    epollSocket.close();
    messageParsingLayer("Disconnected client");
}

//input/ouput layer
void EpollClientLoginMaster::errorParsingLayer(const QString &error)
{
    std::cerr << socketString << ": " << error.toLocal8Bit().constData() << std::endl;
    disconnectClient();
}

void EpollClientLoginMaster::messageParsingLayer(const QString &message) const
{
    std::cout << socketString << ": " << message.toLocal8Bit().constData() << std::endl;
}

void EpollClientLoginMaster::errorParsingLayer(const char * const error)
{
    std::cerr << socketString << ": " << error << std::endl;
    disconnectClient();
}

void EpollClientLoginMaster::messageParsingLayer(const char * const message) const
{
    std::cout << socketString << ": " << message << std::endl;
}

BaseClassSwitch::Type EpollClientLoginMaster::getType() const
{
    return BaseClassSwitch::Type::Client;
}

void EpollClientLoginMaster::parseIncommingData()
{
    ProtocolParsingInputOutput::parseIncommingData();
}

void EpollClientLoginMaster::selectCharacter(const quint8 &query_id,const quint32 &serverUniqueKey,const quint8 &charactersGroupIndex,const quint32 &characterId)
{
    if(charactersGroupIndex<=CharactersGroup::list.size())
    {
        errorParsingLayer("EpollClientLoginMaster::selectCharacter() charactersGroupIndex is out of range");
        return;
    }
    #ifdef CATCHCHALLENGER_EXTRA_CHECK
    removeFromQueryReceived(query_id);
    #endif
    if(!CharactersGroup::list.at(charactersGroupIndex)->containsServerUniqueKey(serverUniqueKey))
    {
        EpollClientLoginMaster::loginIsWrongBuffer[1]=query_id;
        EpollClientLoginMaster::loginIsWrongBuffer[3]=0x05;
        internalSendRawSmallPacket(reinterpret_cast<char *>(EpollClientLoginMaster::loginIsWrongBuffer),sizeof(EpollClientLoginMaster::loginIsWrongBuffer));
        return;
    }
    if(CharactersGroup::list.at(charactersGroupIndex)->lockedAccount.contains(characterId))
    {
        EpollClientLoginMaster::loginIsWrongBuffer[1]=query_id;
        EpollClientLoginMaster::loginIsWrongBuffer[3]=0x03;
        internalSendRawSmallPacket(reinterpret_cast<char *>(EpollClientLoginMaster::loginIsWrongBuffer),sizeof(EpollClientLoginMaster::loginIsWrongBuffer));
        return;
    }
    CharactersGroup::list[charactersGroupIndex]->lockedAccount << characterId;
    EpollClientLoginMaster * gameServer=static_cast<EpollClientLoginMaster *>(CharactersGroup::list.at(charactersGroupIndex)->gameServers.value(serverUniqueKey).link);
    gameServer->trySelectCharacter(this,query_id,serverUniqueKey,charactersGroupIndex,characterId);
}

bool EpollClientLoginMaster::trySelectCharacter(EpollClientLoginMaster * const loginServer,const quint8 &client_query_id,const quint32 &serverUniqueKey,const quint8 &charactersGroupIndex,const quint32 &characterId)
{
    if(queryNumberList.empty())
        return false;
    DataForSelectedCharacterReturn dataForSelectedCharacterReturn;
    dataForSelectedCharacterReturn.loginServer=loginServer;
    dataForSelectedCharacterReturn.client_query_id=client_query_id;
    dataForSelectedCharacterReturn.serverUniqueKey=serverUniqueKey;
    dataForSelectedCharacterReturn.charactersGroupIndex=charactersGroupIndex;
    dataForSelectedCharacterReturn.characterId=characterId;
    loginServerReturnForCharaterSelect << dataForSelectedCharacterReturn;
    //register it
    newFullOutputQuery(0x02,0x05,queryNumberList.back());
    //the data
    EpollClientLoginMaster::selectCharaterRequest[0x02]=queryNumberList.back();
    *reinterpret_cast<quint32 *>(EpollClientLoginMaster::selectCharaterRequest+0x03)=serverUniqueKey;
    EpollClientLoginMaster::selectCharaterRequest[0x07]=charactersGroupIndex;
    *reinterpret_cast<quint32 *>(EpollClientLoginMaster::selectCharaterRequest+0x08)=htole32(characterId);

    queryNumberList.pop_back();
    return internalSendRawSmallPacket(reinterpret_cast<char *>(EpollClientLoginMaster::selectCharaterRequest),sizeof(EpollClientLoginMaster::selectCharaterRequest));
}

void EpollClientLoginMaster::selectCharacter_ReturnToken(const quint8 &query_id,const char * const token)
{
    postReplyData(query_id,token,TOKEN_SIZE);
}

void EpollClientLoginMaster::selectCharacter_ReturnFailed(const quint8 &query_id,const quint8 &errorCode,const quint32 &characterId)
{
    charactersGroupForGameServer->lockedAccount.remove(characterId);
    postReplyData(query_id,reinterpret_cast<const char * const>(&errorCode),1);
}