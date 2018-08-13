// epics_pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<ctype.h>

#include<alarm.h>
#include<cvtFast.h>
#include"epics_pv_factory.h"
#include "epicsVersion.h"

static PV_Factory *epics_pv_factory = new EPICS_PV_Factory();

//#define DEBUG_EPICS

#if EPICS_VERSION == 3
  #if EPICS_REVISION < 14
  /* 1/1/90 20 yr (5 leap) of seconds */
  static const unsigned epochSecPast1970 = 7305*86400;
  #else
  static const unsigned epochSecPast1970 = 0;
  #endif
#else
  static const unsigned epochSecPast1970 = 0;
#endif

// --------------------- EPICS_PV_Factory -------------------------------

static int g_context_created = 0;

// All EPICS_PV_Factories share the same static PV pool,
// a hashtable by PV name
//enum { HashTableSize=43 };
enum { HashTableSize=5003 };
class HashTableItem
{
public:
    const char *name;
    EPICS_ProcessVariable *pv;
    DLNode node;
};

// Helpers for Hashtable template to use the HashTableItem
size_t hash(const HashTableItem *item, size_t N)
{   return generic_string_hash(item->name, N); }

bool equals(const HashTableItem *lhs, const HashTableItem *rhs)
{   return strcmp(lhs->name, rhs->name) == 0; }

typedef Hashtable<HashTableItem,
                  offsetof(HashTableItem,node),
                  HashTableSize> PVHash;
static PVHash processvariables;

EPICS_PV_Factory::EPICS_PV_Factory()
{

  // explicitly set ca single threaded mode
  if ( !g_context_created ) {
    g_context_created = 1;
#if (EPICS_VERSION>3)||((EPICS_VERSION==3)&&(EPICS_REVISION>=14))
    ca_context_create( ca_disable_preemptive_callback );
#endif
  }

#ifdef DEBUG_EPICS
  fprintf( stderr,"EPICS_PV_Factory created\n");
#endif

}

EPICS_PV_Factory::~EPICS_PV_Factory()
{
#ifdef DEBUG_EPICS
    fprintf( stderr,"EPICS_PV_Factory deleted\n");
#endif
}

ProcessVariable *EPICS_PV_Factory::create(const char *PV_name)
{
    EPICS_ProcessVariable *pv;
    HashTableItem item;
    item.name = PV_name;
    PVHash::iterator entry = processvariables.find(&item);
    if (entry != processvariables.end())
    {
        pv = (*entry)->pv;
        pv->reference();
        //pv->processExistingPv();
    }
    else
    {
        HashTableItem *n_item = new HashTableItem();
        pv = new EPICS_ProcessVariable(PV_name);
        n_item->name = pv->get_name();
        n_item->pv = pv;
        processvariables.insert(n_item);
    }
    return pv;
}

void EPICS_PV_Factory::forget(EPICS_ProcessVariable *pv)
{
    HashTableItem item;
    item.name = pv->get_name();
    PVHash::iterator entry = processvariables.find(&item);
    if (entry != processvariables.end())
    {
        HashTableItem *p_item = *entry;
        processvariables.erase(entry);
        delete p_item;
        return;
    }
    fprintf(stderr,"EPICS_PV_Factory: internal error in 'forget', PV %s\n",
            pv->get_name());
}

// ---------------------- EPICS_ProcessVariable -------------------------

EPICS_ProcessVariable::EPICS_ProcessVariable(const char *_name)
        : ProcessVariable(_name)
{
    is_connected = false;
    have_ctrlinfo = false;
    read_access = write_access = false;
    pv_chid = 0;
    pv_value_evid = 0;
    value = 0;
    //fprintf( stderr,"EPICS_ProcessVariable %s created\n", get_name());
    int stat = ca_search_and_connect(get_name(), &pv_chid,
                                     ca_connect_callback, this);
    if (stat != ECA_NORMAL)
    {
        fprintf(stderr, "CA search & connect error('%s'): %s\n",
                get_name(), ca_message(stat));
    }
}

EPICS_ProcessVariable::~EPICS_ProcessVariable()
{
    EPICS_PV_Factory::forget(this);
    if (pv_chid)
        ca_clear_channel(pv_chid);
    //fprintf( stderr,"EPICS_ProcessVariable %s deleted\n", get_name());
    delete value;
}

