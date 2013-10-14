#include "BattleAiAction.h"
#include "BattleUnit.h"
#include "BattleObj.h"

CBattleAiAction::~CBattleAiAction()
{
}

int CBattleAiAction::GetCurTick()
{
    return GetBattleObj()->m_iCurrTick;    
}

CBattleObj* CBattleAiAction::GetBattleObj()
{
    return m_holder->GetBattleObj();
}

CBattleAiControler* CBattleAiAction::GetAiControler()
{
    return &m_holder->m_aiControler;
}

bool CBattleAiAction::OnEnter()
{
    m_iEnterTick = GetCurTick();

    return true;    
}

void CBattleAiAction::SiblingDoAi()
{
    for(std::vector<CBattleAiActionList>::iterator iter = m_siblingActionListVec.begin(); iter != m_siblingActionListVec.end(); ++iter)
    {
        CBattleAiActionList &actionList = *iter;
        actionList.DoAi();
    }    
}

bool CBattleAiAction::OnLeave()
{
    m_iLeaveTick = GetCurTick();

    return true;    
}

CBattleAiStandAction::CBattleAiStandAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
}

CBattleAiStandAction::~CBattleAiStandAction()
{
}

bool CBattleAiStandAction::CanDo()
{
    return true;    
}

bool CBattleAiStandAction::Doing()
{
    return true;    
}

bool CBattleAiStandAction::OnEnter()
{
    //进入站立行为
    CBattleAiAction::OnEnter();

    return true;
}

bool CBattleAiStandAction::OnLeave()
{
    //离开站立行为了
    CBattleAiAction::OnLeave();

    return true;    
}

CBattleAiFollowAction::CBattleAiFollowAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
    m_pFollowUnit = NULL;
}

CBattleAiFollowAction::~CBattleAiFollowAction()
{
}

bool CBattleAiFollowAction::CanDo()
{    
    CBattleUnit *pFollowUnit = GetBattleObj()->GetUnit(GetAiControler()->FollowTargetId());

    if(pFollowUnit == NULL)
    {
        return false;
    }

    //检查距离，如果太近，不需要跟随
    int dis = CBattleUtils::GetDistance(m_holder->iPosX, m_holder->iPosY, pFollowUnit->iPosX, pFollowUnit->iPosY);

    //暂时写死吧, 不能位于5格内
    if(dis <= 5)
    {
        return false;
    }

    m_pFollowUnit = pFollowUnit;
    
    return true;    
}

bool CBattleAiFollowAction::Doing()
{
    if(m_holder->iCdTick > GetCurTick())
    {
        return false;
    }
    
    GetBattleObj()->MoveToTarget(m_holder, m_pFollowUnit->iPosX, m_pFollowUnit->iPosY, 5, m_pFollowUnit);
    
    return true;
}

CBattleAiGoBackAction::CBattleAiGoBackAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
}

CBattleAiGoBackAction::~CBattleAiGoBackAction()
{
}

bool CBattleAiGoBackAction::CanDo()
{
    int dis = CBattleUtils::GetDistance(m_holder->iPosX, m_holder->iPosY, m_holder->GetGroup()->iPosX, m_holder->GetGroup()->iPosY);    

    if(dis <= 5)
    {
        return false;
    }

    return true;    
}

bool CBattleAiGoBackAction::Doing()
{
    if(m_holder->iCdTick > GetCurTick())
    {
        return false;
    }
    
    GetBattleObj()->MoveToTarget(m_holder, m_holder->GetGroup()->iPosX, m_holder->GetGroup()->iPosY, 5);

    return true;
}

CBattleAiChangeAction::CBattleAiChangeAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
}

CBattleAiChangeAction::~CBattleAiChangeAction()
{
}

bool CBattleAiChangeAction::CanDo()
{
    int iUnitId = m_holder->FindTargetInRange(m_holder->GetGroup()->iWatchRange);
    
    if(iUnitId == 0)
    {
        return false;
    }

    return true;    
}

