#include "../base/LocalClientHandler.h"
#include "../../general/base/ProtocolParsing.h"
#include "../base/GlobalServerData.h"

using namespace Pokecraft;

void LocalClientHandler::getRandomNumberIfNeeded()
{
    if(randomSeeds.size()<=POKECRAFT_SERVER_MIN_RANDOM_LIST_SIZE)
        emit askRandomNumber();
}

void LocalClientHandler::newRandomNumber(const QByteArray &randomData)
{
    randomSeeds+=randomData;
}

void LocalClientHandler::tryEscape()
{
    if(wildMonsters.empty())//check if is in fight
    {
        emit error(QString("error: tryEscape() when is not in fight"));
        return;
    }
    if(tryEscapeInternal())
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("escape is successful"));
        #endif
        wildMonsters.clear();
        wildMonstersStat.clear();
    }
    else
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("escape is failed"));
        #endif
        generateOtherAttack();
        checkKOMonsters();
    }
}

void LocalClientHandler::checkKOMonsters()
{
    if(player_informations->public_and_private_informations.playerMonster[selectedMonster].hp==0)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("You current monster (%1) is KO").arg(player_informations->public_and_private_informations.playerMonster[selectedMonster].monster));
        #endif
        updateCanDoFight();
        if(!ableToFight)
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("Player have lost, tp and heal"));
            #endif
            //teleport
            emit teleportTo(player_informations->rescue.map,player_informations->rescue.x,player_informations->rescue.y,player_informations->rescue.orientation);
            //regen all the monsters
            int index=0;
            int size=player_informations->public_and_private_informations.playerMonster.size();
            while(index<size)
            {
                player_informations->public_and_private_informations.playerMonster[index].hp=
                        GlobalServerData::serverPrivateVariables.monsters[player_informations->public_and_private_informations.playerMonster[index].monster].stat.hp*
                        player_informations->public_and_private_informations.playerMonster[index].level/POKECRAFT_MONSTER_LEVEL_MAX;
                index++;
            }
            updateCanDoFight();
            #ifdef POKECRAFT_EXTRA_CHECK
            if(!ableToFight)
            {
                emit error(QString("after lost in fight, remain unable to do a fight"));
                return;
            }
            #endif
            return;
        }
    }
    if(wildMonsters.first().hp==0)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("The wild monster (%1) is KO").arg(wildMonsters.first().monster));
        #endif
        wildMonsters.removeFirst();
        wildMonstersStat.removeFirst();
        //drop the drop item here
    }
}

bool LocalClientHandler::checkFightCollision(Map *map,const COORD_TYPE &x,const COORD_TYPE &y)
{
    bool ok;
    if(!wildMonsters.empty())
    {
        emit error(QString("error: map: %1 (%2,%3), is in fight").arg(map->map_file).arg(x).arg(y));
        return false;
    }
    if(Pokecraft::MoveOnTheMap::isGrass(*map,x,y) && !map->grassMonster.empty())
    {
        if(!ableToFight)
        {
            emit error(QString("LocalClientHandler::singleMove(), can't walk into the grass into map: %1(%2,%3)").arg(map->map_file).arg(x).arg(y));
            return false;
        }
        if(stepFight_Grass==0)
        {
            if(randomSeeds.size()==0)
            {
                emit error(QString("error: no more random seed here, map: %1 (%2,%3), is in fight").arg(map->map_file).arg(x).arg(y));
                return false;
            }
            else
                stepFight_Grass=getOneSeed()%16;
        }
        else
            stepFight_Grass--;
        if(stepFight_Grass==0)
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("Start grass fight"));
            #endif
            PlayerMonster monster=getRandomMonster(map->grassMonster,&ok);
            if(ok)
                wildMonsters << monster;
            else
                emit error(QString("error: no more random seed here to have the get"));
            return ok;
        }
        else
            return false;
    }
    if(Pokecraft::MoveOnTheMap::isWater(*map,x,y) && !map->waterMonster.empty())
    {
        if(!ableToFight)
        {
            emit error(QString("LocalClientHandler::singleMove(), can't walk into the grass into map: %1(%2,%3)").arg(map->map_file).arg(x).arg(y));
            return false;
        }
        if(stepFight_Water==0)
        {
            if(randomSeeds.size()==0)
            {
                emit error(QString("error: no more random seed here, map: %1 (%2,%3), is in fight").arg(map->map_file).arg(x).arg(y));
                return false;
            }
            else
                stepFight_Water=getOneSeed()%16;
        }
        else
            stepFight_Water--;
        if(stepFight_Water==0)
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("Start wather fight"));
            #endif
            PlayerMonster monster=getRandomMonster(map->waterMonster,&ok);
            if(ok)
                wildMonsters << monster;
            else
                emit error(QString("error: no more random seed here to have the get"));
            return ok;
        }
        else
            return false;
    }
    if(!map->caveMonster.empty())
    {
        if(!ableToFight)
        {
            emit error(QString("LocalClientHandler::singleMove(), can't walk into the grass into map: %1(%2,%3)").arg(map->map_file).arg(x).arg(y));
            return false;
        }
        if(stepFight_Cave==0)
        {
            if(randomSeeds.size()==0)
            {
                emit error(QString("error: no more random seed here, map: %1 (%2,%3), is in fight").arg(map->map_file).arg(x).arg(y));
                return false;
            }
            else
                stepFight_Cave=getOneSeed()%16;
        }
        else
            stepFight_Cave--;
        if(stepFight_Cave==0)
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("Start cave fight"));
            #endif
            PlayerMonster monster=getRandomMonster(map->caveMonster,&ok);
            if(ok)
                wildMonsters << monster;
            else
                emit error(QString("error: no more random seed here to have the get"));
            return ok;
        }
        else
            return false;
    }

    /// no fight in this zone
    return false;
}