void EPICS_ProcessVariable::processExistingPv ( void ) {

    // This function is not currently in use

    if ( value ) {

        int stat = ca_array_get_callback(value->get_DBR()+DBR_CTRL_STRING,
         1u, pv_chid, ca_ctrlinfo_refresh_callback, this);

        if (stat != ECA_NORMAL)
          fprintf(stderr, "CA get control info error('%s'): %s\n",
           get_name(), ca_message(stat));

    }

}

void EPICS_ProcessVariable::ca_connect_callback(
    struct connection_handler_args arg)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)ca_puser(arg.chid);
    if (arg.op == CA_OP_CONN_UP)
    {

        me->set_node_name( ca_host_name(me->pv_chid) );

        // Check type of PVValue *value
        if (me->value && me->value->get_DBR() != ca_field_type(arg.chid))
        {
            delete me->value;
            me->value = 0;
        }
        if (!me->value)
        {
            switch (ca_field_type(arg.chid))
            {   // TODO: Implement more types?
                case DBF_STRING:
                    me->value = new PVValueString(me);
                    break;
                case DBF_ENUM:
                    me->value = new PVValueEnum(me);
                    break;
                case DBF_CHAR:
                    me->value = new PVValueChar(me);
                    break;
                case DBF_INT:
                    me->value = new PVValueShort(me);
                    break;
                case DBF_LONG:
                    me->value = new PVValueInt(me);
                    break;
                case DBF_FLOAT:
                    me->value = new PVValueDouble(me,"float");
                    break;
                case DBF_DOUBLE:
                default: // fallback: request as double
                    me->value = new PVValueDouble(me);
            }
        }
        // CA quirk: DBR_CTRL doesn't work with arrays,
        // so get only one element:
        int stat = ca_array_get_callback(me->value->get_DBR()+DBR_CTRL_STRING,
                                         1u, me->pv_chid,
                                         ca_ctrlinfo_callback, me);
        if (stat != ECA_NORMAL)
            fprintf(stderr, "CA get control info error('%s'): %s\n",
                    me->get_name(), ca_message(stat));
        me->is_connected = true;
        // status_callback only after ctrlinfo arrives
    }
    else
    {
        me->is_connected = false;
        me->have_ctrlinfo = false;
        me->do_conn_state_callbacks(); // tell widgets we disconnected
    }
}

void EPICS_ProcessVariable::ca_access_security_callback(
    struct access_rights_handler_args arg)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)ca_puser(arg.chid);

    if ( arg.ar.read_access ) {
      me->read_access = true;
    }
    else {
      me->read_access = false;
    }

    if ( arg.ar.write_access ) {
      me->write_access = true;
    }
    else {
      me->write_access = false;
    }

    me->do_access_security_callbacks(); // tell widgets about change

}

void EPICS_ProcessVariable::ca_ctrlinfo_callback(
    struct event_handler_args args)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)args.usr;

    // Sometimes "get callback" functions get called with
    // args.status set to ECA_DISCONN and args.dbr set to NULL.
    // If so, return
    if ( !args.dbr ) return;

    me->value->read_ctrlinfo(args.dbr);
    if (!me->pv_value_evid)
    {
        int stat = ca_add_masked_array_event(me->value->get_DBR()+
                                             DBR_TIME_STRING,
                                             0, //me->get_dimension(),
                                             me->pv_chid,
                                             ca_value_callback,
                                             (void *)me,
                                             (float) 0.0, (float) 0.0,
                                             (float) 0.0,
                                             &me->pv_value_evid,
                                             DBE_VALUE|DBE_ALARM);
        if (stat != ECA_NORMAL)
            fprintf(stderr, "CA add event error('%s'): %s\n",
                    me->get_name(), ca_message(stat));

        stat = ca_replace_access_rights_event(me->pv_chid,
                          ca_access_security_callback);

        if (stat != ECA_NORMAL)
            fprintf(stderr, "CA replace access rights event error('%s'): %s\n",
                    me->get_name(), ca_message(stat));

    }
    else
    {
        if ( !me->have_ctrlinfo ) {
            me->have_ctrlinfo = true;
            me->do_conn_state_callbacks();  // tell widgets we connected
	                                    // & got info
            me->do_access_security_callbacks();
	}
    }
}

void EPICS_ProcessVariable::ca_ctrlinfo_refresh_callback(
    struct event_handler_args args)
{