bool CBattleAiChangeAction::Doing()
{
    CBattleGroup *pGroup = m_holder->GetGroup();    
    BattleTroopCfg* pTroopCfg = GAME_DATA->cfgBattleTroop.GetCfg(pGroup->m_iDynTrooopId);

    if(pTroopCfg == NULL)
    {
        return false;
    }
    
    pGroup->iPosX = pTroopCfg->stMonsterPos.iPosX;
    pGroup->iPosY = pTroopCfg->stMonsterPos.iPosY;
    pGroup->iOccupyRange = pTroopCfg->stMonsterPos.iRange;
    pGroup->iChaseRange = pTroopCfg->iMoveRange;
    pGroup->iWatchRange = pTroopCfg->iWatchRange;    
    
    for(int i=0;i<pTroopCfg->iMonsterNodeCount;i++)
    {
        for(int j=0;j<pTroopCfg->astMonsterNodes[i].iCount;j++)
        {
            int iMonsterId = pTroopCfg->astMonsterNodes[i].iMonsterId;
            MonsterCfg* pMonsterCfg = GAME_DATA->cfgMonster.GetCfg(iMonsterId);
            
            if(pMonsterCfg==NULL)
            {
                break;
            }
            
            CBattleUnit *pUnit = new CBattleUnit(pGroup);
            pUnit->id(GetBattleObj()->AllocUnitId());
            pUnit->iUnitType = BattleUnitType::Monster;
            pUnit->iUnitCfgId = iMonsterId;            
            pUnit->Set(pMonsterCfg);
            
            if(!pUnit->RandomPos())
            {
                delete pUnit;
                
                break;
            }

            pUnit->iGroupOffsetX = CUtils::RandNumber(-6,6);
            pUnit->iGroupOffsetY = CUtils::RandNumber(-6,6);
            pUnit->m_aiControler.EnableAi(true);
            pUnit->m_aiControler.BuildAiAction(pMonsterCfg->iMonsterType);
            pGroup->m_dynUnitVec.push_back(pUnit);            
        }
    }
    
    return true;
}

CBattleAiCreateUnitAction::CBattleAiCreateUnitAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
}

CBattleAiCreateUnitAction::~CBattleAiCreateUnitAction()
{
}

bool CBattleAiCreateUnitAction::CanDo()
{
    //最多100个，不然太多会卡死前后台
    if(m_holder->GetGroup()->size() >= 100)
    {
        return false;
    }
    
    int iUnitId = m_holder->FindTargetInRange(m_holder->GetGroup()->iWatchRange);
    
    if(iUnitId == 0)
    {
        return false;
    }
    
    return true;
}

bool CBattleAiCreateUnitAction::Doing()
{
    if(GetAiControler()->m_iCreateTick > GetCurTick())
    {
        return false;
    }

    CBattleGroup *pGroup = m_holder->GetGroup();    
    BattleTroopCfg* pTroopCfg = GAME_DATA->cfgBattleTroop.GetCfg(pGroup->m_iDynTrooopId);

    if(pTroopCfg == NULL)
    {
        return false;
    }
    
    pGroup->iPosX = pTroopCfg->stMonsterPos.iPosX;
    pGroup->iPosY = pTroopCfg->stMonsterPos.iPosY;
    pGroup->iOccupyRange = pTroopCfg->stMonsterPos.iRange;
    pGroup->iChaseRange = pTroopCfg->iMoveRange;
    GetAiControler()->m_iCreateTick = GetCurTick() + CONFIG_DATA->m_iServerFPS * 3;
    
    for(int i=0;i<pTroopCfg->iMonsterNodeCount;i++)
    {
        for(int j=0;j<pTroopCfg->astMonsterNodes[i].iCount;j++)
        {
            int iMonsterId = pTroopCfg->astMonsterNodes[i].iMonsterId;
            MonsterCfg* pMonsterCfg = GAME_DATA->cfgMonster.GetCfg(iMonsterId);
            
            if(pMonsterCfg==NULL)
            {
                break;
            }
            
            CBattleUnit *pUnit = new CBattleUnit(pGroup);
            pUnit->id(GetBattleObj()->AllocUnitId());
            pUnit->iUnitType = BattleUnitType::Monster;
            pUnit->iUnitCfgId = iMonsterId;
            pUnit->Set(pMonsterCfg);
            pUnit->iGroupOffsetX = CUtils::RandNumber(-6,6);
            pUnit->iGroupOffsetY = CUtils::RandNumber(-6,6);
            pUnit->m_aiControler.EnableAi(true);
            pUnit->m_aiControler.BuildAiAction(pMonsterCfg->iMonsterType);
            pUnit->iPosX = pGroup->iPosX;
            pUnit->iPosY = pGroup->iPosY;            
            pGroup->m_createUnitVec.push_back(pUnit);
        }
    }
    
    return true;
}