quint8 LocalClientHandler::getOneSeed(const quint8 &max)
{
    quint16 number=static_cast<quint8>(randomSeeds.at(0));
    if(max!=0)
    {
        number*=max;
        number/=255;
    }
    randomSeeds.remove(0,1);
    return number;
}

PlayerMonster LocalClientHandler::getRandomMonster(const QList<MapMonster> &monsterList,bool *ok)
{
    PlayerMonster playerMonster;
    playerMonster.captured_with=0;
    playerMonster.egg_step=0;
    playerMonster.remaining_xp=0;
    playerMonster.sp=0;
    quint8 randomMonsterInt=getOneSeed()%100;
    bool monsterFound=false;
    int index=0;
    while(index<monsterList.size())
    {
        int luck=monsterList.at(index).luck;
        if(randomMonsterInt<luck)
        {
            //it's this monster
            playerMonster.monster=monsterList.at(index).id;
            //select the level
            if(monsterList.at(index).maxLevel==monsterList.at(index).minLevel)
                playerMonster.level=monsterList.at(index).minLevel;
            else
            {
                playerMonster.level=getOneSeed()%(monsterList.at(index).maxLevel-monsterList.at(index).minLevel+1)+monsterList.at(index).minLevel;
            }
            monsterFound=true;
            break;
        }
        else
            randomMonsterInt-=luck;
        index++;
    }
    if(!monsterFound)
    {
        emit error(QString("error: no wild monster selected"));
        *ok=false;
        playerMonster.monster=0;
        playerMonster.level=0;
        playerMonster.gender=PlayerMonster::Unknown;
        return playerMonster;
    }
    Monster monsterDef=GlobalServerData::serverPrivateVariables.monsters[playerMonster.monster];
    if(monsterDef.ratio_gender>0 && monsterDef.ratio_gender<100)
    {
        qint8 temp_ratio=getOneSeed()%101;
        if(temp_ratio<monsterDef.ratio_gender)
            playerMonster.gender=PlayerMonster::Male;
        else
            playerMonster.gender=PlayerMonster::Female;
    }
    else
    {
        switch(monsterDef.ratio_gender)
        {
            case 0:
                playerMonster.gender=PlayerMonster::Male;
            break;
            case 100:
                playerMonster.gender=PlayerMonster::Female;
            break;
            default:
                playerMonster.gender=PlayerMonster::Unknown;
            break;
        }
    }
    Monster::Stat monsterStat=getStat(monsterDef,playerMonster.level);
    playerMonster.hp=monsterStat.hp;
    index=monsterDef.attack.size()-1;
    while(index>=0 && playerMonster.skills.size()<POKECRAFT_MONSTER_WILD_SKILL_NUMBER)
    {
        if(monsterDef.attack.at(index).level<=playerMonster.level)
            playerMonster.skills << monsterDef.attack.at(index).skill;
        index--;
    }
    *ok=true;
    wildMonstersStat << monsterStat;
    return playerMonster;
}

/** \warning you need check before the input data */
Monster::Stat LocalClientHandler::getStat(const Monster &monster, const quint8 &level)
{
    Monster::Stat stat=monster.stat;
    stat.attack=stat.attack*level/POKECRAFT_MONSTER_LEVEL_MAX;
    stat.defense=stat.defense*level/POKECRAFT_MONSTER_LEVEL_MAX;
    stat.hp=stat.hp*level/POKECRAFT_MONSTER_LEVEL_MAX;
    stat.special_attack=stat.special_attack*level/POKECRAFT_MONSTER_LEVEL_MAX;
    stat.special_defense=stat.special_defense*level/POKECRAFT_MONSTER_LEVEL_MAX;
    stat.speed=stat.speed*level/POKECRAFT_MONSTER_LEVEL_MAX;
    return stat;
}

bool LocalClientHandler::tryEscapeInternal()
{
    quint8 value=getOneSeed()%101;
    if(wildMonsters.first().level<player_informations->public_and_private_informations.playerMonster.at(selectedMonster).level && value<75)
        return true;
    if(wildMonsters.first().level==player_informations->public_and_private_informations.playerMonster.at(selectedMonster).level && value<50)
        return true;
    if(wildMonsters.first().level>player_informations->public_and_private_informations.playerMonster.at(selectedMonster).level && value<25)
        return true;
    return false;
}

