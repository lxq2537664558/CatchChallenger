#include "MapVisualiserPlayer.h"

#include "../../general/base/MoveOnTheMap.h"

MapVisualiserPlayer::MapVisualiserPlayer(const bool &centerOnPlayer,const bool &debugTags,const bool &useCache,const bool &OpenGL) :
    MapVisualiser(debugTags,useCache,OpenGL)
{
    inMove=false;
    x=0;
    y=0;

    lookToMove.setInterval(200);
    lookToMove.setSingleShot(true);
    connect(&lookToMove,SIGNAL(timeout()),this,SLOT(transformLookToMove()));

    moveTimer.setInterval(250/5);
    moveTimer.setSingleShot(true);
    connect(&moveTimer,SIGNAL(timeout()),this,SLOT(moveStepSlot()));

    this->centerOnPlayer=centerOnPlayer;

    if(centerOnPlayer)
    {
        setSceneRect(-2000,-2000,4000,4000);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    stepAlternance=false;
    animationTileset=new Tiled::Tileset("animation",16,16);
    nextCurrentObject=new Tiled::MapObject();
    grassCurrentObject=new Tiled::MapObject();
    haveGrassCurrentObject=false;
    haveNextCurrentObject=false;
}

MapVisualiserPlayer::~MapVisualiserPlayer()
{
    delete animationTileset;
    delete nextCurrentObject;
    delete grassCurrentObject;
}

void MapVisualiserPlayer::keyPressEvent(QKeyEvent * event)
{
    if(current_map==NULL)
        return;

    //ignore the no arrow key
    if(event->key()!=Qt::Key_Left && event->key()!=Qt::Key_Right && event->key()!=Qt::Key_Up && event->key()!=Qt::Key_Down)
    {
        event->ignore();
        return;
    }

    //ignore the repeated event
    if(event->isAutoRepeat())
        return;

    //add to pressed key list
    keyPressed << event->key();

    //apply the key
    keyPressParse();
}

void MapVisualiserPlayer::keyPressParse()
{
    //ignore is already in move
    if(inMove)
        return;

    if(keyPressed.contains(Qt::Key_Left))
    {
        //already turned on this direction, then try move into this direction
        if(direction==Pokecraft::Direction_look_at_left)
        {
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_left,current_map->logicalMap,x,y,true))
                return;//Can't do at the left!
            //the first step
            direction=Pokecraft::Direction_move_at_left;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(10));
            direction=Pokecraft::Direction_look_at_left;
            lookToMove.start();
        }
    }
    else if(keyPressed.contains(Qt::Key_Right))
    {
        //already turned on this direction, then try move into this direction
        if(direction==Pokecraft::Direction_look_at_right)
        {
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_right,current_map->logicalMap,x,y,true))
                return;//Can't do at the right!
            //the first step
            direction=Pokecraft::Direction_move_at_right;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(4));
            direction=Pokecraft::Direction_look_at_right;
            lookToMove.start();
        }
    }
    else if(keyPressed.contains(Qt::Key_Up))
    {
        //already turned on this direction, then try move into this direction
        if(direction==Pokecraft::Direction_look_at_top)
        {
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_top,current_map->logicalMap,x,y,true))
                return;//Can't do at the top!
            //the first step
            direction=Pokecraft::Direction_move_at_top;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(1));
            direction=Pokecraft::Direction_look_at_top;
            lookToMove.start();
        }
    }
    else if(keyPressed.contains(Qt::Key_Down))
    {
        //already turned on this direction, then try move into this direction
        if(direction==Pokecraft::Direction_look_at_bottom)
        {
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
                return;//Can't do at the bottom!
            //the first step
            direction=Pokecraft::Direction_move_at_bottom;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(7));
            direction=Pokecraft::Direction_look_at_bottom;
            lookToMove.start();
        }
    }
}

