// -*- c++ -*-
//
// loc_pv_factory.h
//
// sinclair@mail.phy.ornl.gov

#ifndef __LOC_PV_FACTORY_OLD_H__
#define __LOC_PV_FACTORY_OLD_H__

#include "pv_factory.h"

class LOC_ProcessVariable;

class LOC_PV_Factory : public PV_Factory
{
public:
    LOC_PV_Factory();
    ~LOC_PV_Factory();
    ProcessVariable *create(const char *PV_name);
private:
    friend class LOC_ProcessVariable;
    static void forget(LOC_ProcessVariable *pv);
};

typedef struct con_args {
  void *ptr;
  void *addr;
  void *usrArg;
} conHndArgsType, *conHndArgsPtr;

typedef struct ev_args {
  void *ptr;
  void *addr;
  void *usrArg;
} eventArgsType, *eventArgsPtr;

// See comments on ProcessVariable for API
class LOC_ProcessVariable : public ProcessVariable
{
public:
    static const int MAX_BUF_CHARS = 63;
    static const int MAX_UNITS_LEN = 12;
    static const size_t MAX_ENUM_NUM = 16;
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
    const char *get_enum(size_t i) const;

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

    bool have_write_access() const;
    bool put(double value);
    bool put(int value);
    bool put(const char *value);
    bool putText(char *value);
    bool putArrayText(char *value);

private:
    friend class LOC_PV_Factory;

    int setAttributes (
      char *string
    );

    // hidden, use create/release
    LOC_ProcessVariable(const char *_name);
    LOC_ProcessVariable(const ProcessVariable &rhs); // not impl.
    LOC_ProcessVariable &operator = (const ProcessVariable &rhs); // not impl.
    virtual ~LOC_ProcessVariable();

    bool is_connected;     // currently connected to CA?
    bool have_ctrlinfo;    // have received DBR_CTRL_XXX

    int bufLen;
    char buf[MAX_BUF_CHARS+1];
    char dataType;
    char *enums[MAX_ENUM_NUM];
    size_t numEnumStates;
    int startVal;
    int endVal;
    int period;

    time_t  time;
    unsigned long nano;

    short   status;
    short   severity;
    short   precision;
    char    units[MAX_UNITS_LEN+1];
    double  upper_disp_limit;
    double  lower_disp_limit;
    double  upper_alarm_limit;  
    double  upper_warning_limit;
    double  lower_warning_limit;
    double  lower_alarm_limit;
    double  upper_ctrl_limit;
    double  lower_ctrl_limit;

    typedef struct conHndArgsTag {
      LOC_ProcessVariable *lpv;
      void *userArg;
    } conHndArgsType, *conHndArgsPtr;
    
    typedef struct eventArgsTag {
      LOC_ProcessVariable *lpv;
      void *userArg;
    } eventArgsType, *eventArgsPtr;

    static void connect_callback(conHndArgsType arg);
    static void value_callback(eventArgsType args);
};

#ifdef __cplusplus
extern "C" {
#endif

ProcessVariable *create_LOCPtr (
  const char *PV_name
);

#ifdef __cplusplus
}
#endif

#endif

