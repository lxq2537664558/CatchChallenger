#ifndef POKECRAFT_MAPCONTROLLERMP_H
#define POKECRAFT_MAPCONTROLLERMP_H

#include "../../client/base/render/MapVisualiserPlayer.h"
#include "../../client/base/Api_protocol.h"

#include <QString>
#include <QList>
#include <QStringList>
#include <QHash>
#include <QTimer>

class MapControllerMP : public MapVisualiserPlayer
{
    Q_OBJECT
public:
    explicit MapControllerMP(Pokecraft::Api_protocol *client, const bool &centerOnPlayer=true, const bool &debugTags=false, const bool &useCache=true, const bool &OpenGL=false);
    ~MapControllerMP();

    virtual void resetAll();
    void setScale(int scaleSize);
public slots:
    //map move
    void insert_player(const Pokecraft::Player_public_informations &player,const quint32 &mapId,const quint16 &x,const quint16 &y,const Pokecraft::Direction &direction);
    void move_player(const quint16 &id,const QList<QPair<quint8,Pokecraft::Direction> > &movement);
    void remove_player(const quint16 &id);
    void reinsert_player(const quint16 &id,const quint8 &x,const quint8 &y,const Pokecraft::Direction &direction);
    void reinsert_player(const quint16 &id,const quint32 &mapId,const quint8 &x,const quint8 &y,const Pokecraft::Direction &direction);
    void dropAllPlayerOnTheMap();
    void teleportTo(const quint32 &mapId,const quint16 &x,const quint16 &y,const Pokecraft::Direction &direction);

    //player info
    void have_current_player_info(const Pokecraft::Player_private_and_public_informations &informations);

    //the datapack
    void setDatapackPath(const QString &path);
    virtual void datapackParsed();
    virtual void reinject_signals();
private:
    //the other player
    struct OtherPlayer
    {
        Tiled::MapObject * playerMapObject;
        Tiled::Tileset * playerTileset;
        int moveStep;
        Pokecraft::Direction direction;
        quint8 x,y;
        bool inMove;
        bool stepAlternance;
        QString current_map;
        QSet<QString> mapUsed;
        Pokecraft::Player_public_informations informations;

        //presumed map
        Map_full *presumed_map;
        quint8 presumed_x,presumed_y;
        Pokecraft::Direction presumed_direction;
        QTimer *oneStepMore;
    };
    QHash<quint16,OtherPlayer> otherPlayerList;
    QHash<QTimer *,quint16> otherPlayerListByTimer;
    QHash<QString,quint16> mapUsedByOtherPlayer;

    Pokecraft::Api_protocol *client;

    //datapack
    QStringList skinFolderList;

    //the delayed action
    struct DelayedInsert
    {
        Pokecraft::Player_public_informations player;
        quint32 mapId;
        quint16 x;
        quint16 y;
        Pokecraft::Direction direction;
    };
    struct DelayedMove
    {
        quint16 id;
        QList<QPair<quint8,Pokecraft::Direction> > movement;
    };
    struct DelayedReinsertSingle
    {
        quint16 id;
        quint8 x;
        quint8 y;
        Pokecraft::Direction direction;
    };
    struct DelayedReinsertFull
    {
        quint16 id;
        quint32 mapId;
        quint8 x;
        quint8 y;
        Pokecraft::Direction direction;
    };
    enum DelayedType
    {
        DelayedType_Insert,
        DelayedType_Move,
        DelayedType_Remove,
        DelayedType_Reinsert_single,
        DelayedType_Reinsert_full,
        DelayedType_Drop_all
    };
    struct DelayedMultiplex
    {
        DelayedType type;
        DelayedInsert insert;
        DelayedMove move;
        quint16 remove;
        DelayedReinsertSingle reinsert_single;
        DelayedReinsertFull reinsert_full;
    };
    QList<DelayedMultiplex> delayedActions;

    struct DelayedTeleportTo
    {
        quint32 mapId;
        quint16 x;
        quint16 y;
        Pokecraft::Direction direction;
    };
    QList<DelayedTeleportTo> delayedTeleportTo;
protected:
    bool mHaveTheDatapack;

    //current player
    Pokecraft::Player_private_and_public_informations player_informations;
    bool player_informations_is_set;

    //datapack
    QString datapackPath;
    QString datapackMapPath;
private slots:
    bool loadPlayerMap(const QString &fileName,const quint8 &x,const quint8 &y);
    virtual void removeUnusedMap();
    void moveOtherPlayerStepSlot();
protected slots:
    //call after enter on new map
    virtual void loadOtherPlayerFromMap(OtherPlayer otherPlayer);
    //call before leave the old map (and before loadPlayerFromCurrentMap())
    virtual void unloadOtherPlayerFromMap(OtherPlayer otherPlayer);
signals:
    void teleportDone();
};

#endif // POKECRAFT_MAPCONTROLLERMP_H