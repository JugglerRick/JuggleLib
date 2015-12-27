#include "pch.h"
#include "JuggleLib.h"
#include "common_state_machine.h"
#include "Hand.h"
#include <iostream>
#include <typeinfo>



namespace 
{
    using namespace StateMachine;
    BOOST_MSM_EUML_DECLARE_ATTRIBUTE(Prop*, prop_);
    BOOST_MSM_EUML_DECLARE_ATTRIBUTE(PropPublisher, tossed_);
    BOOST_MSM_EUML_DECLARE_ATTRIBUTE(PropPublisher, catch_);
    BOOST_MSM_EUML_DECLARE_ATTRIBUTE(PropPublisher, dropped_);

    BOOST_MSM_EUML_FLAG(isDroppedFlag_);
    BOOST_MSM_EUML_FLAG(isInFlightFlag_);

    BOOST_MSM_EUML_EVENT_WITH_ATTRIBUTES(pickupEvent, handAttributes)
    BOOST_MSM_EUML_EVENT_WITH_ATTRIBUTES(caughtEvent, handAttributes)

    template<class FSM>
    void connectHandToFsm(FSM& fsm, Hand* hand)
    { 
        if(nullptr == hand){
            return;
        }
        fsm.get_attribute(Ahand) = hand;
        PropSlot slot(nullptr);
        slot = (std::bind(&Hand::Catch, hand, std::placeholders::_1));
        fsm.get_attribute(catch_).connect(slot);
        slot = (std::bind(&Hand::Collision, hand, std::placeholders::_1));
        fsm.get_attribute(dropped_).connect(slot);
    }

    template<class FSM>
    void disconnectHandFromFsm(FSM& fsm)
    {
        Hand* hand = fsm.get_attribute(Ahand);
        if(nullptr == hand){
            return;
        }
        fsm.get_attribute(Ahand) = nullptr;
        PropSlot slot(nullptr);
        slot = (std::bind(&Hand::Catch, hand, std::placeholders::_1));
        fsm.get_attribute(catch_).disconnect(slot);
        slot = (std::bind(&Hand::Collision, hand, std::placeholders::_1));
        fsm.get_attribute(dropped_).disconnect(slot);
    }


    /* 
     *  The catch state and support methods
     */

    BOOST_MSM_EUML_ACTION(catch_entry_action)
    {
        template <class Event, class FSM, class STATE>
        void operator()(Event const& evt, FSM& fsm, STATE& state)
        {
            DebugOut() << "PropStateMachine::catch_entry_action";
        }
    };
    BOOST_MSM_EUML_ACTION(catch_exit_action)
    {
        template <class FSM, class EVT, class State>
        void operator()(EVT const& evt, FSM& fsm, State& state)
        {
            DebugOut(_T("PropStateMachine::catch_exit_action"));
        }
    };
   
    BOOST_MSM_EUML_STATE(
    (
        catch_entry_action, 
        catch_exit_action
    ), CATCH)

    /* 
     *  The dropped state and support methods
     */

    BOOST_MSM_EUML_ACTION(dropped_entry)
    {
        template <class Event, class FSM, class STATE>
        void operator()(Event const& evt, FSM& fsm, STATE& state)
        {
            DebugOut(_T("PropStateMachine::dropped_entry"));
            if(!fsm.get_attribute(dropped_).empty()){
                fsm.get_attribute(dropped_)(fsm.get_attribute(prop_));
            }
            disconnectHandFromFsm(fsm);
        }
    };

    BOOST_MSM_EUML_STATE(
    (
        dropped_entry,
        no_action,
        attributes_ << no_attributes_,
        configure_ << isDroppedFlag_
    ), DROPPED)
 
    /* 
     *  The flight state and support methods
     */

