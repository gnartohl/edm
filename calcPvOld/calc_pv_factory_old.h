// -*- c++ -*-
//
// calc_pv_factory_old.h
//
// kasemir@lanl.gov

// new version 8/23/2011 sinclairjw@ornl.gov

#ifndef __CALC_PV_FACTORY_OLD_H__
#define __CALC_PV_FACTORY_OLD_H__

#include"postfix.h"
#include"pv_factory.h"
#include"expString.h"

#define CALC_FILENAME "calc.list"
#define CALC_ENV "EDMCALC"

class HashedExpression
{
public:
    HashedExpression();
    HashedExpression(const char *name, char *formula,
     char *rewriteString );
    ~HashedExpression();
    
    char *name;
    
    bool calc(const double args[], double &result);
    
    // Required for Hashtable<>:
    DLNode node;
    expStringClass expStr;
private:
    char compiled[MAX_POSTFIX_SIZE+1];
};

class CALC_PV_Factory : public PV_Factory
{
public:
    CALC_PV_Factory();
    ~CALC_PV_Factory();

    // Called on initialization with default file.
    // Could be called later to add new files?
    static bool parseFile(const char *filename);
    
    // Result is referenced once, release when no longer needed.
    class ProcessVariable *create(const char *PV_name);
};

class CALC_ProcessVariable : public ProcessVariable
{
public:
    // ProcessVariable methods
    bool is_valid() const;
    const Type &get_type() const;
    const specificType &get_specific_type() const;
    double get_double() const;
    size_t get_dimension() const;
    const char *get_char_array() const;
    const short *get_short_array() const;
    const int *get_int_array() const;
    const double *get_double_array() const;
    time_t get_time_t() const;
    unsigned long get_nano() const;
    short get_status() const;
    short get_severity() const;
    short get_precision() const;
    double get_upper_disp_limit() const;
    double get_lower_disp_limit() const;
    double get_upper_alarm_limit() const;
    double get_upper_warning_limit() const;
    double get_lower_warning_limit() const;
    double get_lower_alarm_limit() const;
    double get_upper_ctrl_limit() const;
    double get_lower_ctrl_limit() const;
    bool put(double value);
    bool put(const char *value);
    bool put(int value);
    bool putText(char *value);
    bool putArrayText(char *value);

protected:
    friend class CALC_PV_Factory;
    // hidden, use CALC_PV_Factory::create()/ProcessVariable::release()
    CALC_ProcessVariable(const char *name,
                         HashedExpression *expression,
                         size_t arg_count, const char *arg_names[]);
    //CALC_ProcessVariable(); // not impl.
    CALC_ProcessVariable(const ProcessVariable &rhs); // not impl.
    CALC_ProcessVariable &operator = (const ProcessVariable &rhs); // not impl.
    ~CALC_ProcessVariable();

    HashedExpression *expression;
    // Arguments A, B, ...: value and source PVs
    enum { MaxArgs = CALCPERFORM_NARGS };
    
    double arg[MaxArgs];
    ProcessVariable *arg_pv[MaxArgs];
    size_t arg_count; // 0..MaxArgs

    void recalc();
    
    double value;
    time_t time;
    unsigned long nano;
    short status, severity, precision;
    double upper_display, lower_display;
    double upper_alarm, lower_alarm;
    double upper_warning, lower_warning;
    double upper_ctrl, lower_ctrl;

    // Registered with each used arg_pv
    static void status_callback(ProcessVariable *pv, void *userarg);
    static void value_callback(ProcessVariable *pv, void *userarg);
};

#ifdef __cplusplus
extern "C" {
#endif

ProcessVariable *create_CALCPtr (
  const char *PV_name
);

#ifdef __cplusplus
}
#endif

#endif