CBattleAiChaseAction::CBattleAiChaseAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
    m_pChaseUnit = NULL;    
}

CBattleAiChaseAction::~CBattleAiChaseAction()
{
}

bool CBattleAiChaseAction::CanDo()
{
    if(GetAiControler()->AttackTargetId() == 0)
    {
        return false;
    }
    
    CBattleUnit *pChaseUnit = GetBattleObj()->GetUnit(GetAiControler()->AttackTargetId());

    if(pChaseUnit == NULL)
    {
        GetAiControler()->AttackTargetId(0);
        
        return false;
    }

    m_pChaseUnit = pChaseUnit;
    
    return true;    
}

bool CBattleAiChaseAction::Doing()
{
    if(m_holder->iCdTick > GetCurTick())
    {
        return false;
    }
    
    GetBattleObj()->MoveToTarget(m_holder, m_pChaseUnit->iPosX, m_pChaseUnit->iPosY, GetBattleObj()->GetNormalAttackDis(m_holder, m_pChaseUnit), m_pChaseUnit);
    
    return true;
}

CBattleAiAttackAction::CBattleAiAttackAction(CBattleUnit *pUnit) : CBattleAiAction(pUnit)
{
    m_pAttackUnit = NULL;    
}

CBattleAiAttackAction::~CBattleAiAttackAction()
{
}

bool CBattleAiAttackAction::OnEnter()
{
    CBattleAiAction::OnEnter();
    //m_holder->iCdTick = GetCurTick() + CONFIG_DATA->m_iServerFPS;

    return true;    
}

bool CBattleAiAttackAction::CanDo()
{
    if(GetAiControler()->AttackTargetId() == 0)
    {
        return false;
    }
    
    CBattleUnit *pAttackUnit = GetBattleObj()->GetUnit(GetAiControler()->AttackTargetId());
    
    if(pAttackUnit == NULL)
    {
        GetAiControler()->AttackTargetId(0);
        
        return false;
    }

    if(pAttackUnit->IsDead())
    {
        GetAiControler()->AttackTargetId(0);

        return false;
    }
    
    int dis = CBattleUtils::GetDistance(m_holder->iPosX, m_holder->iPosY, pAttackUnit->iPosX, pAttackUnit->iPosY);
    
    if(dis > GetBattleObj()->GetNormalAttackDis(m_holder, pAttackUnit))
    {
        return false;
    }
    
    m_pAttackUnit = pAttackUnit;    
    
    return true;
}

bool CBattleAiAttackAction::Doing()
{
    if(m_holder->m_state == battle_unit_state::STATE_MOVE)
    {
        //m_holder->MoveLastPos();
        m_holder->iCdTick = GetCurTick() + CONFIG_DATA->m_iServerFPS;
        
        return false;        
    }
    
    if(m_holder->iCdTick > GetCurTick())
    {
        return false;
    }
    
    GetBattleObj()->AttackTarget(m_holder, m_pAttackUnit);
    
    return true;
}

CBattleAiControler::CBattleAiControler(CBattleUnit *holder)
{
    m_holder = holder;
    m_iFollowTargetId = 0;
    m_iAttackTargetId = 0;
    m_bEnableAi = false;    
    m_iDoAiCdTick = 0;
    m_iCreateTick = 0;    
    RegBuildFuncs();    
}

int CBattleAiControler::GetCurTick()
{
    return m_holder->GetBattleObj()->m_iCurrTick;
}

CBattleAiControler::~CBattleAiControler()
{
    DeleteAction();
}