    // This function is not currently in use

    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)args.usr;

    // Sometimes "get callback" functions get called with
    // args.status set to ECA_DISCONN and args.dbr set to NULL.
    // If so, return
    if ( !args.dbr ) return;

    me->value->read_ctrlinfo(args.dbr);

}

void EPICS_ProcessVariable::ca_value_callback(struct event_handler_args args)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)args.usr;

    if (args.status == ECA_NORMAL  &&  args.dbr)
    {
        me->put_count( (size_t) args.count );
        me->value->read_value(args.dbr);
    }

    if ( !me->have_ctrlinfo ) {
      me->have_ctrlinfo = true;
      me->do_conn_state_callbacks();  // tell widgets we connected & got info
    }

    if (args.status == ECA_NORMAL  &&  args.dbr)
    {
        me->do_value_callbacks();
    }
    else
        fprintf(stderr, "CA value callback('%s'): No data, CA status %s\n",
                me->get_name(), ca_message(args.status));
}

bool EPICS_ProcessVariable::is_valid() const
{   return is_connected && have_ctrlinfo; }

const ProcessVariable::Type &EPICS_ProcessVariable::get_type() const
{   return value->get_type(); }   

const ProcessVariable::specificType &EPICS_ProcessVariable::get_specific_type() const
{   return value->get_specific_type(); }   

int EPICS_ProcessVariable::get_int() const
{   return value->get_int(); }

double EPICS_ProcessVariable::get_double() const
{   return value->get_double(); }

size_t EPICS_ProcessVariable::get_string(char *strbuf, size_t buflen) const
{   return value->get_string(strbuf, buflen); }

size_t EPICS_ProcessVariable::get_dimension() const
{   return ca_element_count(pv_chid); }

const char *EPICS_ProcessVariable::get_char_array() const
{   return value->get_char_array(); }

const short *EPICS_ProcessVariable::get_short_array() const
{   return value->get_short_array(); }

const int *EPICS_ProcessVariable::get_int_array() const
{   return value->get_int_array(); }

const double * EPICS_ProcessVariable::get_double_array() const
{   return value->get_double_array(); }

size_t EPICS_ProcessVariable::get_enum_count() const
{   return value->get_enum_count(); }

const char *EPICS_ProcessVariable::get_enum(size_t i) const
{   return value->get_enum(i); }

time_t EPICS_ProcessVariable::get_time_t() const
{   return value->time; }

unsigned long EPICS_ProcessVariable::get_nano() const
{   return value->nano; }

short EPICS_ProcessVariable::get_status() const
{   return value->status; }

short EPICS_ProcessVariable::get_severity() const
{   return value->severity; }

short EPICS_ProcessVariable::get_precision() const
{   return value->precision; }

const char *EPICS_ProcessVariable::get_units() const
{   return value->units; }

double EPICS_ProcessVariable::get_upper_disp_limit() const
{   return value->upper_disp_limit; }

double EPICS_ProcessVariable::get_lower_disp_limit() const
{   return value->lower_disp_limit; }

double EPICS_ProcessVariable::get_upper_alarm_limit() const
{   return value->upper_alarm_limit; }

double EPICS_ProcessVariable::get_upper_warning_limit() const
{   return value->upper_warning_limit; }

double EPICS_ProcessVariable::get_lower_warning_limit() const
{   return value->lower_warning_limit; }

double EPICS_ProcessVariable::get_lower_alarm_limit() const
{   return value->lower_alarm_limit; }

double EPICS_ProcessVariable::get_upper_ctrl_limit() const
{   return value->upper_ctrl_limit; }

double EPICS_ProcessVariable::get_lower_ctrl_limit() const
{   return value->lower_ctrl_limit; }

bool EPICS_ProcessVariable::have_read_access() const
{

    return ca_read_access(pv_chid) != 0;

}

bool EPICS_ProcessVariable::have_write_access() const
{

    if ( isReadOnly() ) {
      return 0;
    }

    return ca_write_access(pv_chid) != 0;

}

bool EPICS_ProcessVariable::put(double value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        dbr_double_t v = value;
        ca_put(DBR_DOUBLE, pv_chid, &v);
        return true;
    }
    return false;

}

bool EPICS_ProcessVariable::put(int value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        dbr_long_t v = value;
        ca_put(DBR_LONG, pv_chid, &v);
        return true;
    }
    return false;

}
    
