// -*- c++ -*-
//
// calc_pv_factory.h
//
// kasemir@lanl.gov

#include<sys/time.h>
#include<unistd.h>   
#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<math.h>
#include"calc_pv_factory.h"
#include"postfix.h"

static const char *whitespace = " \t\n\r";

//#define CALC_DEBUG

// HashedExpression:
// One formula, hashed by name, read from the config file
// and converted into postfix notation.
// Can calc. result when fed arguments.
//
// CALC_PV_Factor:
// Called by main PV_Factory for "CALC\..." ProcessVariables.
// Reads the config file.
// When create("sum(fred, freddy)") is called,
// it looks for the formula "sum"
// and creates a CALC_ProcessVariable for sum with fred & freddy
// as arguments
//
// CALC_ProcessVariable
// Subscribes to arguments, recalculates value of formula
// whenever arguments change.
// If there are no arguments (-> no incoming events),
// timer is used to generate value events.

// ------------------------------------------------------------------
// HashedExpression ----------------------------------------------------
// ------------------------------------------------------------------
HashedExpression::HashedExpression()
{   name = 0; }

HashedExpression::HashedExpression(const char *_name, char *formula)
{
    short error;
    name = strdup(_name);
    if (postfix(formula, this->compiled, &error) != 0)
    {
        fprintf(stderr, "CALC '%s': error in expression '%s'\n",
                name, formula);
        return;
    }
#ifdef CALC_DEBUG
    printf("HashedExpression(%s) = %s\n", name, formula);
#endif
}

HashedExpression::~HashedExpression()
{
#ifdef CALC_DEBUG
    printf("HashedExpression(%s) deleted\n", name);
#endif
    if (name)
    {
        free(name);
        name = 0;
    }
}

bool HashedExpression::calc(const double args[], double &result)
{   return calcPerform(args, &result, compiled) == 0; }

// Required for Hashtable<>:
size_t hash(const HashedExpression *item, size_t N)
{   return generic_string_hash(item->name, N); }

bool equals(const HashedExpression *lhs, const HashedExpression *rhs)
{   return strcmp(lhs->name, rhs->name)==0; }

// ------------------------------------------------------------------
// CALC_PV_Factory --------------------------------------------------
// ------------------------------------------------------------------
enum { HashTableSize = 43 };
typedef Hashtable<HashedExpression,
                  offsetof(HashedExpression, node),
                  HashTableSize> ExpressionHash;
static ExpressionHash *expressions;

CALC_PV_Factory::CALC_PV_Factory()
{
#ifdef CALC_DEBUG
    printf("CALC_PV_Factory created\n");
#endif
    if (expressions)
        fprintf(stderr, "Error: More than one CALC_PV_Factory created!\n");
    else
    {
        expressions = new ExpressionHash();
        parseFile();
    }
}

CALC_PV_Factory::~CALC_PV_Factory()
{
    if (expressions)
    {
        ExpressionHash::iterator i;
        i=expressions->begin();
        while (i!=expressions->end())
        {
            HashedExpression *e = *i;
            expressions->erase(i);
            delete e;
            i=expressions->begin();
        }
        delete expressions;
        expressions = 0;
    }
#ifdef CALC_DEBUG
    printf("CALC_PV_Factory deleted\n");
#endif
}

bool CALC_PV_Factory::parseFile(const char *filename)
{
    char line[200], name[200];
    size_t len;

    FILE *f = fopen(filename, "rt");
    if (!f)
    {
        const char *path=getenv("EDMFILES");
        if (path)
        {
            sprintf(name, "%s/%s", path, filename);
            f = fopen(name, "rt");
        }
    }
    if (!f)
        return false;

    // Check version code
    if (!fgets(line, sizeof line, f)  ||
        strncmp(line, "CALC1", 5))
    {
        fprintf(stderr, "Invalid CALC configuration file '%s'\n",
                filename);
        fclose(f);
        return false;
    }

    // Loop over lines
    bool need_name = true;
    char *p;
    while (fgets(line, sizeof line, f))
    {
        len = strlen(line);
        if (len <= 0 || (len > 0 && line[0] == '#')) // skip comments
            continue;
        // Remove trailing white space
        while (len>0 && strchr(whitespace, line[len-1]))
        {
            line[len-1] = '\0';
            --len;
        }
        if (len <= 0)
            continue;
        // Skip leading white space
        p=line;
        while (strchr(whitespace, *p))
        {
            ++p;
            --len;
        }
        
        if (need_name)
        {
            strcpy(name, p);
            need_name = false;
        }
        else
        {
            need_name = true;
            expressions->insert(new HashedExpression(name, p));
        }
    }
    fclose(f);
    return true;
}