void LocalClientHandler::updateCanDoFight()
{
    ableToFight=false;
    int index=0;
    while(index<player_informations->public_and_private_informations.playerMonster.size())
    {
        const PlayerMonster &playerMonsterEntry=player_informations->public_and_private_informations.playerMonster.at(index);
        if(playerMonsterEntry.hp>0 && playerMonsterEntry.egg_step==0)
        {
            selectedMonster=index;
            ableToFight=true;
            return;
        }
        index++;
    }
}

void LocalClientHandler::generateOtherAttack()
{
    const PlayerMonster &otherMonster=wildMonsters.first();
    if(otherMonster.skills.empty())
    {
        emit message(QString("Unable to fight because the other monster (%1, level: %2) have no skill").arg(otherMonster.monster).arg(otherMonster.level));
        return;
    }
    int position;
    if(otherMonster.skills.size()==1)
        position=0;
    else
        position=getOneSeed()%otherMonster.skills.size();
    const PlayerMonster::Skill &otherMonsterSkill=otherMonster.skills.at(position);
    const Monster::Skill::SkillList &skillList=GlobalServerData::serverPrivateVariables.monsterSkills[otherMonsterSkill.skill].level.at(otherMonsterSkill.level-1);
    int index=0;
    while(index<skillList.buff.size())
    {
        const Monster::Skill::Buff &buff=skillList.buff.at(index);
        bool success;
        if(buff.success==100)
            success=true;
        else
            success=(getOneSeed(100)<buff.success);
        if(success)
        {
            applyOtherBuffEffect(buff.effect);
        }
        index++;
    }
    index=0;
    while(index<skillList.life.size())
    {
        const Monster::Skill::Life &life=skillList.life.at(index);
        bool success;
        if(life.success==100)
            success=true;
        else
            success=(getOneSeed(100)<life.success);
        if(success)
            applyOtherLifeEffect(life.effect);
        index++;
    }
}

void LocalClientHandler::applyOtherLifeEffect(const Monster::Skill::LifeEffect &effect)
{
    qint32 quantity;
    Monster::Stat stat;
    switch(effect.on)
    {
        case Monster::ApplyOn_AloneEnemy:
        case Monster::ApplyOn_AllEnemy:
            if(effect.type==QuantityType_Quantity)
                quantity=effect.quantity;
            else
                quantity=(player_informations->public_and_private_informations.playerMonster[selectedMonster].hp*effect.quantity)/100;
            stat=getStat(GlobalServerData::serverPrivateVariables.monsters[player_informations->public_and_private_informations.playerMonster[selectedMonster].monster],player_informations->public_and_private_informations.playerMonster[selectedMonster].level);
            if(quantity<0 && (-quantity)>player_informations->public_and_private_informations.playerMonster[selectedMonster].hp)
                player_informations->public_and_private_informations.playerMonster[selectedMonster].hp=0;
            else if(quantity>0 && quantity>(stat.hp-player_informations->public_and_private_informations.playerMonster[selectedMonster].hp))
                player_informations->public_and_private_informations.playerMonster[selectedMonster].hp=stat.hp;
            else
                player_informations->public_and_private_informations.playerMonster[selectedMonster].hp+=quantity;
        break;
        case Monster::ApplyOn_Themself:
        case Monster::ApplyOn_AllAlly:
            if(effect.type==QuantityType_Quantity)
                quantity=effect.quantity;
            else
                quantity=(wildMonsters.first().hp*effect.quantity)/100;
            stat=getStat(GlobalServerData::serverPrivateVariables.monsters[wildMonsters.first().monster],wildMonsters.first().level);
            if(quantity<0 && (-quantity)>wildMonsters.first().hp)
                wildMonsters.first().hp=0;
            else if(quantity>0 && quantity>(stat.hp-wildMonsters.first().hp))
                wildMonsters.first().hp=stat.hp;
            else
                wildMonsters.first().hp+=quantity;
        break;
        default:
            qDebug() << "Not apply match, can't apply the buff";
        break;
    }
}

void LocalClientHandler::applyOtherBuffEffect(const Monster::Skill::BuffEffect &effect)
{
    PlayerMonster::Buff tempBuff;
    tempBuff.buff=effect.buff;
    tempBuff.level=effect.level;
    switch(effect.on)
    {
        case Monster::ApplyOn_AloneEnemy:
        case Monster::ApplyOn_AllEnemy:
            player_informations->public_and_private_informations.playerMonster[selectedMonster].buffs << tempBuff;
        break;
        case Monster::ApplyOn_Themself:
        case Monster::ApplyOn_AllAlly:
            wildMonsters.first().buffs << tempBuff;
        break;
        default:
            qDebug() << "Not apply match, can't apply the buff";
        break;
    }
}