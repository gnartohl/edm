// -*- c++ -*-
//
// pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<stdlib.h>
#include"pv_factory.h"

PV_Factory::~PV_Factory()
{}

// These two should be static, but then "friend" doesn't work,
// so the CallBackInfo would have to be public which is
// not what I want, either...
size_t hash(const ProcessVariable::CallbackInfo *item, size_t N)
{   return ((size_t)item->func*41 + (size_t)item->userarg*43)%N; }

bool equals(const ProcessVariable::CallbackInfo *lhs,
            const ProcessVariable::CallbackInfo *rhs)
{   return lhs->func == rhs->func && lhs->userarg == rhs->userarg; }

ProcessVariable::ProcessVariable(const char *_name)
{
    name = strdup(_name);
    refcount = 1;
}

ProcessVariable::~ProcessVariable()
{
    if (refcount != 0)
        printf("ProcessVariable %s deleted with refcount %d\n",
               name, refcount);
    free(name);
}

void ProcessVariable::add_status_callback(Callback func, void *userarg)
{
    // TODO: search for existing one?
    status_callbacks.insert(new CallbackInfo(func, userarg));
}

void ProcessVariable::remove_status_callback(Callback func, void *userarg)
{
    CallbackInfo info(func, userarg);
    CallbackInfoHash::iterator entry = status_callbacks.find(&info);
    if (entry != status_callbacks.end())
        status_callbacks.erase(entry);
}

void ProcessVariable::add_value_callback(Callback func, void *userarg)
{
    // TODO: search for existing one?
    value_callbacks.insert(new CallbackInfo(func, userarg));
}

void ProcessVariable::remove_value_callback(Callback func, void *userarg)
{
    CallbackInfo info(func, userarg);
    CallbackInfoHash::iterator entry = value_callbacks.find(&info);
    if (entry != status_callbacks.end())
        value_callbacks.erase(entry);
}

void ProcessVariable::do_status_callbacks()
{
    for (CallbackInfoHash::iterator entry = status_callbacks.begin();
         entry != status_callbacks.end();
         ++entry)
        (*entry)->call(this);
}

void ProcessVariable::do_value_callbacks()
{
    for (CallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
        (*entry)->call(this);
}

bool ProcessVariable::put(int value)
{   return put(double(value)); }