void MapVisualiserPlayer::moveStepSlot()
{
    int baseTile=1;
    //move the player for intermediate step and define the base tile (define the stopped step with direction)
    switch(direction)
    {
        case Pokecraft::Direction_move_at_left:
        baseTile=10;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setX(playerMapObject->x()-0.20);
            break;
        }
        break;
        case Pokecraft::Direction_move_at_right:
        baseTile=4;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setX(playerMapObject->x()+0.20);
            break;
        }
        break;
        case Pokecraft::Direction_move_at_top:
        baseTile=1;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setY(playerMapObject->y()-0.20);
            break;
        }
        break;
        case Pokecraft::Direction_move_at_bottom:
        baseTile=7;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setY(playerMapObject->y()+0.20);
            break;
        }
        break;
        default:
        qDebug() << QString("moveStepSlot(): moveStep: %1, wrong direction").arg(moveStep);
        return;
    }

    //apply the right step of the base step defined previously by the direction
    switch(moveStep)
    {
        //stopped step
        case 0:
        playerMapObject->setTile(playerTileset->tileAt(baseTile+0));
        break;
        //transition step
        case 2:
        if(stepAlternance)
            playerMapObject->setTile(playerTileset->tileAt(baseTile-1));
        else
            playerMapObject->setTile(playerTileset->tileAt(baseTile+1));
        stepAlternance=!stepAlternance;
        break;
        //stopped step
        case 4:
        playerMapObject->setTile(playerTileset->tileAt(baseTile+0));
        break;
    }

    if(centerOnPlayer)
        centerOn(MapObjectItem::objectLink[playerMapObject]);
    loadGrassTile();

    moveStep++;

    //if have finish the step
    if(moveStep>5)
    {
        Pokecraft::Map * old_map=&current_map->logicalMap;
        Pokecraft::Map * map=&current_map->logicalMap;
        //set the final value (direction, position, ...)
        switch(direction)
        {
            case Pokecraft::Direction_move_at_left:
            direction=Pokecraft::Direction_look_at_left;
            Pokecraft::MoveOnTheMap::move(Pokecraft::Direction_move_at_left,&map,&x,&y);
            break;
            case Pokecraft::Direction_move_at_right:
            direction=Pokecraft::Direction_look_at_right;
            Pokecraft::MoveOnTheMap::move(Pokecraft::Direction_move_at_right,&map,&x,&y);
            break;
            case Pokecraft::Direction_move_at_top:
            direction=Pokecraft::Direction_look_at_top;
            Pokecraft::MoveOnTheMap::move(Pokecraft::Direction_move_at_top,&map,&x,&y);
            break;
            case Pokecraft::Direction_move_at_bottom:
            direction=Pokecraft::Direction_look_at_bottom;
            Pokecraft::MoveOnTheMap::move(Pokecraft::Direction_move_at_bottom,&map,&x,&y);
            break;
            default:
            qDebug() << QString("moveStepSlot(): moveStep: %1, wrong direction when moveStep>2").arg(moveStep);
            return;
        }
        //if the map have changed
        if(old_map!=map)
        {
            loadOtherMap(map->map_file);
            if(!all_map.contains(map->map_file))
                qDebug() << QString("map changed not located: %1").arg(map->map_file);
            else
            {
                unloadPlayerFromCurrentMap();
                all_map[current_map->logicalMap.map_file]=current_map;
                current_map=all_map[map->map_file];
                loadCurrentMap();
                loadPlayerFromCurrentMap();
            }
        }
        //move to the final position (integer), y+1 because the tile lib start y to 1, not 0
        playerMapObject->setPosition(QPoint(x,y+1));
        if(centerOnPlayer)
            centerOn(MapObjectItem::objectLink[playerMapObject]);
        stopGrassAnimation();

        //check if one arrow key is pressed to continue to move into this direction
        if(keyPressed.contains(Qt::Key_Left))
        {
            //if can go, then do the move
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_left,current_map->logicalMap,x,y,true))
            {
                direction=Pokecraft::Direction_look_at_left;
                playerMapObject->setTile(playerTileset->tileAt(10));
                inMove=false;
            }
            //can go into this direction, then just look into this direction
            else
            {
                direction=Pokecraft::Direction_move_at_left;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Right))
        {
            //if can go, then do the move
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_right,current_map->logicalMap,x,y,true))
            {
                direction=Pokecraft::Direction_look_at_right;
                playerMapObject->setTile(playerTileset->tileAt(4));
                inMove=false;
            }
            //can go into this direction, then just look into this direction
            else
            {
                direction=Pokecraft::Direction_move_at_right;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Up))
        {
            //if can go, then do the move
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_top,current_map->logicalMap,x,y,true))
            {
                direction=Pokecraft::Direction_look_at_top;
                playerMapObject->setTile(playerTileset->tileAt(1));
                inMove=false;
            }
            //can go into this direction, then just look into this direction
            else
            {
                direction=Pokecraft::Direction_move_at_top;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Down))
        {
            //if can go, then do the move
            if(!Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
            {
                direction=Pokecraft::Direction_look_at_bottom;
                playerMapObject->setTile(playerTileset->tileAt(7));
                inMove=false;
            }
            //can go into this direction, then just look into this direction
            else
            {
                direction=Pokecraft::Direction_move_at_bottom;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        //now stop walking, no more arrow key is pressed
        else
        {
            emit send_player_direction(direction);
            inMove=false;
        }
    }
    else
        moveTimer.start();
}

//have look into another direction, if the key remain pressed, apply like move
void MapVisualiserPlayer::transformLookToMove()
{
    if(inMove)
        return;

    switch(direction)
    {
        case Pokecraft::Direction_look_at_left:
        if(keyPressed.contains(Qt::Key_Left) && Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_left,current_map->logicalMap,x,y,true))
        {
            direction=Pokecraft::Direction_move_at_left;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case Pokecraft::Direction_look_at_right:
        if(keyPressed.contains(Qt::Key_Right) && Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_right,current_map->logicalMap,x,y,true))
        {
            direction=Pokecraft::Direction_move_at_right;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case Pokecraft::Direction_look_at_top:
        if(keyPressed.contains(Qt::Key_Up) && Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_top,current_map->logicalMap,x,y,true))
        {
            direction=Pokecraft::Direction_move_at_top;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case Pokecraft::Direction_look_at_bottom:
        if(keyPressed.contains(Qt::Key_Down) && Pokecraft::MoveOnTheMap::canGoTo(Pokecraft::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
        {
            direction=Pokecraft::Direction_move_at_bottom;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        default:
        qDebug() << QString("transformLookToMove(): wrong direction");
        return;
    }
}

void MapVisualiserPlayer::keyReleaseEvent(QKeyEvent * event)
{
    if(current_map==NULL)
        return;

    //ignore the no arrow key
    if(event->key()!=Qt::Key_Left && event->key()!=Qt::Key_Right && event->key()!=Qt::Key_Up && event->key()!=Qt::Key_Down)
    {
        event->ignore();
        return;
    }

    //ignore the repeated event
    if(event->isAutoRepeat())
        return;

    //remove from the key list pressed
    keyPressed.remove(event->key());

    if(keyPressed.size()>0)//another key pressed, repeat
        keyPressParse();
}

QString MapVisualiserPlayer::lastLocation() const
{
    return mLastLocation;
}

void MapVisualiserPlayer::setAnimationTilset(QString animationTilset)
{
    animationTileset->loadFromImage(QImage(":/images/player_default/animation.png"),":/images/player_default/animation.png");
    if(QFile::exists(animationTilset))
        if(!animationTileset->loadFromImage(QImage(animationTilset),animationTilset))
            qDebug() << "Unable the load the datapack animation tileset: " << animationTilset;
}

//call after enter on new map
void MapVisualiserPlayer::loadPlayerFromCurrentMap()
{
    Tiled::ObjectGroup *currentGroup=playerMapObject->objectGroup();
    if(currentGroup!=NULL)
    {
        if(ObjectGroupItem::objectGroupLink.contains(currentGroup))
            ObjectGroupItem::objectGroupLink[currentGroup]->removeObject(playerMapObject);
        //currentGroup->removeObject(playerMapObject);
        if(currentGroup!=current_map->objectGroup)
            qDebug() << QString("loadPlayerFromCurrentMap(), the playerMapObject group is wrong: %1").arg(currentGroup->name());
    }
    if(ObjectGroupItem::objectGroupLink.contains(current_map->objectGroup))
        ObjectGroupItem::objectGroupLink[current_map->objectGroup]->addObject(playerMapObject);
    else
        qDebug() << QString("loadPlayerFromCurrentMap(), ObjectGroupItem::objectGroupLink not contains current_map->objectGroup");
    mLastLocation=current_map->logicalMap.map_file;

    //move to the final position (integer), y+1 because the tile lib start y to 1, not 0
    playerMapObject->setPosition(QPoint(x,y+1));
    if(centerOnPlayer)
        centerOn(MapObjectItem::objectLink[playerMapObject]);
}

//call before leave the old map (and before loadPlayerFromCurrentMap())
void MapVisualiserPlayer::unloadPlayerFromCurrentMap()
{
    //unload the player sprite
    if(ObjectGroupItem::objectGroupLink.contains(playerMapObject->objectGroup()))
        ObjectGroupItem::objectGroupLink[playerMapObject->objectGroup()]->removeObject(playerMapObject);
    else
        qDebug() << QString("unloadPlayerFromCurrentMap(), ObjectGroupItem::objectGroupLink not contains playerMapObject->objectGroup()");
}

void MapVisualiserPlayer::startGrassAnimation(const Pokecraft::Direction &direction)
{
    switch(direction)
    {
        case Pokecraft::Direction_move_at_left:
        case Pokecraft::Direction_move_at_right:
        case Pokecraft::Direction_move_at_top:
        case Pokecraft::Direction_move_at_bottom:
        break;
        default:
        return;
    }

    if(!haveGrassCurrentObject)
    {
        haveGrassCurrentObject=Pokecraft::MoveOnTheMap::haveGrass(current_map->logicalMap,x,y);
        if(haveGrassCurrentObject)
        {
            ObjectGroupItem::objectGroupLink[current_map->objectGroup]->addObject(grassCurrentObject);
            grassCurrentObject->setPosition(QPoint(x,y+1));
            grassCurrentObject->setTile(animationTileset->tileAt(2));
        }
    }
    else
        qDebug() << "haveGrassCurrentObject true here, it's wrong!";

    if(!haveNextCurrentObject)
    {
        haveNextCurrentObject=false;
        Pokecraft::Map * map_destination=&current_map->logicalMap;
        COORD_TYPE x_destination=x;
        COORD_TYPE y_destination=y;
        if(Pokecraft::MoveOnTheMap::move(direction,&map_destination,&x_destination,&y_destination))
            if(loadedNearMap.contains(map_destination->map_file))
                haveNextCurrentObject=Pokecraft::MoveOnTheMap::haveGrass(*map_destination,x_destination,y_destination);
        if(haveNextCurrentObject)
        {
            ObjectGroupItem::objectGroupLink[all_map[map_destination->map_file]->objectGroup]->addObject(nextCurrentObject);
            nextCurrentObject->setPosition(QPoint(x_destination,y_destination+1));
            nextCurrentObject->setTile(animationTileset->tileAt(1));
        }
    }
    else
        qDebug() << "haveNextCurrentObject true here, it's wrong!";
}

void MapVisualiserPlayer::stopGrassAnimation()
{
    if(haveGrassCurrentObject)
    {
        ObjectGroupItem::objectGroupLink[grassCurrentObject->objectGroup()]->removeObject(grassCurrentObject);
        haveGrassCurrentObject=false;
    }
    if(haveNextCurrentObject)
    {
        ObjectGroupItem::objectGroupLink[nextCurrentObject->objectGroup()]->removeObject(nextCurrentObject);
        haveNextCurrentObject=false;
    }
}

void MapVisualiserPlayer::loadGrassTile()
{
    if(haveGrassCurrentObject)
    {
        switch(moveStep)
        {
            case 0:
            case 1:
            break;
            case 2:
                grassCurrentObject->setTile(animationTileset->tileAt(0));
            break;
        }
    }
    if(haveNextCurrentObject)
    {
        switch(moveStep)
        {
            case 0:
            case 1:
            break;
            case 3:
                nextCurrentObject->setTile(animationTileset->tileAt(2));
            break;
        }
    }
}