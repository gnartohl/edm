// -*- c++ -*-
//
// pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<stdlib.h>
#include "pvBindings.h"

static int edmReadOnly = 0;
static pvBindingClass pvObj;

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

  return (int) pvObj.pend_io( sec );

}

int pend_event ( double sec )
{

  return (int) pvObj.pend_event( sec );

}

void task_exit ( void ) {

  pvObj.task_exit();

}

// Available PV_Factories:
       PV_Factory *the_PV_Factory   = new PV_Factory();

//extern "C" static void remove_pv_factories()
static void remove_pv_factories()
{
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

char *supportedType;

  supportedType = pvObj.firstPvName();
  while ( supportedType ) {

    if ( strcmp( pv_type, supportedType ) == 0 ) {
      return 1;
    }

    supportedType = pvObj.nextPvName();

  }

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
char buf[255+1];
int i, l, pos;

  if ( strchr(PV_name, '\\') ) {

    strcpy( buf, "" );
    l = strlen(PV_name);
    if ( l > 255 ) l = 255;
    for ( i=0; i<l; i++ ) {
      if ( PV_name[i] == '\\' ) break;
      buf[i] = PV_name[i];
    }
    buf[i] = 0;

    pos = i + 1;

    pv = pvObj.createNew( buf, PV_name+pos );
    if ( pv ) {
      return pv;
    }
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
      return 0;
    }

  }

  if ( strcmp( default_pv_type, "" ) ) {

    pv = pvObj.createNew( default_pv_type, PV_name );
    if ( pv ) {
      return pv;
    }
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
      return 0;
    }

  }

  pv = pvObj.createNew( pvObj.firstPvName(), PV_name );
  return pv;

}

class ProcessVariable *PV_Factory::createWithInitialCallbacks (
  const char *PV_name
) {

class ProcessVariable *pv;
char buf[255+1];
int i, l, pos;

  if ( strchr(PV_name, '\\') ) {

    strcpy( buf, "" );
    l = strlen(PV_name);
    if ( l > 255 ) l = 255;
    for ( i=0; i<l; i++ ) {
      if ( PV_name[i] == '\\' ) break;
      buf[i] = PV_name[i];
    }
    buf[i] = 0;

    pos = i + 1;

    pv = pvObj.createNew( buf, PV_name+pos );
    if ( pv ) {
      return pv;
    }
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
      return 0;
    }

  }

  if ( strcmp( default_pv_type, "" ) ) {

    pv = pvObj.createNew( default_pv_type, PV_name );
    if ( pv ) {
      return pv;
    }
    else {
      fprintf(stderr, "Unknown PV Factory for PV '%s'\n", PV_name);
      return 0;
    }

  }

  pv = pvObj.createNew( pvObj.firstPvName(), PV_name );
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
    curEleCount = 1;
    
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

void ProcessVariable::add_access_security_callback (
  PVCallback func, void *userarg
) {

PVCallbackInfo *info = new PVCallbackInfo;

  info->func = func;
  info->userarg = userarg;
  // TODO: search for existing one?
  access_security_callbacks.insert(info);
  // Perform initial callback in case we already have information
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

void ProcessVariable::remove_access_security_callback (
  PVCallback func, void *userarg
) {

PVCallbackInfo info;

  info.func = func;
  info.userarg = userarg;
  PVCallbackInfoHash::iterator entry = access_security_callbacks.find(&info);
  if (entry != access_security_callbacks.end()) {
    PVCallbackInfo *p_item = *entry;
    access_security_callbacks.erase(entry);
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

int ProcessVariable::get_num_conn_state_callbacks ( void ) {

    PVCallbackInfo *info;
    int n = 0;

    for (PVCallbackInfoHash::iterator entry = conn_state_callbacks.begin();
         entry != conn_state_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func)
            n++;
    }

    return n;

}

void ProcessVariable::do_access_security_callbacks ( void ) {

PVCallbackInfo *info;

  for (PVCallbackInfoHash::iterator entry = access_security_callbacks.begin();
   entry != access_security_callbacks.end(); ++entry) {
    info = *entry;
    if (info->func) {
      (*info->func) (this, info->userarg);
    }
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

int ProcessVariable::get_num_value_callbacks ( void ) {

    PVCallbackInfo *info;
    int n = 0;

    for (PVCallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func) {
	  n++;
	}
    }

    return n;

}

void ProcessVariable::recalc() {
}

bool ProcessVariable::have_read_access() const
{
    return true;
}

bool ProcessVariable::have_write_access() const
{
    return true;
}

bool ProcessVariable::put(double value) {
  return true;
}

bool ProcessVariable::put(const char *dsp, double value) {
  return put(value);
}

bool ProcessVariable::put(const char *value) {
  return true;
}

bool ProcessVariable::put(const char *dsp, const char *value) {
  return put(value);
}

bool ProcessVariable::put(int value)
{   return true; }

bool ProcessVariable::put(const char *dsp, int value)
{   return put(value); }

bool ProcessVariable::putText(char *value) {
  return true;
}

bool ProcessVariable::putText(const char *dsp, char *value) {
  return putText(value);
}

bool ProcessVariable::putAck(short value)
{    return true; }

bool ProcessVariable::putAck(const char *dsp, short value)
{    return putAck(value); }