    BOOST_MSM_EUML_ACTION(flight_entry_action)
    {
        template <class FSM,class EVT,class State>
        void operator()(EVT const& evt ,FSM& fsm, State& state )
        {
            DebugOut(_T("PropStateMachine::flight_entry_action"));
            Throw* destinationToss(fsm.get_attribute(Atoss));
            Throw* sourceToss(evt.get_attribute(Atoss));
            PropSlot slot(nullptr);
            if(nullptr != destinationToss){
                if(nullptr != sourceToss){
                    *destinationToss = *sourceToss;
                }
                if(nullptr != destinationToss->destination){
                    if(nullptr != fsm.get_attribute(Ahand)){
                        disconnectHandFromFsm(fsm);
                    }
                    connectHandToFsm(fsm, destinationToss->destination);
                }
            }
            if(!fsm.get_attribute(tossed_).empty()){
                fsm.get_attribute(tossed_)(fsm.get_attribute(prop_));
            }
        }
    };

    BOOST_MSM_EUML_ACTION(flight_exit_action)
    {
        template <class Event, class FSM, class STATE>
        void operator()(Event const& evt, FSM& fsm, STATE& state)
        {
            DebugOut(_T("PropStateMachine::flight_exit_action"));
            if(!fsm.get_attribute(catch_).empty()){
                fsm.get_attribute(catch_)(fsm.get_attribute(prop_));
            }
        }
    };

    BOOST_MSM_EUML_ACTION(tick_action)
    {
        template <class FSM, class EVT, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState& source, TargetState& target )
        {
            DebugOut(_T("PropStateMachine::tick_action"));
            Throw* toss(fsm.get_attribute(Atoss));
            if(nullptr != toss){
                if(0 < toss->siteswap){
                    --toss->siteswap;
                }
            }
            
        }
    };

    BOOST_MSM_EUML_STATE(
    (
        flight_entry_action,
        flight_exit_action,
        attributes_ << Atoss ,
        configure_ << isInFlightFlag_
    ), FLIGHT)

    BOOST_MSM_EUML_ACTION(tick_guard)
    {
        template <class FSM, class EVT, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM& fsm, SourceState& source, TargetState& target )
        {
            bool bRet(false);
            Throw* toss(fsm.get_attribute(Atoss));
            if(nullptr != toss)
            {
                bRet =  0 == toss->siteswap;
            }
            return bRet;
        }
    };


    BOOST_MSM_EUML_ACTION(pickup_action)
    {
        template <class FSM, class EVT, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState& source, TargetState& target )
        {
            Hand* hand(evt.get_attribute(Ahand));
            if(nullptr != hand){
                connectHandToFsm(fsm, hand);
            }
        }
    };



    /**
     *  Prop state machine transition table
     */

    BOOST_MSM_EUML_TRANSITION_TABLE(
    (
        DWELL + tossEvent                       == FLIGHT,
        FLIGHT + tickEvent / tick_action,
        FLIGHT [tick_guard]                     == CATCH,
        CATCH + caughtEvent                     == DWELL,
        CATCH + tickEvent                       == DROPPED,
        DROPPED + pickupEvent / pickup_action   == DWELL,
        DWELL + collisionEvent                  == DROPPED,
        FLIGHT + collisionEvent                 == DROPPED,
        CATCH + collisionEvent                  == DROPPED,
        DROPPED + collisionEvent
    ), prop_transition_table)

    /**
     * Invalid transistion handler
     * in this case process a collisionEvent to force the Dropped state
     */
    BOOST_MSM_EUML_ACTION(invalid_state_transistion)
    {
        template <class FSM,class Event>
        void operator()(Event const& e,FSM& fsm,int state)
        {
            DebugOut() << "PropStateMachine::invald_state_transistion: by event: " << typeid(e).name() << "with PropMachine state: " << state; 
            fsm.process_event(collisionEvent);
        }
    };

    /**
     * The declaration of the actual state machine type
     */
    BOOST_MSM_EUML_DECLARE_STATE_MACHINE( 
    (
        prop_transition_table, 
        init_ << DROPPED,
        no_action,
        no_action,
        attributes_<< Aid << prop_ << tossed_ << catch_ << dropped_ << Atoss << Ahand,
        configure_ << no_configure_,
        invalid_state_transistion
    ), prop_state_machine)

    // the type for the state machine

    typedef msm::back::state_machine<prop_state_machine> Base;
    const TCHAR* stateNames[] = {
        TEXT("Dwell"),
        TEXT("Flight"),
        TEXT("Catch"),
        TEXT("Dropped"),
    };

}

