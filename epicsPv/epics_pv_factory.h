// -*- c++ -*-
//
// epics_pv_factory.h
//
// kasemir@lanl.gov

#ifndef __EPICS_PV_FACTORY_H__
#define __EPICS_PV_FACTORY_H__

#include<cadef.h>
#include"pv_factory.h"

class EPICS_ProcessVariable;

class EPICS_PV_Factory : public PV_Factory
{
public:
    EPICS_PV_Factory();
    ~EPICS_PV_Factory();
    ProcessVariable *create(const char *PV_name);
private:
    friend class EPICS_ProcessVariable;
    static void forget(EPICS_ProcessVariable *pv);
};

// See comments on ProcessVariable for API
class EPICS_ProcessVariable : public ProcessVariable
{
public:
    bool is_valid() const;
    const Type &get_type() const;
    const specificType &get_specific_type() const;
    int         get_int() const;
    double      get_double() const;
    size_t      get_string(char *strbuf, size_t buflen) const;
    size_t      get_dimension() const;
    const char  *get_char_array() const;
    const short *get_short_array() const;
    const int   *get_int_array() const;
    const double *get_double_array() const;
    size_t      get_enum_count() const;
    const char  *get_enum(size_t i) const;

	time_t get_time_t() const;
	unsigned long get_nano() const;

    short       get_status() const;
    short       get_severity() const;
    short       get_precision() const;
    const char *get_units() const;
    double      get_upper_disp_limit() const;
    double      get_lower_disp_limit() const;
    double      get_upper_alarm_limit() const;  
    double      get_upper_warning_limit() const;
    double      get_lower_warning_limit() const;
    double      get_lower_alarm_limit() const;
    double      get_upper_ctrl_limit() const;
    double      get_lower_ctrl_limit() const;

    bool have_read_access() const;
    bool have_write_access() const;
    bool put(double value);
    bool put(int value);
    bool put(const char *value);
    bool putText(char *value);
    bool putArrayText(char *value);
    bool putAck(short value);

    virtual bool is_epics() const { return true; }

private:
    friend class EPICS_PV_Factory;
    
    // hidden, use create/release
    EPICS_ProcessVariable(const char *_name);
    EPICS_ProcessVariable(const ProcessVariable &rhs); // not impl.
    EPICS_ProcessVariable &operator = (const ProcessVariable &rhs); // not impl.
    virtual ~EPICS_ProcessVariable();

    void processExistingPv ( void );

    bool is_connected;     // currently connected to CA?
    bool have_ctrlinfo;    // have received DBR_CTRL_XXX
    chid pv_chid;          // CAC channel ID
    evid pv_value_evid;    // CAC event ID
    class PVValue *value;  // current value, type-dependent
    bool read_access;
    bool write_access;
    
    static void ca_connect_callback(struct connection_handler_args arg);
    static void ca_ctrlinfo_callback(struct event_handler_args args);
    static void ca_ctrlinfo_refresh_callback(struct event_handler_args args);
    static void ca_value_callback(struct event_handler_args args);
    static void ca_access_security_callback(struct access_rights_handler_args args);

};

// ----------- Internals of less interest to users ---------------------

// Used by ProcessVariable, handles the 'native' subscription and conversions.
// Virtual base...
class PVValue
{
public:
    PVValue(EPICS_ProcessVariable *epv);
    virtual ~PVValue();
    virtual const ProcessVariable::Type &get_type() const = 0;
    virtual const ProcessVariable::specificType &get_specific_type() const;
    virtual short       get_DBR() const = 0;
    virtual int         get_int() const;
    virtual double      get_double() const = 0;
    virtual size_t      get_string(char *strbuf, size_t buflen) const;
    virtual const char *get_char_array() const;
    virtual const short *get_short_array() const;
    virtual const int  *get_int_array() const;
    virtual const double *get_double_array() const;
    virtual size_t      get_enum_count() const;
    virtual const char *get_enum(size_t i) const;
	time_t get_time_t();
	unsigned long get_nano();

    virtual void read_ctrlinfo(const void *buf) = 0;
    virtual void read_value(const void *buf) = 0;
protected:
    friend class EPICS_ProcessVariable;
    EPICS_ProcessVariable *epv;
	time_t  time;
	unsigned long nano;
    short   status;
    short   severity;
    short   precision;
    char    units[MAX_UNITS_SIZE+1];
    double  upper_disp_limit;
    double  lower_disp_limit;
    double  upper_alarm_limit;  
    double  upper_warning_limit;
    double  lower_warning_limit;
    double  lower_alarm_limit;
    double  upper_ctrl_limit;
    double  lower_ctrl_limit;
    ProcessVariable::specificType specific_type;
};

// Implementations for specific types
class PVValueInt : public PVValue
{
public:
    PVValueInt(EPICS_ProcessVariable *epv);
    PVValueInt(EPICS_ProcessVariable *epv, const char* typeInfo);
    ~PVValueInt();
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    int         get_int() const;
    double      get_double() const;
    size_t      get_string(char *strbuf, size_t buflen) const;
    const int  *get_int_array() const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    int *value;
};

class PVValueDouble : public PVValue
{
public:
    PVValueDouble(EPICS_ProcessVariable *epv);
    PVValueDouble(EPICS_ProcessVariable *epv, const char* typeInfo);
    ~PVValueDouble();
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    double      get_double() const;
    const double *get_double_array() const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    double *value;
};

class PVValueEnum : public PVValue
{
public:
    PVValueEnum(EPICS_ProcessVariable *epv);
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    int         get_int() const;
    double      get_double() const;
    size_t      get_enum_count() const;
    const char *get_enum(size_t i) const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    int    value;
    size_t enums;
    char   strs[MAX_ENUM_STATES][MAX_ENUM_STRING_SIZE];
};

class PVValueString : public PVValue
{
public:
    PVValueString(EPICS_ProcessVariable *epv);
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    int         get_int() const;
    double      get_double() const;
    size_t      get_string(char *strbuf, size_t buflen) const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    dbr_string_t value;
};

class PVValueChar : public PVValue
{
public:
    PVValueChar(EPICS_ProcessVariable *epv);
    ~PVValueChar();
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    int         get_int() const;
    double      get_double() const;
    const char * get_char_array() const;
    size_t      get_string(char *strbuf, size_t buflen) const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    char *value;
    size_t len;
};

class PVValueShort : public PVValue
{
public:
    PVValueShort(EPICS_ProcessVariable *epv);
    ~PVValueShort();
    const ProcessVariable::Type &get_type() const;
    short       get_DBR() const;
    int         get_int() const;
    double      get_double() const;
    const short *get_short_array() const;
    size_t      get_string(char *strbuf, size_t buflen) const;
    void read_ctrlinfo(const void *buf);
    void read_value(const void *buf);
private:
    short *value;
    size_t len;
};

#ifdef __cplusplus
extern "C" {
#endif

int epics_pend_io (
  double sec
);

int epics_pend_event (
  double sec
);

void epics_task_exit ( void );

ProcessVariable *create_EPICSPtr (
  const char *PV_name
);

#ifdef __cplusplus
}
#endif

#endif