// Fills *arg with copy of next argument found,
// returns 0 or updated position p
static const char *get_arg(const char *p, char **arg)
{
    // find start...
    while (*p && strchr(", \t)", *p))
        ++p;
    if (! *p)
        return 0;

    // find end, including spaces...
    const char *end = p+1;
    int braces = 0;
    while (*end)
    {
        if (*end == '(')
            ++braces;
        else if (*end == ')')
        {
            --braces;
            if (braces < 0)
                break;
        }
        else if (*end == ',')
        {
            if (braces==0)
                break;
        }
        ++end;
    }
    // end is on character NOT to copy: '\0'  ','  ')'
    // remove trailing space
    while (end > p  && strchr(whitespace, *(end-1)))
        --end;
    // copy
    int len = end - p;
    if (len <= 0)
        return 0;
    // Compiler created trash when using *arg instead of temp. narg
    char *narg = (char *)malloc(len+1);
    memcpy(narg, p, len);
    narg[len] = '\0';
    *arg = narg;

    return end;
}

ProcessVariable *CALC_PV_Factory::create(const char *PV_name)
{
    char *arg_name[CALC_ProcessVariable::MaxArgs];
    size_t i, arg_count = 0;
    const char *p;

    for (i=0; i<CALC_ProcessVariable::MaxArgs; ++i)
        arg_name[i] = 0;
    
    // Locate expression: start...
    while (strchr(whitespace, *PV_name))
        ++PV_name;
    p = PV_name;
    // end...
    while (*p && !strchr(" \t(", *p))
        ++p;
    // copy
    int len = p - PV_name; 
    if (len <= 0)
    {
        fprintf(stderr, "Empty expression '%s'\n", PV_name);
        return 0;
    }
    char *expression = (char *)malloc(len+1);
    memcpy(expression, PV_name, len);
    expression[len] = '\0';
    
    while (*p && strchr(whitespace, *p))
        ++p;
    // Do arguments follow?
    if (*p == '(')
    {
        ++p;
        while ((p=get_arg(p, &arg_name[arg_count])) != 0)
            ++arg_count;
    }

#ifdef CALC_DEBUG
    printf("CALC PV: '%s'\n", PV_name);
    printf("\texpression: '%s'\n", expression);
    for (size_t i=0; i<arg_count; ++i)
        printf("\targ %d: '%s'\n", i, arg_name[i]);
#endif    
    HashedExpression he;
    he.name = expression;
    ExpressionHash::iterator entry = expressions->find(&he);
    if (entry == expressions->end())
    {
        fprintf(stderr, "Unknown CALC expression '%s'\n", expression);
        return 0;
    }
    CALC_ProcessVariable *pv =
        new CALC_ProcessVariable(PV_name, *entry,
                                 arg_count, (const char **)arg_name);
    // 'he' will delete the strdup'ed  expression
    for (size_t i=0; i<arg_count; ++i)
        free(arg_name[i]);
    return pv;
}

// ------------------------------------------------------------------
// CALC_ProcessVariable ---------------------------------------------
// ------------------------------------------------------------------

CALC_ProcessVariable::CALC_ProcessVariable(const char *name,
                                           HashedExpression *_expression,
                                           size_t _arg_count,
                                           const char *arg_name[])
        : ProcessVariable(name)
{
    size_t i;

    precision = 4;
    upper_display = 10.0;
    lower_display = 0.0;
    upper_alarm = DBL_MAX;
    lower_alarm = -DBL_MAX;
    upper_warning = DBL_MAX;
    lower_warning = -DBL_MAX;
    upper_ctrl = 10.0;
    lower_ctrl = 0.0;
    
    expression = _expression;
    arg_count = _arg_count;
    for (i=0; i<arg_count; ++i)
    {
        // Poor excuse for a real "number" check:
        if (strchr("0123456789+-.", arg_name[i][0]) &&
            strspn(arg_name[i], "0123456789+-.eE"))
        {
            arg_pv[i] = 0;
            arg[i] = strtod(arg_name[i], 0);
            if (arg[i] == HUGE_VAL ||
                arg[i] == -HUGE_VAL)
            {
                fprintf(stderr, "CALC PV %s: invalid number arg '%s'\n",
                        name, arg_name[i]);
                arg[i] = 0.0;
            }
        }
        else
        {
            arg[i] = 0.0;
            arg_pv[i] = the_PV_Factory->create(arg_name[i]);
            if (arg_pv[i])
            {
                arg_pv[i]->add_conn_state_callback(status_callback, this);
                arg_pv[i]->add_value_callback(value_callback, this);
            }
            else
            {
                fprintf(stderr, "CALC PV %s: invalid PV arg '%s'\n",
                        name, arg_name[i]);
            }
        }
    }
    for (/**/; i<MaxArgs; ++i)
    {
        arg[i] = 0.0;
        arg_pv[i] = 0;
    }
}