/**
 *
 */

struct Prop::PropStateMachine : public Base
{
    PropStateMachine(int id, Prop* prop)
    {
        get_attribute(StateMachine::Aid) = id;
        get_attribute(prop_) = prop;
        get_attribute(Ahand) = nullptr;
    }
};

/**
 *
 */

Prop::Prop(int id)
:   stateMachine_(new Prop::PropStateMachine(id, this) )
,   id_(id)
{
    stateMachine_->get_attribute(StateMachine::Atoss) = &toss_;
}


/**
 *
 */

Prop::~Prop(void)
{
}

/**
 *
 */

Prop::State Prop::getState()
{
    return static_cast<Prop::State>((*(stateMachine_->current_state())));
}


/**
 *
 */

const TCHAR* Prop::getStateName()
{
    return  stateNames[(*(stateMachine_->current_state()))];
}

/**
 *
 */

bool Prop::isDropped()
{
    return stateMachine_->is_flag_active<isDroppedFlag__helper>();
}

/**
 *
 */

bool Prop::isInFlight()
{
    return stateMachine_->is_flag_active<isInFlightFlag__helper>();
}
        
/**
 *
 */

int Prop::getCurrentSwap()
{
    return toss_.siteswap;
}

/**
 *
 */

void Prop::ConnectToToss(PropSlot slot)
{
    stateMachine_->get_attribute(tossed_).connect(slot);
}

/**
 *
 */

void Prop::DisconnectFromToss(PropSlot slot)
{
    stateMachine_->get_attribute(tossed_).disconnect(slot);
}

/**
 *
 */

void Prop::ConnectToDrop(PropSlot slot)
{
    stateMachine_->get_attribute(dropped_).connect(slot);
}

/**
 *
 */

void Prop::DisconnectFromDrop(PropSlot slot)
{
    stateMachine_->get_attribute(dropped_).disconnect(slot);
}


/**
 *
 */

void Prop::ConnectToCatch(PropSlot slot)
{
    stateMachine_->get_attribute(catch_).connect(slot);
}

/**
 *
 */

void Prop::DisconnectFromCatch(PropSlot slot)
{
    stateMachine_->get_attribute(catch_).disconnect(slot);
}

/**
 *
 */

void Prop::ConnectToAll(PropSlot tossSlot, PropSlot dropSlot, PropSlot propSlot)
{
    ConnectToToss(tossSlot);
    ConnectToDrop(dropSlot);
    ConnectToCatch(propSlot);
}

/**
 *
 */

void Prop::DisconnectFromAll(PropSlot tossSlot, PropSlot dropSlot, PropSlot propSlot)
{
    DisconnectFromToss(tossSlot);
    DisconnectFromDrop(dropSlot);
    DisconnectFromCatch(propSlot);
}


/**
 *
 */

bool Prop::isIdValid(int id)
{
    return id == id_;
}

/**
 * Start accelorating the prop
 * @remarks 
 * A sucessful toss can only be started if the prop has be caught and in the DWELL state
 */

void Prop::Toss(Throw* toss)
{
    assert(nullptr != toss);
    stateMachine_->process_event(StateMachine::tossEvent(toss));
}


/**
 *
 */

void Prop::Catch(Hand* hand)
{
    if(nullptr != hand){
        DebugOut(_T("Prop::Catch(%d) Prop state: %s  Hand state: %s"), hand->getId(), getStateName(), hand->getStateName()); 
    }
    if(State::DROPPED != getState()){
        stateMachine_->process_event(caughtEvent(hand));
    }
}

/**
 *
 */

void Prop::Collision()
{
    stateMachine_->process_event(StateMachine::collisionEvent(this));
}

/**
 *
 */

void Prop::Pickup(Hand* hand)
{
    stateMachine_->process_event(pickupEvent(hand));
}

/**
 *
 */

void Prop::Tick()
{
    stateMachine_->process_event(StateMachine::tickEvent);
}