bool EPICS_ProcessVariable::put(const char *value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        ca_bput(pv_chid, value);
        return true;
    }
    return false;

}

bool EPICS_ProcessVariable::putText(char *value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        ca_put( DBR_STRING, pv_chid, value);
        return true;
    }
    return false;

}

bool EPICS_ProcessVariable::putArrayText(char *value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        ca_array_put( DBR_CHAR, strlen(value)+1, pv_chid, value );
        return true;
    }
    return false;

}

bool EPICS_ProcessVariable::putAck(short value)
{

    if ( !have_write_access() ) {
      return false;
    }

    if (is_valid())
    {
        ca_put( DBR_PUT_ACKS, pv_chid, &value );
        return true;
    }
    return false;

}

// ---------------------- PVValue ---------------------------------
PVValue::PVValue(EPICS_ProcessVariable *epv)
{
    this->epv = epv;
    time = 0;
    nano = 0;
    status = UDF_ALARM;
    severity = INVALID_ALARM;
    precision = 0;
    units[0] = '\0';
    upper_disp_limit = 10.0;
    lower_disp_limit = 0.0;
    upper_alarm_limit = DBL_MAX;
    upper_warning_limit = DBL_MAX;
    lower_warning_limit = DBL_MIN;
    lower_alarm_limit = DBL_MIN;
    upper_ctrl_limit = 10.0;
    lower_ctrl_limit = 0.0;
}

PVValue::~PVValue()
{}

int PVValue::get_int() const
{   return (int) get_double(); }

size_t PVValue::get_string(char *strbuf, size_t len) const
{
    if (get_enum_count() > 0)
        strcpy(strbuf, get_enum(get_int()));
    else
    {
        // TODO: Handle arrays?
        // TODO: Respect len!
        cvtDoubleToString(get_double(), strbuf, precision);
        if (units[0])
        {
            strcat(strbuf, " ");
            strcat(strbuf, units);
        }
    }
    return strlen(strbuf);
}

const char *PVValue::get_char_array() const
{   return 0; }

const short *PVValue::get_short_array() const
{   return 0; }

const int  *PVValue::get_int_array() const
{   return 0; }

const double *PVValue::get_double_array() const
{   return 0; }

size_t PVValue::get_enum_count() const
{   return 0; }

const char *PVValue::get_enum(size_t i) const
{   return "<not enumerated>"; }

const ProcessVariable::specificType &PVValue::get_specific_type() const
{   return specific_type; }

// ---------------------- PVValueInt ---------------------------

static ProcessVariable::Type integer_type =
{ ProcessVariable::Type::integer, 32, "integer:32" };

static ProcessVariable::specificType i_type =
{ ProcessVariable::specificType::integer, 32 };

static ProcessVariable::specificType s_type =
{ ProcessVariable::specificType::shrt, 16 };

PVValueInt::PVValueInt(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    unsigned int i;
    value = new int[epv->get_dimension()];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    specific_type = i_type;
}

PVValueInt::PVValueInt(EPICS_ProcessVariable *epv, const char* typeInfo)
        : PVValue(epv)
{
    unsigned int i;
    value = new int[epv->get_dimension()];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    if ( !strcmp( typeInfo, "short" ) ) {
      specific_type = s_type;
    }
    else {
      specific_type = i_type;
    }
}

PVValueInt::~PVValueInt()
{
    delete [] value;
}

const ProcessVariable::Type &PVValueInt::get_type() const
{   return integer_type; }

short PVValueInt::get_DBR() const
{   return DBR_LONG; }

int PVValueInt::get_int() const
{   return value[0]; }

double PVValueInt::get_double() const
{   return (double)value[0]; }

size_t PVValueInt::get_string(char *strbuf, size_t len) const
{
    // TODO: Handle arrays?
    int printed;
    if (units[0])
        printed = snprintf(strbuf, len, "%d %s", value[0], units);
    else
        printed = snprintf(strbuf, len, "%d", value[0]);
    // snprintf stops printing at len. But some versions return
    // full string length even if that would have been > len
    if (printed > (int)len)
        return len;
    if (printed < 0)
        return 0;
    return (size_t) printed;
}

const int * PVValueInt::get_int_array() const
{   return value; }

