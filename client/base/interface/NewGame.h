#ifndef NEWGAME_H
#define NEWGAME_H

#include <QDialog>
#include <QTimer>
#include "../../general/base/GeneralStructures.h"

namespace Ui {
class NewGame;
}

class NewGame : public QDialog
{
    Q_OBJECT

public:
    explicit NewGame(const QString &skinPath, const QString &monsterPath, std::vector<std::vector<CatchChallenger::Profile::Monster> > monstergroup, const std::vector<uint8_t> &forcedSkin, QWidget *parent = 0);
    ~NewGame();
    bool haveTheInformation();
    QString pseudo();
    QString skin();
    uint8_t skinId();
    uint8_t monsterGroupId();
    bool haveSkin();
    void updateSkin();
private slots:
    void on_ok_clicked();
    void on_pseudo_textChanged(const QString &arg1);
    void on_pseudo_returnPressed();
    void on_nextSkin_clicked();
    void on_previousSkin_clicked();
    void timerSlot();
private:
    std::vector<uint8_t> forcedSkin;
    QString monsterPath;
    std::vector<std::vector<CatchChallenger::Profile::Monster> > monstergroup;
    Ui::NewGame *ui;
    enum Step
    {
        Step1,
        Step2,
        StepOk,
    };
    Step step;
    bool okAccepted;
    bool skinLoaded;
    std::vector<std::string> skinList;
    std::vector<uint8_t> skinListId;
    unsigned int currentSkin;
    unsigned int currentMonsterGroup;
    std::string skinPath;
    bool okCanBeEnabled();

    QTimer timer;
};

#endif // NEWGAME_H