void CBattleAiControler::DoAi()
{
    if(m_holder->IsDead())
    {
        return;
    }
    
    m_holder->m_moveControler.DoMove();

    //是否开启ai控制
    if(!EnableAi())
    {
        return;
    }

    if(GetCurTick() < m_holder->GetGroup()->m_iDynEffectTick)
    {
        return;
    }
    
    if(AttackTargetId() == 0)
    {
        int iTargetId = 0;
        
        if(GetCurTick() < m_iDoAiCdTick)
        {
            return;
        }

        m_iDoAiCdTick = GetCurTick() + 30;
        
        if(m_holder->IsPriHero() || m_holder->IsChariot())
        {
            //主将的话，查找所有的目标
            iTargetId = m_holder->FindTargetInAll();            
        }
        else
        {
            int iWatchRange = m_holder->GetGroup()->iWatchRange;
            iWatchRange = iWatchRange <= 0 ? 20 : iWatchRange;
            iTargetId = m_holder->FindTargetInRange(iWatchRange);
        }
        
        AttackTargetId(iTargetId);        
    }

    m_firstAiActionList.DoAi();
}

void CBattleAiActionList::DoAi()
{
    for(std::vector<CBattleAiAction*>::iterator iter = m_actionVec.begin(); iter != m_actionVec.end(); ++iter)
    {
        CBattleAiAction *pAction = *iter;

        if(pAction->CanDo())
        {
            if(pAction != m_pCurAiAction)
            {
                if(m_pCurAiAction != NULL)
                {
                    m_pCurAiAction->OnLeave();
                }

                pAction->OnEnter();
            }

            m_pCurAiAction = pAction;
            m_pCurAiAction->Doing();
            m_pCurAiAction->SiblingDoAi();

            break;            
        }
    }
}

void CBattleAiControler::BuildAiAction(int AiId)
{    
    if(AiId >= battle_ai::MAX)
    {
        return;
    }

    (this->*m_funcArr[AiId])();
}

void CBattleAiControler::RegBuildFuncs()
{
    m_funcArr[0] = &CBattleAiControler::BuildAi_0;
    m_funcArr[1] = &CBattleAiControler::BuildAi_1;
    m_funcArr[2] = &CBattleAiControler::BuildAi_2;
    m_funcArr[3] = &CBattleAiControler::BuildAi_3;
    m_funcArr[4] = &CBattleAiControler::BuildAi_4;    
}

CBattleAiActionList CBattleAiControler::BuildActionList(CBattleAiAction **pActionArr, int iArrLen)
{
    CBattleAiActionList actionList;
    
    for(int i = 0; i != iArrLen; ++i)
    {
        CBattleAiAction *pAction = pActionArr[i];
        AddAction(pAction);
        actionList.AddAction(pAction);
    }

    return actionList;    
}

//常规ai
void CBattleAiControler::BuildAi_0()
{
    CBattleAiAction* actionList[] = {
        new CBattleAiAttackAction(m_holder),
        new CBattleAiChaseAction(m_holder),        
        new CBattleAiStandAction(m_holder)
    };

    m_firstAiActionList = BuildActionList(actionList, 3);
}

//变身怪ai
void CBattleAiControler::BuildAi_1()
{
    CBattleAiAction* actionList[] = {
        new CBattleAiChangeAction(m_holder),
        new CBattleAiStandAction(m_holder)
    };

    m_firstAiActionList = BuildActionList(actionList, 2);
}

//兵营ai
void CBattleAiControler::BuildAi_2()
{
    CBattleAiAction* actionList[] = {
        new CBattleAiCreateUnitAction(m_holder),
        new CBattleAiStandAction(m_holder)
    };

    m_firstAiActionList = BuildActionList(actionList, 2);
}

//静态物，比如城门，可以被攻击，不能攻击
void CBattleAiControler::BuildAi_3()
{
    CBattleAiAction* actionList[] = {
        new CBattleAiStandAction(m_holder)
    };

    m_firstAiActionList = BuildActionList(actionList, 1);
}

//静态物，比如防御塔，可以攻击，不能行走
void CBattleAiControler::BuildAi_4()
{
    CBattleAiAction* actionList[] = {
        new CBattleAiAttackAction(m_holder),
        new CBattleAiStandAction(m_holder)
    };

    m_firstAiActionList = BuildActionList(actionList, 2);
}

void CBattleAiControler::AddAction(CBattleAiAction *pAction)
{
    m_allActionSet.insert(pAction);
}

void CBattleAiControler::DeleteAction()
{
    for(std::set<CBattleAiAction*>::iterator iter = m_allActionSet.begin(); iter != m_allActionSet.end(); ++iter)
    {
        delete *iter;        
    }    
}