void PVValueInt::read_ctrlinfo(const void *buf)
{
    const  dbr_ctrl_long *val = (const dbr_ctrl_long *)buf;
    status = val->status;
    severity = val->severity;
    precision = 0;
    strncpy(units, val->units, MAX_UNITS_SIZE);
    units[MAX_UNITS_SIZE] = '\0';
    upper_disp_limit = val->upper_disp_limit;
    lower_disp_limit = val->lower_disp_limit;
    upper_alarm_limit = val->upper_alarm_limit; 
    upper_warning_limit = val->upper_warning_limit;
    lower_warning_limit = val->lower_warning_limit;
    lower_alarm_limit = val->lower_alarm_limit;
    upper_ctrl_limit = val->upper_ctrl_limit;
    lower_ctrl_limit = val->lower_ctrl_limit;
    *value = val->value;
}

void PVValueInt::read_value(const void *buf)
{
    const dbr_time_long *val = (const dbr_time_long *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    memcpy(value, &val->value, sizeof(int) * epv->get_dimension());
}

// ---------------------- PVValueShort ---------------------------

static ProcessVariable::specificType shrt_type =
{ ProcessVariable::specificType::shrt, 16 };

PVValueShort::PVValueShort(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    unsigned int i;
    value = new short[epv->get_dimension()];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    specific_type = shrt_type;
}

PVValueShort::~PVValueShort()
{
    delete [] value;
}

const ProcessVariable::Type &PVValueShort::get_type() const
{   return integer_type; }

short PVValueShort::get_DBR() const
{   return DBR_SHORT; }

int PVValueShort::get_int() const
{   return (int) value[0]; }

double PVValueShort::get_double() const
{   return (double) value[0]; }

size_t PVValueShort::get_string(char *strbuf, size_t len) const
{
    // TODO: Handle arrays?
    int printed;
    if (units[0])
        printed = snprintf(strbuf, len, "%d %s", (int) value[0], units);
    else
        printed = snprintf(strbuf, len, "%d", (int) value[0]);
    // snprintf stops printing at len. But some versions return
    // full string length even if that would have been > len
    if (printed > (int)len)
        return len;
    if (printed < 0)
        return 0;
    return (size_t) printed;
}

const short * PVValueShort::get_short_array() const
{   return value; }

void PVValueShort::read_ctrlinfo(const void *buf)
{
    const  dbr_ctrl_short *val = (const dbr_ctrl_short *)buf;
    status = val->status;
    severity = val->severity;
    precision = 0;
    strncpy(units, val->units, MAX_UNITS_SIZE);
    units[MAX_UNITS_SIZE] = '\0';
    upper_disp_limit = val->upper_disp_limit;
    lower_disp_limit = val->lower_disp_limit;
    upper_alarm_limit = val->upper_alarm_limit; 
    upper_warning_limit = val->upper_warning_limit;
    lower_warning_limit = val->lower_warning_limit;
    lower_alarm_limit = val->lower_alarm_limit;
    upper_ctrl_limit = val->upper_ctrl_limit;
    lower_ctrl_limit = val->lower_ctrl_limit;
    *value = val->value;
}

void PVValueShort::read_value(const void *buf)
{
    const dbr_time_short *val = (const dbr_time_short *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    memcpy(value, &val->value, sizeof(short) * epv->get_dimension());
}

// ---------------------- PVValueDouble ---------------------------

static ProcessVariable::Type double_type =
{ ProcessVariable::Type::real, 64, "real:64" };

static ProcessVariable::specificType d_type =
{ ProcessVariable::specificType::real, 64 };

static ProcessVariable::specificType f_type =
{ ProcessVariable::specificType::flt, 32 };

PVValueDouble::PVValueDouble(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    unsigned int i;
    value = new double[epv->get_dimension()];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    specific_type = d_type;
}

PVValueDouble::PVValueDouble(EPICS_ProcessVariable *epv, const char* typeInfo)
        : PVValue(epv)
{
    unsigned int i;
    value = new double[epv->get_dimension()];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    if ( !strcmp( typeInfo, "float" ) ) {
      specific_type = f_type;
    }
    else {
      specific_type = d_type;
    }
}

PVValueDouble::~PVValueDouble()
{
    delete [] value;
}

const ProcessVariable::Type &PVValueDouble::get_type() const
{   return double_type; }

short PVValueDouble::get_DBR() const
{

  if ( specific_type.type == f_type.flt ) {
    return DBR_FLOAT;
  }
  else {
    return DBR_DOUBLE;
  }

}

double PVValueDouble::get_double() const
{   return value[0]; }

const double * PVValueDouble::get_double_array() const
{   return value; }

void PVValueDouble::read_ctrlinfo(const void *buf)
{

    const  dbr_ctrl_double *dval = (const dbr_ctrl_double *)buf;
    const  dbr_ctrl_float *fval = (const dbr_ctrl_float *)buf;

    if ( specific_type.type == f_type.flt ) {

      status = fval->status;
      severity = fval->severity;
      precision = fval->precision;
      strncpy(units, fval->units, MAX_UNITS_SIZE);
      units[MAX_UNITS_SIZE] = '\0';
      upper_disp_limit = fval->upper_disp_limit;
      lower_disp_limit = fval->lower_disp_limit;
      upper_alarm_limit = fval->upper_alarm_limit; 
      upper_warning_limit = fval->upper_warning_limit;
      lower_warning_limit = fval->lower_warning_limit;
      lower_alarm_limit = fval->lower_alarm_limit;
      upper_ctrl_limit = fval->upper_ctrl_limit;
      lower_ctrl_limit = fval->lower_ctrl_limit;
      *value = fval->value;

    }
    else {

      status = dval->status;
      severity = dval->severity;
      precision = dval->precision;
      strncpy(units, dval->units, MAX_UNITS_SIZE);
      units[MAX_UNITS_SIZE] = '\0';
      upper_disp_limit = dval->upper_disp_limit;
      lower_disp_limit = dval->lower_disp_limit;
      upper_alarm_limit = dval->upper_alarm_limit; 
      upper_warning_limit = dval->upper_warning_limit;
      lower_warning_limit = dval->lower_warning_limit;
      lower_alarm_limit = dval->lower_alarm_limit;
      upper_ctrl_limit = dval->upper_ctrl_limit;
      lower_ctrl_limit = dval->lower_ctrl_limit;
      *value = dval->value;

    }

}

void PVValueDouble::read_value(const void *buf)
{

    const  dbr_time_double *dval = (const dbr_time_double *)buf;
    const  dbr_time_float *fval = (const dbr_time_float *)buf;
    unsigned int i;

    if ( specific_type.type == f_type.flt ) {

      time = fval->stamp.secPastEpoch + epochSecPast1970;
      nano = fval->stamp.nsec;
      status = fval->status;
      severity = fval->severity;

      for ( i=0; i<epv->get_dimension(); i++ ) {
	value[i] = (double) (&fval->value)[i];
      }

    }
    else {

      time = dval->stamp.secPastEpoch + epochSecPast1970;
      nano = dval->stamp.nsec;
      status = dval->status;
      severity = dval->severity;

      memcpy(value, &dval->value, sizeof(double) * epv->get_dimension());

    }

}

// ---------------------- PVValueEnum -----------------------------

static ProcessVariable::Type enum_type =
{ ProcessVariable::Type::enumerated, 16, "enumerated:16" };

static ProcessVariable::specificType e_type =
{ ProcessVariable::specificType::enumerated, 16 };

PVValueEnum::PVValueEnum(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    enums = 0;
    specific_type = e_type;
}

const ProcessVariable::Type &PVValueEnum::get_type() const
{   return enum_type; }

short PVValueEnum::get_DBR() const
{   return DBR_ENUM; }

int PVValueEnum::get_int() const
{   return value; }

double PVValueEnum::get_double() const
{   return (double)value; }

size_t      PVValueEnum::get_enum_count() const
{   return enums; }

const char *PVValueEnum::get_enum(size_t i) const
{
    if (i < enums)
        return strs[i];
    return "<undefined>";
}
    
void PVValueEnum::read_ctrlinfo(const void *buf)
{
    const  dbr_ctrl_enum *val = (const dbr_ctrl_enum *)buf;
    status = val->status;
    severity = val->severity;
    enums = val->no_str;
    for (size_t i=0; i<enums; ++i)
        strncpy(strs[i], val->strs[i], MAX_ENUM_STRING_SIZE);
    value = val->value;
    upper_disp_limit = enums;
    upper_ctrl_limit = enums;
}

void PVValueEnum::read_value(const void *buf)
{
    const  dbr_time_enum *val = (const dbr_time_enum *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    value = val->value;
}

// ---------------------- PVValueString -----------------------------


static ProcessVariable::Type string_type =
{ ProcessVariable::Type::text, 0, "text:0" };
   
static ProcessVariable::specificType str_type =
{ ProcessVariable::specificType::text, 0 };

PVValueString::PVValueString(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    value[0] = '\0';
    specific_type = str_type;
}

const ProcessVariable::Type &PVValueString::get_type() const
{   return string_type; }

short PVValueString::get_DBR() const
{   return DBR_STRING; }

int PVValueString::get_int() const
{   return atoi(value); }

double PVValueString::get_double() const
{   return atof(value); }

size_t PVValueString::get_string(char *strbuf, size_t buflen) const
{
    size_t len = strlen(value);
    if (buflen <= len)
        len = buflen-1;
    strncpy(strbuf, value, len);
    strbuf[len] = '\0';
    
    return len;
}

void PVValueString::read_ctrlinfo(const void *buf)
{
    const struct dbr_sts_string *val = (const dbr_sts_string *)buf;
    status = val->status;
    severity = val->severity;
    strcpy(value, val->value);
}
    
void PVValueString::read_value(const void *buf)
{
    const struct dbr_time_string *val = (const dbr_time_string *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    strcpy(value, val->value);
}

// ---------------------- PVValueChar -------------------------------

static ProcessVariable::specificType c_type =
{ ProcessVariable::specificType::chr, 8 };

PVValueChar::PVValueChar(EPICS_ProcessVariable *epv)
        : PVValue(epv)
{
    unsigned int i;
    size_t room = epv->get_dimension() + 1;
    if (room < 2)
        room = 2;
    value = new char[room];
    for ( i=0; i<epv->get_dimension(); i++ ) value[i] = 0;
    len = 0;
    specific_type = c_type;

    //    fprintf( stderr,"PVValueChar(%s): dimension %d, room %d\n",
    //           epv->get_name(), epv->get_dimension(), room);
}

PVValueChar::~PVValueChar()
{
    //    fprintf( stderr,"~PVValueChar(%s)\n", epv->get_name());
    delete [] value;
}

const ProcessVariable::Type &PVValueChar::get_type() const
{   return string_type; }

short PVValueChar::get_DBR() const
{   return DBR_CHAR; }

int PVValueChar::get_int() const
{   return (len > 0) ? (int)value[0] : 0; }

double PVValueChar::get_double() const
{   return (len > 0) ? (double)value[0] : 0; }

const char * PVValueChar::get_char_array() const
{   return value; }

size_t PVValueChar::get_string(char *strbuf, size_t buflen) const
{
    size_t src = 0, dst = 0;

    while (value[src]  &&  src < len  &&  dst < buflen)
    {
        if (isprint(value[src]))
        {
            strbuf[dst++] = value[src++];
        }
        else  if (dst < buflen-5)
        {
            sprintf(strbuf+dst, "(%02X)", (int)value[src]);
            dst += 4;
            ++src;
        }
        else
            break;
    }
    strbuf[dst] = '\0';
    
    return dst;
}

void PVValueChar::read_ctrlinfo(const void *buf)
{
    const struct dbr_sts_char *val = (const dbr_sts_char *)buf;
    status = val->status;
    severity = val->severity;
    value[0] = val->value;
    value[1] = '\0';
    len = 1;
    //   fprintf( stderr,"PVValueChar(%s)::read_ctrlinfo '%s'\n",
    //           epv->get_name(), value);
}
    
void PVValueChar::read_value(const void *buf)
{
    const struct dbr_time_char *val = (const dbr_time_char *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    size_t copy = epv->get_dimension();
    memcpy(value, &val->value, copy);
    value[copy] = '\0';
    len = copy;
    //    fprintf( stderr,"PVValueChar(%s)::read_value '%s'\n",
    //           epv->get_name(), value);
}

#ifdef __cplusplus
extern "C" {
#endif

int epics_pend_io (
  double sec
) {

  return ca_pend_io( sec );

}

int epics_pend_event (
  double sec
) {

  return ca_pend_event( sec );

}

void epics_task_exit ( void ) {

  ca_task_exit();

}

ProcessVariable *create_EPICSPtr (
  const char *PV_name
) {

ProcessVariable *ptr;

  ptr = epics_pv_factory->create( PV_name );
  return ptr;

}

#ifdef __cplusplus
}
#endif
