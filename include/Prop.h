#pragma once
#include <memory>
#include "Throw.h"
class Hand;


/// This is a base class for an object being juggled
class Prop
{
public:
         
    Prop(int id);
    virtual ~Prop(void);

    //Properties
    int get_id()        {return id_;}


    /** 
     * Get the string of the current state name
     */
    const TCHAR* getStateName();
    
    /**
     * The current count away from the next hand
     * @return If the state is Flight then the number of counts away from its 
     *         destination Hand, else the value is 0;
     * @remark The value is set by the Throw and decremented each time Count is called 
     */
    int getCurrentSwap();

    /**
     * 
     */
    bool isDropped();
    bool isInFlight();

    virtual void Toss(Throw* pass);

    virtual void Catch();

    virtual void Collision();

    virtual void Pickup();

    virtual void Tick();

    
    void ConnectToToss(IdSlot slot);
    void DisconnectFromToss(IdSlot slot);

    void ConnectToDrop(IdSlot slot);
    void DisconnectFromDrop(IdSlot slot);

    void ConnectToCatch(PropSlot slot);
    void DisconnectFromCatch(PropSlot slot);

    void ConnectToAll(IdSlot tossSlot, IdSlot dropSlot, PropSlot propSlot);
    void DisonnectFromAll(IdSlot tossSlot, IdSlot dropSlot, PropSlot propSlot);

    void Tossed(int id);
    void Catch(int id);
    void Dropped(int id);
protected:
    //void DecrementSiteswap(int id);

    bool isIdValid(int id);

    int getState();

    Throw toss_;
private:
    IdPublisher tossed_;
    PropPublisher ready_to_be_caught_;
    IdPublisher dropped_;


    int id_;
    struct PropStateMachine;
    std::shared_ptr<PropStateMachine> stateMachine_;

};

typedef std::shared_ptr<Prop> PropPtr;
