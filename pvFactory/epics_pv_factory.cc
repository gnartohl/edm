// epics_pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<float.h>

#include<alarm.h>
#include<cvtFast.h>
#include"epics_pv_factory.h"

/* 1/1/90 20 yr (5 leap) of seconds */
static const unsigned epochSecPast1970 = 7305*86400;

// --------------------- EPICS_PV_Factory -------------------------------

// All EPICS_PV_Factories share the same static PV pool,
// a hashtable by PV name
enum { HashTableSize=43 };
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
    fprintf(stderr, "EPICS_PV_Factory created\n");
}

EPICS_PV_Factory::~EPICS_PV_Factory()
{
    fprintf(stderr, "EPICS_PV_Factory deleted\n");
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
    printf("EPICS_PV_Factory: internal error in 'forget', PV %s\n",
           pv->get_name());
}

// ---------------------- EPICS_ProcessVariable -------------------------

EPICS_ProcessVariable::EPICS_ProcessVariable(const char *_name)
        : ProcessVariable(_name)
{
    is_connected = false;
    have_ctrlinfo = false;
    pv_chid = 0;
    pv_value_evid = 0;
    value = 0;
    //printf("EPICS_ProcessVariable %s created\n", get_name());
    int stat = ca_search_and_connect(get_name(), &pv_chid,
                                     ca_connect_callback, this);
    if (stat != ECA_NORMAL)
    {
        printf("CA search & connect error('%s'): %s\n",
               get_name(), ca_message(stat));
    }
}

EPICS_ProcessVariable::~EPICS_ProcessVariable()
{
    EPICS_PV_Factory::forget(this);
    if (pv_chid)
        ca_clear_channel(pv_chid);
    //printf("EPICS_ProcessVariable %s deleted\n", get_name());
    delete value;
}

void EPICS_ProcessVariable::ca_connect_callback(
    struct connection_handler_args arg)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)ca_puser(arg.chid);
    if (arg.op == CA_OP_CONN_UP)
    {
        // Check type of PVValue *value
        if (me->value && me->value->get_DBR() != ca_field_type(arg.chid))
        {
            delete me->value;
            me->value = 0;
        }
        if (!me->value)
        {
            switch (ca_field_type(arg.chid))
            {   // TODO: Implement more types
                case DBF_ENUM:
                    me->value = new PVValueEnum();
                    break;
                case DBF_DOUBLE:
                default: // fallback: request as double
                    me->value = new PVValueDouble();
            }
        }
        int stat = ca_array_get_callback(me->value->get_DBR()+DBR_CTRL_STRING,
                                         1u, me->pv_chid,
                                         ca_ctrlinfo_callback, me);
        if (stat != ECA_NORMAL)
            printf("CA get control info error('%s'): %s\n",
                   me->get_name(), ca_message(stat));
        me->is_connected = true;
        // status_callback only after ctrlinfo arrives
    }
    else
    {
        me->is_connected = false;
        me->do_status_callbacks();
    }
}

void EPICS_ProcessVariable::ca_ctrlinfo_callback(
    struct event_handler_args args)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)args.usr;
    me->value->read_ctrlinfo(args.dbr);
    me->have_ctrlinfo = true;
    if (!me->pv_value_evid)
    {
        int stat = ca_add_masked_array_event(me->value->get_DBR()+
                                             DBR_TIME_STRING,
                                             1, me->pv_chid,
                                             ca_value_callback,
                                             (void *)me,
                                             (float) 0.0, (float) 0.0,
                                             (float) 0.0,
                                             &me->pv_value_evid,
                                             DBE_VALUE|DBE_ALARM);
        if (stat != ECA_NORMAL)
            printf("CA add event error('%s'): %s\n",
                   me->get_name(), ca_message(stat));
    }
    me->do_status_callbacks();
    me->do_value_callbacks();
}

void EPICS_ProcessVariable::ca_value_callback(struct event_handler_args args)
{
    EPICS_ProcessVariable *me = (EPICS_ProcessVariable *)args.usr;
    me->value->read_value(args.dbr);
    me->do_value_callbacks();
}

bool EPICS_ProcessVariable::is_valid() const
{   return is_connected && have_ctrlinfo; }

const ProcessVariable::Type &EPICS_ProcessVariable::get_type() const
{   return value->get_type(); }   

double EPICS_ProcessVariable::get_double() const
{   return value->get_double(); }

int EPICS_ProcessVariable::get_int() const
{   return value->get_int(); }

size_t EPICS_ProcessVariable::get_string(char *strbuf, size_t buflen) const
{   return value->get_string(strbuf, buflen); }

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


bool EPICS_ProcessVariable::put(double value)
{
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
    if (is_valid())
    {
        dbr_int_t v = value;
        ca_put(DBR_INT, pv_chid, &v);
        return true;
    }
    return false;
}
    
bool EPICS_ProcessVariable::put(const char *value)
{
    if (is_valid())
    {
        ca_bput(pv_chid, value);
        return true;
    }
    return false;
}


// ---------------------- PVValue ---------------------------------
PVValue::PVValue()
{
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
        cvtDoubleToString(get_double(), strbuf, precision);
        if (units[0])
        {
            strcat(strbuf, " ");
            strcat(strbuf, units);
        }
    }
    return strlen(strbuf);
}

size_t PVValue::get_enum_count() const
{   return 0; }

const char *PVValue::get_enum(size_t i) const
{   return "<not enumerated>"; }

// ---------------------- PVValueDouble ---------------------------

static ProcessVariable::Type double_type =
{
    ProcessVariable::Type::real,
    64,
    "real:64"
};
   
const ProcessVariable::Type &PVValueDouble::get_type() const
{   return double_type; }

short PVValueDouble::get_DBR() const
{   return DBF_DOUBLE; }

double PVValueDouble::get_double() const
{   return value; }

void PVValueDouble::read_ctrlinfo(const void *buf)
{
    const  dbr_ctrl_double *val = (const dbr_ctrl_double *)buf;
    status = val->status;
    severity = val->severity;
    precision = val->precision;
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
    value = val->value;
}

void PVValueDouble::read_value(const void *buf)
{
    const  dbr_time_double *val = (const dbr_time_double *)buf;
    time = val->stamp.secPastEpoch + epochSecPast1970;
    nano = val->stamp.nsec;
    status = val->status;
    severity = val->severity;
    value = val->value;
}

// ---------------------- PVValueEnum -----------------------------

PVValueEnum::PVValueEnum()
{   enums = 0; }

static ProcessVariable::Type enum_type =
{
    ProcessVariable::Type::enumerated,
    16,
    "enumerated:16"
};
   
const ProcessVariable::Type &PVValueEnum::get_type() const
{   return enum_type; }


short PVValueEnum::get_DBR() const
{   return DBR_ENUM; }

double PVValueEnum::get_double() const
{   return (double)value; }

int PVValueEnum::get_int() const
{   return value; }

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
