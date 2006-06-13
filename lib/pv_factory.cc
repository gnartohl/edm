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
//#include"db_pv_factory.h"

static int edmReadOnly = 0;

void setReadOnly ( void )
{
  edmReadOnly = 1;
}

void setReadWrite ( void )
{
  edmReadOnly = 0;
}

int isReadOnly ( void )
{
  return edmReadOnly;
}

int pend_io ( double sec )
{

#ifdef __epics__
  return (int) ca_pend_io( sec );
#endif

#ifndef __epics__
  return (int) ECA_NORMAL;
#endif

}

int pend_event ( double sec )
{

#ifdef __epics__
  return (int) ca_pend_event( sec );
#endif

#ifndef __epics__
  return (int) ECA_NORMAL;
#endif

}

// Available PV_Factories:
       PV_Factory *the_PV_Factory   = new PV_Factory();
static PV_Factory *epics_pv_factory = new EPICS_PV_Factory();
static PV_Factory *calc_pv_factory  = new CALC_PV_Factory();
static PV_Factory *loc_pv_factory  = new LOC_PV_Factory();
//static PV_Factory *db_pv_factory  = new DB_PV_Factory();

//extern "C" static void remove_pv_factories()
static void remove_pv_factories()
{
    //if (db_pv_factory)
    //{
    //    delete db_pv_factory;
    //    db_pv_factory = 0;
    //}
    if (loc_pv_factory)
    {
        delete loc_pv_factory;
        loc_pv_factory = 0;
    }
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
    strcpy( default_pv_type, "" );
    atexit(remove_pv_factories);
}

PV_Factory::~PV_Factory()
{

}

int PV_Factory::legal_pv_type (
  const char *pv_type )
{

  if ( strcmp( pv_type, "EPICS" ) == 0 ) {
    return 1;
  }
  else if ( strcmp(pv_type, "CALC" ) == 0 ) {
    return 1;
  }
  else if ( strcmp(pv_type, "LOC" ) == 0 ) {
    return 1;
  }
  //else if ( strcmp(pv_type, "DB" ) == 0 ) {
  //  return 1;
  //}

  return 0;

}

void PV_Factory::set_default_pv_type (
  const char *pv_type )
{

  if ( legal_pv_type( pv_type ) ) {

    strncpy( default_pv_type, pv_type, 31 );
    default_pv_type[31] = 0;

  }
  else {

    fprintf(stderr,
     "Cannot set default PV type to '%s', using system default\n", pv_type );
    strcpy( default_pv_type, "" );

  }

}

void PV_Factory::clear_default_pv_type ( void )
{

  //fprintf( stderr, "clearing default pv type\n" );

  strcpy( default_pv_type, "" );

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
  //else if (strncmp(PV_name, "DB\\", 3)==0) {
  //  pv = db_pv_factory->create(PV_name+3);
  //  return pv;
  //}
  else if (strchr(PV_name, '\\')) {
    fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
    return 0;
  }

  if ( strcmp( default_pv_type, "" ) ) {

    if (strncmp(default_pv_type, "EPICS", 6)== 0) {
      pv = epics_pv_factory->create(PV_name);
      return pv;
    }
    else if (strncmp(default_pv_type, "CALC", 5)==0) {
      pv = calc_pv_factory->create(PV_name);
      return pv;
    }
    else if (strncmp(default_pv_type, "LOC", 4)==0) {
      pv = loc_pv_factory->create(PV_name);
      return pv;
    }
    //else if (strncmp(default_pv_type, "DB", 3)==0) {
    //  pv = db_pv_factory->create(PV_name);
    //  return pv;
    //}
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s\\%s'\n",
       default_pv_type, PV_name);
      return 0;
    }

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
  //else if (strncmp(PV_name, "DB\\", 3)==0) {
  //  pv = db_pv_factory->create(PV_name+3);
  //  return pv;
  //}
  else if (strchr(PV_name, '\\')) {
    fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
    return 0;
  }
    
  if ( strcmp( default_pv_type, "" ) ) {

    if (strncmp(default_pv_type, "EPICS", 6)== 0) {
      pv = epics_pv_factory->create(PV_name);
      return pv;
    }
    else if (strncmp(default_pv_type, "CALC", 5)==0) {
      pv = calc_pv_factory->create(PV_name);
      return pv;
    }
    else if (strncmp(default_pv_type, "LOC", 4)==0) {
      pv = loc_pv_factory->create(PV_name);
      return pv;
    }
    //else if (strncmp(default_pv_type, "DB", 3)==0) {
    //  pv = db_pv_factory->create(PV_name);
    //  return pv;
    //}
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s\\%s'\n",
       default_pv_type, PV_name);
      return 0;
    }

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

size_t hash(const NodeNameInfo *item, size_t N)
{   return generic_string_hash(item->nodeName, N); }

bool equals(const NodeNameInfo *lhs,
            const NodeNameInfo *rhs)
{   return strcmp(lhs->nodeName, rhs->nodeName) == 0; }

ProcessVariable::ProcessVariable(const char *_name)
{
    name = strdup(_name);
    refcount = 1;
    numTimesConnected = numTimesDisconnected = numValueChangeEvents = 0;
    nodeName = NULL;
}

ProcessVariable::~ProcessVariable()
{

    //fprintf( stderr, "~ProcessVariable, name=[%s]\n", name );
    //fprintf( stderr, "num times connected = %-d, disconnected = %-d\n",
    // numTimesConnected, numTimesDisconnected );
    //fprintf( stderr, "num value change events = %-d\n", numValueChangeEvents );
    //if ( nodeName ) {
    //  fprintf( stderr, "node name = [%s] [%-x]\n", nodeName, (int) nodeName );
    //}

    if (refcount != 0)
        fprintf( stderr,"ProcessVariable %s deleted with refcount %d\n",
               name, refcount);
    if ( name ) {
      free(name);
      name = NULL;
    }

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
    if (is_valid()) {
        (*func)(this, userarg);
    }
}

void ProcessVariable::set_node_name( const char *_nodeName ) {

  char *theName;
  NodeNameInfo info;
  info.nodeName = (char *) _nodeName;
  NodeNameInfoHash::iterator entry = nodeNames.find(&info);
  if (entry == nodeNames.end()) {
    NodeNameInfo *pinfo = new NodeNameInfo;
    pinfo->nodeName = strdup( (char *) _nodeName );
    nodeNames.insert(pinfo);
    theName = pinfo->nodeName;
  }
  else {
    theName = (*entry)->nodeName;
  }

  this->nodeName = theName;

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
    if (is_valid()) {
        recalc();
        (*func)(this, userarg);
    }
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

    if ( is_valid() ) {
      numTimesConnected++;
    }
    else {
      numTimesDisconnected++;
    }

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

    numValueChangeEvents++;

    for (PVCallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func) {
            (*info->func) (this, info->userarg);
	}
    }
}

void ProcessVariable::recalc() {
}

bool ProcessVariable::have_write_access() const
{
    return true;
}

bool ProcessVariable::put(int value)
{   return put(double(value)); }

bool ProcessVariable::putAck(short value)
{    return true; }
