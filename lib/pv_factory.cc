// -*- c++ -*-
//
// pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<stdlib.h>
#include"epics_pv_factory.h"
#include"calc_pv_factory.h"
#include"loc_pv_factory.h"

// Available PV_Factories:
       PV_Factory *the_PV_Factory   = new PV_Factory();
static PV_Factory *epics_pv_factory = new EPICS_PV_Factory();
static PV_Factory *calc_pv_factory  = new CALC_PV_Factory();
static PV_Factory *loc_pv_factory  = new LOC_PV_Factory();

//extern "C" static void remove_pv_factories()
static void remove_pv_factories()
{
    if (calc_pv_factory)
    {
        delete calc_pv_factory;
        calc_pv_factory = 0;
    }
    if (epics_pv_factory)
    {
        delete epics_pv_factory;
        epics_pv_factory = 0;
    }
    if (the_PV_Factory)
    {
        delete the_PV_Factory;
        the_PV_Factory = 0;
    }
}

PV_Factory::PV_Factory()
{
    atexit(remove_pv_factories);
}

PV_Factory::~PV_Factory()
{
}

class ProcessVariable *PV_Factory::create(const char *PV_name)
{

class ProcessVariable *pv;

  if (strncmp(PV_name, "EPICS\\", 6)==0) {
    pv = epics_pv_factory->create(PV_name+6);
    return pv;
  }
  else if (strncmp(PV_name, "CALC\\", 5)==0) {
    pv = calc_pv_factory->create(PV_name+5);
    return pv;
  }
  else if (strncmp(PV_name, "LOC\\", 4)==0) {
    pv = loc_pv_factory->create(PV_name+4);
    return pv;
  }
  else if (strchr(PV_name, '\\')) {
    fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
    return 0;
  }
    
  pv = epics_pv_factory->create(PV_name);

  return pv;

}

class ProcessVariable *PV_Factory::createWithInitialCallbacks (
  const char *PV_name
) {

class ProcessVariable *pv;

  if (strncmp(PV_name, "EPICS\\", 6)==0) {
    pv = epics_pv_factory->create(PV_name+6);
    return pv;
  }
  else if (strncmp(PV_name, "CALC\\", 5)==0) {
    pv = calc_pv_factory->create(PV_name+5);
    return pv;
  }
  else if (strncmp(PV_name, "LOC\\", 4)==0) {
    pv = loc_pv_factory->create(PV_name+4);
    return pv;
  }
  else if (strchr(PV_name, '\\')) {
    fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
    return 0;
  }
    
  pv = epics_pv_factory->create(PV_name);

  return pv;

}

// These two should be static, but then "friend" doesn't work,
// so the CallBackInfo would have to be public which is
// not what I want, either...
size_t hash(const PVCallbackInfo *item, size_t N)
{   return ((size_t)item->func*41 + (size_t)item->userarg*43)%N; }

bool equals(const PVCallbackInfo *lhs,
            const PVCallbackInfo *rhs)
{
    return lhs->func == rhs->func &&
        lhs->userarg == rhs->userarg;
}

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

// Some defaults for member functions:
int ProcessVariable::get_int() const
{   return (int) get_double(); }

size_t ProcessVariable::get_string(char *strbuf, size_t buflen) const
{
    sprintf(strbuf, "%g", get_double());
    return strlen(strbuf);
}

size_t ProcessVariable::get_enum_count() const
{   return 0; }

const char *ProcessVariable::get_enum(size_t i) const
{   return 0; }

const char *ProcessVariable::get_units() const
{   return ""; }

void ProcessVariable::add_conn_state_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo *info = new PVCallbackInfo;
    info->func = func;
    info->userarg = userarg;
    // TODO: search for existing one?
    conn_state_callbacks.insert(info);
    // Perform initial callback in case we already have a value
    // (otherwise user would have to wait until the next change)
    if (is_valid())
        (*func)(this, userarg);
}

void ProcessVariable::remove_conn_state_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo info;
    info.func = func;
    info.userarg = userarg;
    PVCallbackInfoHash::iterator entry = conn_state_callbacks.find(&info);
    if (entry != conn_state_callbacks.end()) {
        PVCallbackInfo *p_item = *entry;
        conn_state_callbacks.erase(entry);
        delete p_item;
    }
}

void ProcessVariable::add_value_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo *info = new PVCallbackInfo;
    info->func = func;
    info->userarg = userarg;
    // TODO: search for existing one?
    value_callbacks.insert(info);

    // Perform initial callback in case we already have a value
    // (otherwise user would have to wait until the next change)
    if (is_valid())
        (*func)(this, userarg);
}

void ProcessVariable::remove_value_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo info;
    info.func = func;
    info.userarg = userarg;
    PVCallbackInfoHash::iterator entry = value_callbacks.find(&info);
    if (entry != value_callbacks.end()) {
        PVCallbackInfo *p_item = *entry;
        value_callbacks.erase(entry);
        delete p_item;
    }
}

void ProcessVariable::do_conn_state_callbacks()
{
    PVCallbackInfo *info;
    for (PVCallbackInfoHash::iterator entry = conn_state_callbacks.begin();
         entry != conn_state_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func)
            (*info->func) (this, info->userarg);
    }
}

void ProcessVariable::do_value_callbacks()
{
    PVCallbackInfo *info;
    for (PVCallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func)
            (*info->func) (this, info->userarg);
    }
}

bool ProcessVariable::have_write_access() const
{
    return true;
}

bool ProcessVariable::put(int value)
{   return put(double(value)); }

