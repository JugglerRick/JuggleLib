#include "pch.h"
#include "Prop.h"
#include "Pass.h"
#include "common_state_machine.h"
#include <iostream>

namespace msm = boost::msm;
using namespace boost::msm::front::euml;

namespace StateMachine
{
    BOOST_MSM_EUML_DECLARE_ATTRIBUTE(IPropResponder*, responder_)


    BOOST_MSM_EUML_ACTION(catch_entry)
    {
        template <class Event, class FSM, class STATE>
        void operator()(Event const& evt, FSM& fsm, STATE& state)
        {
            if(nullptr != fsm.get_attribute(responder_))
            {
                fsm.get_attribute(responder_)->Catch(fsm.get_attribute(id_), fsm.get_attribute(destinationHand_));
            }
        }
    };

    BOOST_MSM_EUML_ACTION(dropped_entry)
    {
        template <class Event, class FSM, class STATE>
        void operator()(Event const& evt, FSM& fsm, STATE& state)
        {
            if(nullptr != fsm.get_attribute(responder_))
            {
                fsm.get_attribute(responder_)->Dropped(fsm.get_attribute(id_));
            }
        }
    };
    BOOST_MSM_EUML_STATE((catch_entry), Catch)
    BOOST_MSM_EUML_STATE((dropped_entry), Dropped)

    BOOST_MSM_EUML_ACTION(toss_action)
    {
        template <class FSM,class EVT,class SourceState,class TargetState>
        void operator()(EVT const& evt ,FSM& fsm,SourceState& ,TargetState& )
        {
            fsm.get_attribute(siteswap_) = evt.get_attribute(siteswap_);
            fsm.get_attribute(destinationHand_) = evt.get_attribute(destinationHand_);
            if(nullptr != fsm.get_attribute(responder_))
            {
                fsm.get_attribute(responder_)->Tossed(fsm.get_attribute(id_));
            }
        }
    };



    BOOST_MSM_EUML_TRANSITION_TABLE(
        (
            Dwell + tossEvent / toss_action == Flight,
            Flight + tickEvent / if_then_((fsm_(siteswap_) > Int_<0>()), (--fsm_(siteswap_)))   ,
            Flight + tickEvent [fsm_(siteswap_) == Int_<0>()]      == Catch,
            Catch + catchEvent                       == Dwell,
            Catch + tickEvent                       == Dropped,
            Dropped + pickupEvent                   == Dwell,
            Dwell + collisionEvent                  == Dropped,
            Flight + collisionEvent                 == Dropped,
            Catch + collisionEvent                  == Dropped
        )
        , prop_transition_table
    )

    BOOST_MSM_EUML_DECLARE_STATE_MACHINE
    ( 
        (
            prop_transition_table, 
            init_ << Dropped,
            no_action,
            no_action,
            attributes_<< id_ << responder_  << siteswap_ <<  destinationHand_ 
        ), 
        prop_state_machine
    )

    // the type for the state machine

    typedef msm::back::state_machine<prop_state_machine> Base;
    const TCHAR* stateNames[] = {
        TEXT("Dwell"),
        TEXT("Flight"),
        TEXT("Catch"),
        TEXT("Dropped"),
    };

}

struct Prop::PropStateMachine : public StateMachine::Base
{
    PropStateMachine(int id, IPropResponder* responder)
    {
        get_attribute(StateMachine::id_) = id;
        get_attribute(StateMachine::responder_) = responder;
    }
};

Prop::Prop(int id, IPropResponder* responder)
:   stateMachine(new Prop::PropStateMachine(id, responder) )
{
}


Prop::~Prop(void)
{
}

Prop::State Prop::getState()
{
    return static_cast<Prop::State>(*(stateMachine->current_state()));
}


const TCHAR* Prop::getStateName()
{
    return  StateMachine::stateNames[getState()];
}


int Prop::getCurrentSwap()
{
    return stateMachine->get_attribute(StateMachine::siteswap_);
}


/**
 * Start accelorating the prop
 * @remarks 
 * A sucessful toss can only be started if the prop has be caught and in the DWELL state
 */

void Prop::Toss(const Pass* pass)
{
    assert(nullptr != pass);
    stateMachine->process_event(StateMachine::tossEvent(pass->siteswap, pass->destination));
}


void Prop::Catch()
{
    stateMachine->process_event(StateMachine::catchEvent);
}

void Prop::Collision()
{
    stateMachine->process_event(StateMachine::collisionEvent);
}

void Prop::Pickup()
{
    stateMachine->process_event(StateMachine::pickupEvent);
}

void Prop::Tick()
{
    stateMachine->process_event(StateMachine::tickEvent);
}