CALC_ProcessVariable::~CALC_ProcessVariable()
{
#ifdef CALC_DEBUG
    printf("CALC_ProcessVariable(%s) deleted\n", get_name());
#endif
    for (size_t i=0; i<arg_count; ++i)
    {
        if (arg_pv[i])
        {
            arg_pv[i]->remove_value_callback(value_callback, this);
            arg_pv[i]->remove_conn_state_callback(status_callback, this);
            arg_pv[i]->release();
            arg_pv[i] = 0;
        }
    }
}

void CALC_ProcessVariable::status_callback(ProcessVariable *pv, void *userarg)
{
    CALC_ProcessVariable *me = (CALC_ProcessVariable *)userarg;
#ifdef CALC_DEBUG
    printf("CALC %s: status change from %s. Overall: %s\n",
           me->get_name(), pv->get_name(),
           (const char *)(me->is_valid() ? "valid" : "invalid"));
#endif
    me->recalc();
    me->do_conn_state_callbacks();
}

void CALC_ProcessVariable::value_callback(ProcessVariable *pv, void *userarg)
{
    CALC_ProcessVariable *me = (CALC_ProcessVariable *)userarg;
    me->recalc();
    me->do_value_callbacks();
}

void CALC_ProcessVariable::recalc()
{
    status = 0;
    severity = 0;
    time = 0;
    nano = 0;
    // Evaluate arguments, if any:
    for (size_t i=0; i<arg_count; ++i)
    {
        if (!arg_pv[i])
            continue;
        if (arg_pv[i]->is_valid())
        {
            arg[i] = arg_pv[i]->get_double();
            // Maximize time stamp and severity
            if (arg_pv[i]->get_time_t() > time  ||
                (arg_pv[i]->get_time_t() == time  ||
                 arg_pv[i]->get_nano() > nano))
            {
                time = arg_pv[i]->get_time_t();
                nano = arg_pv[i]->get_nano();
            }
            if (arg_pv[i]->get_severity() > severity)
            {
                severity = arg_pv[i]->get_severity();
                status   = arg_pv[i]->get_status();
            }
        }
        else
        {
            status = UDF_ALARM;
            severity = INVALID_ALARM;
        }
    }
        
    if (severity != INVALID_ALARM)
        expression->calc(arg, value);

    if (time == 0)
    {
        struct timeval t;
        gettimeofday(&t, 0);
        time = t.tv_sec;
        nano = t.tv_usec*(unsigned long)1000;
    }
}

bool CALC_ProcessVariable::is_valid() const
{   // invalid if any argument is invalid
    for (size_t i=0; i<arg_count; ++i)
    {
        if (arg_pv[i] && !arg_pv[i]->is_valid())
            return false;
    }
    return true;
}

static ProcessVariable::Type calc_type =
{
    ProcessVariable::Type::real,
    64,
    "real:64"
};

const ProcessVariable::Type &CALC_ProcessVariable::get_type() const
{   return calc_type; }

double CALC_ProcessVariable::get_double() const
{   return value; }

size_t CALC_ProcessVariable::get_dimension() const
{   return 1; }

const int *CALC_ProcessVariable::get_int_array() const
{   return 0; }

const double *CALC_ProcessVariable::get_double_array() const
{   return &value; }

time_t CALC_ProcessVariable::get_time_t() const
{   return time; }

unsigned long CALC_ProcessVariable::get_nano() const
{   return nano; }

short CALC_ProcessVariable::get_status() const
{   return status; }

short CALC_ProcessVariable::get_severity() const
{   return severity; }

short CALC_ProcessVariable:: get_precision() const
{   return precision; }

double CALC_ProcessVariable::get_upper_disp_limit() const
{   return upper_display; }

double CALC_ProcessVariable::get_lower_disp_limit() const
{   return lower_display; }

double CALC_ProcessVariable::get_upper_alarm_limit() const
{   return upper_alarm; }

double CALC_ProcessVariable::get_upper_warning_limit() const
{   return upper_warning; }

double CALC_ProcessVariable::get_lower_warning_limit() const
{   return lower_warning; }

double CALC_ProcessVariable::get_lower_alarm_limit() const
{   return lower_alarm; }

double CALC_ProcessVariable::get_upper_ctrl_limit() const
{   return upper_ctrl; }

double CALC_ProcessVariable::get_lower_ctrl_limit() const
{   return lower_ctrl; }

bool CALC_ProcessVariable::put(double value)
{   return false; }

bool CALC_ProcessVariable::put(const char *value)
{   return false; }



