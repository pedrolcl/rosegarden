/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
  Rosegarden
  A sequencer and musical notation editor.
  Copyright 2000-2022 the Rosegarden development team.
  See the AUTHORS file for more details.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.  See the file
  COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[LV2Worker]"
#define RG_NO_DEBUG_PRINT 1

#include "LV2Worker.h"

#include "sound/LV2Utils.h"

#include <QTimer>

#include "misc/Debug.h"

namespace {
    // cppcheck-suppress unusedFunction
    LV2_Worker_Status scheduleWorkC(LV2_Worker_Schedule_Handle handle,
                                    uint32_t                   size,
                                    const void*                data)
    {
        Rosegarden::LV2Utils::PluginPosition* pp =
            (Rosegarden::LV2Utils::PluginPosition*)handle;
        Rosegarden::LV2Utils* lv2utils = Rosegarden::LV2Utils::getInstance();
        Rosegarden::LV2Worker* lw =
            (Rosegarden::LV2Worker*)lv2utils->getWorker();
        if (!lw) return LV2_WORKER_SUCCESS;
        return lw->scheduleWork(size, data, *pp);
    }

    LV2_Worker_Status respondWorkC(LV2_Worker_Respond_Handle handle,
                                   uint32_t size,
                                   const void *data)
    {
        Rosegarden::LV2Utils::PluginPosition* pp =
            (Rosegarden::LV2Utils::PluginPosition*)handle;
        Rosegarden::LV2Utils* lv2utils = Rosegarden::LV2Utils::getInstance();
        Rosegarden::LV2Worker* lw =
            (Rosegarden::LV2Worker*)lv2utils->getWorker();
        if (!lw) return LV2_WORKER_SUCCESS;
        return lw->respondWork(size, data, *pp);
    }

}

namespace Rosegarden
{

LV2Worker::LV2Worker()
{
    RG_DEBUG << "create LV2Worker";
    m_workTimer = new QTimer(this);
    connect(m_workTimer, &QTimer::timeout,
            this, &LV2Worker::workTimeUp);
    m_workTimer->start(50);
}

LV2Worker::~LV2Worker()
{
    RG_DEBUG << "~LV2Worker";
}

LV2Utils::ScheduleWork LV2Worker::getScheduler()
{
    return scheduleWorkC;
}

LV2Utils::WorkerJob* LV2Worker::getResponse(const LV2Utils::PluginPosition& pp)
{
    // called in the audio thread
    RG_DEBUG << "getResponse called" << m_workerResponses.size();
    auto it = m_workerResponses.find(pp);
    if (it == m_workerResponses.end()) return nullptr;
    JobQueue& jq = (*it).second;
    if (jq.empty()) return nullptr;
    LV2Utils::WorkerJob* jobcopy =
        new LV2Utils::WorkerJob(jq.front());
    jq.pop();
    return jobcopy;
}

LV2_Worker_Status LV2Worker::scheduleWork(uint32_t size,
                                          const void* data,
                                          const LV2Utils::PluginPosition& pp)
{
    // this is called by the plugin in the audio thread
    RG_DEBUG << "scheduleWork called" << size;

    // if we were doing direct rendering we could call work here. In
    // real time processing the work must be queued
    LV2Utils::WorkerJob job;
    job.size = size;
    job.data = new char[size];
    memcpy((void*)job.data, data, size);
    JobQueue& jq = m_workerJobs[pp];
    jq.push(job);

    for(auto& pair : m_workerJobs) {
        //const LV2Utils::PluginPosition& pp = pair.first;
        JobQueue& jq = pair.second;
        RG_DEBUG << "sched job queue" << jq.size();
    }

    return LV2_WORKER_SUCCESS;
}

LV2_Worker_Status LV2Worker::respondWork(uint32_t size,
                                         const void* data,
                                         const LV2Utils::PluginPosition& pp)
{
    // this is called by the plugin in the non audio thread
    RG_DEBUG << "respondWork called" << m_workerResponses.size();
    LV2Utils::WorkerJob job;
    job.size = size;
    job.data = new char[size];
    memcpy((void*)job.data, data, size);
    JobQueue& jq = m_workerResponses[pp];
    LV2Utils* lv2utils = LV2Utils::getInstance();
    lv2utils->lock();
    jq.push(job);
    lv2utils->unlock();

    return LV2_WORKER_SUCCESS;
}

void LV2Worker::workTimeUp()
{
    //RG_DEBUG << "workTimeUp" << m_workerJobs.size();
    LV2Utils* lv2utils = LV2Utils::getInstance();
    lv2utils->lock();
    for(auto& pair : m_workerJobs) {
        const LV2Utils::PluginPosition& pp = pair.first;
        JobQueue& jq = pair.second;
        RG_DEBUG << "job queue" << jq.size();
        while(! jq.empty()) {
            RG_DEBUG << "work to do";
            LV2Utils::WorkerJob& job = jq.front();
            // call work
            lv2utils->runWork(pp, job.size, job.data, respondWorkC);

            delete[] (char*)job.data;
            jq.pop();
        }
    }
    lv2utils->unlock();
}

}
