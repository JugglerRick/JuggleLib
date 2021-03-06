﻿#pragma once
#include <tchar.h>
#include <string>
#include <mutex>
#include <condition_variable>

#include <boost\signals2.hpp>
#include "DropReport.h"
namespace JuggleLib
{
    namespace std = ::std;
    namespace boost = ::boost;
    class Throw;

    typedef std::lock_guard<std::mutex> CriticalSection;


    typedef boost::signals2::signal<void(int)> IdPublisher;
    typedef boost::signals2::signal<void(Prop*)> PropPublisher;
    typedef boost::signals2::signal<void(Hand*)> handPublisher;
    typedef boost::signals2::signal<void(Throw*)> throwPublisher;

    typedef std::function<void()> ActionSlot;
    typedef std::function<void(int)> IdSlot;
    typedef std::function<void(Prop*)> PropSlot;
    typedef std::function<void(Hand*)> HandSlot;
    typedef std::function<void(Throw*)> ThrowSlot;
    typedef boost::signals2::signal<void(DropReportPtr)> DropReportPublisher;
    typedef std::function<void(DropReportPtr)> DropReportSlot;
}
#include "Prop.h"
#include "Hand.h"
#include "Throw.h"

