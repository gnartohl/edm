// -*- c++ -*-
//
// epics_pv_factory.h
//
// kasemir@lanl.gov

#ifndef __EPICS_PV_FACTORY_H__
#define __EPICS_PV_FACTORY_H__

#include<cadef.h>
#include"pv_factory.h"

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

class EPICS_ProcessVariable : public ProcessVariable
{
public:
    // Implies 1) connected 2) have received valid control info
    bool is_valid() const;

    const Type &get_type() const;
    
    // ProcessVariable internally holds the "native" value,
    // can be asked for any type -> conversions on client side
    // -- Don't call when is_valid() returns false!!
    double      get_double() const;
    int         get_int() const;
    // writes strbuf, formatted according to precision etc.
    // returns actual strlen
    size_t      get_string(char *strbuf, size_t buflen) const;
    // Get number and (if > 0) strings for enumerated value
    size_t      get_enum_count() const;
    const char *get_enum(size_t i) const;
    // Time: support for seconds in ANSI time_t format (since 1970 UTC)
    // as well as nano second extension
	time_t get_time_t() const;
	unsigned long get_nano() const;
    // Status info, various limits
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

    // Output/write methods
    bool have_write_access() const;
    bool put(double value);
    bool put(int value);
    bool put(const char *value);

private:
    friend class EPICS_PV_Factory;
    
    // hidden, use create/release
    EPICS_ProcessVariable(const char *_name);
    EPICS_ProcessVariable(const ProcessVariable &rhs); // not impl.
    EPICS_ProcessVariable &operator = (const ProcessVariable &rhs); // not impl.
    virtual ~EPICS_ProcessVariable();

    bool is_connected;     // currently connected to CA?
    bool have_ctrlinfo;    // have received DBR_CTRL_XXX
    chid pv_chid;          // CAC channel ID
    evid pv_value_evid;    // CAC event ID
    class PVValue *value;  // current value, type-dependent
    
    static void ca_connect_callback(struct connection_handler_args arg);
    static void ca_ctrlinfo_callback(struct event_handler_args args);
    static void ca_value_callback(struct event_handler_args args);
};

// ----------- Internals of less interest to users ---------------------

// Used by ProcessVariable, handles the 'native' subscription and conversions.
// Virtual base...
class PVValue
{
public:
    PVValue();
    virtual ~PVValue();
    virtual const ProcessVariable::Type &get_type() const = 0;
    virtual short       get_DBR() const = 0;
    virtual double      get_double() const = 0;
    virtual int         get_int() const;
    virtual size_t      get_string(char *strbuf, size_t buflen) const;
    virtual size_t      get_enum_count() const;
    virtual const char *get_enum(size_t i) const;
	time_t get_time_t();
	unsigned long get_nano();

    virtual void read_ctrlinfo(const void *buf) = 0;
    virtual void read_value(const void *buf) = 0;
protected:
    friend class EPICS_ProcessVariable;
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
};

// Implementations for specific types
class PVValueDouble : public PVValue
{
public:
    virtual const ProcessVariable::Type &get_type() const;
    virtual short       get_DBR() const;
    virtual double      get_double() const;
    
    virtual void read_ctrlinfo(const void *buf);
    virtual void read_value(const void *buf);
private:
    double value;
};

class PVValueEnum : public PVValue
{
public:
    PVValueEnum();
    virtual const ProcessVariable::Type &get_type() const;
    virtual short       get_DBR() const;
    virtual double      get_double() const;
    virtual int         get_int() const;
    virtual size_t      get_enum_count() const;
    virtual const char *get_enum(size_t i) const;
    
    virtual void read_ctrlinfo(const void *buf);
    virtual void read_value(const void *buf);
private:
    int    value;
    size_t enums;
    char   strs[MAX_ENUM_STATES][MAX_ENUM_STRING_SIZE];
};

class PVValueString : public PVValue
{
public:
    PVValueString();
    virtual const ProcessVariable::Type &get_type() const;
    virtual short       get_DBR() const;
    virtual double      get_double() const;
    virtual int         get_int() const;
    virtual size_t      get_string(char *strbuf, size_t buflen) const;
    
    virtual void read_ctrlinfo(const void *buf);
    virtual void read_value(const void *buf);
private:
    dbr_string_t value;
};

#endif