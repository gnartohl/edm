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
#include"calc_pv_factory.h"
#include"postfix.h"

static const char *whitespace = " \t\n\r";

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
    fprintf(stderr, "HashedExpression(%s) = %s\n", name, formula);
}

HashedExpression::~HashedExpression()
{
    fprintf(stderr, "HashedExpression(%s) deleted\n", name);
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
    fprintf(stderr, "CALC_PV_Factory created\n");
    if (expressions)
    {
        fprintf(stderr, "Error: More than one CALC_PV_Factory created!\n");
    }
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
    fprintf(stderr, "CALC_PV_Factory deleted\n");
}

bool CALC_PV_Factory::parseFile(const char *filename)
{
    char line[200], name[200];
    size_t len;

    FILE *f = fopen(filename, "rt");
    if (! f)
    {
        fprintf(stderr, "Cannot find '%s' for CALC configuration\n",
                filename);
        return false;
    }

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

ProcessVariable *CALC_PV_Factory::create(const char *PV_name)
{
    const char *arg_name[CALC_ProcessVariable::MaxArgs];
    size_t arg_count = 0;

    // Locate start of expression
    while (strchr(whitespace, *PV_name))
        ++PV_name;
    size_t len = strlen(PV_name);
    if (len <= 0)
    {
        fprintf(stderr, "Empty expression '%s'\n", PV_name);
        return 0;
    }
    char *expression = strdup(PV_name);
    char *p = expression;

    // Locate end of expression name
    const char *end = expression + len;
    while (*p && p<end && !strchr(" \t(", *p))
        ++p;

    if (*p) // anything after end of expression name?
    {
        bool have_args = false;
        // ... start of arg1 in "(arg1, arg1, ...)"
        do
        {
            if (*p == '(')
                have_args = true;
            *(p++) = '\0';
        }
        while (*p && p<end && strchr(" \t(", *p));

        if (p < end && !have_args)
        {
            fprintf(stderr, "Malformed expression '%s'\n", PV_name);
            return 0;
        }

        if (have_args)
        {
            do
            {
                arg_name[arg_count++] = p;
                do
                    ++p;
                while (*p && !strchr(" \t,)", *p));
                if (*p)
                {
                    *p = '\0'; // end of arg, find next arg
                    do
                        ++p;
                    while (*p && strchr(" \t,)", *p));
                }
            }
            while (p && p < end);
        }
    }
    printf("CALC PV: '%s'\n", expression);
    for (size_t i=0; i<arg_count; ++i)
        printf("\targ %d: '%s'\n", i, arg_name[i]);
    
    HashedExpression he;
    he.name = expression;
    ExpressionHash::iterator entry = expressions->find(&he);
    // 'he' will delete the strdup'ed  expression!
    if (entry == expressions->end())
    {
        fprintf(stderr, "Unknown CALC expression '%s'\n", expression);
        return 0;
    }
    return new CALC_ProcessVariable(PV_name,
                                    *entry,
                                    arg_count,
                                    arg_name);
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
        arg[i] = 0.0;
        arg_pv[i] = the_PV_Factory->create(arg_name[i]);
        if (arg_pv[i])
        {
            arg_pv[i]->add_status_callback(status_callback, this);
            arg_pv[i]->add_value_callback(value_callback, this);
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
    fprintf(stderr, "CALC_ProcessVariable(%s) deleted\n", get_name());
    for (size_t i=0; i<arg_count; ++i)
    {
        if (arg_pv[i])
        {
            arg_pv[i]->remove_value_callback(value_callback, this);
            arg_pv[i]->remove_status_callback(status_callback, this);
            arg_pv[i]->release();
            arg_pv[i] = 0;
        }
    }
}

void CALC_ProcessVariable::status_callback(ProcessVariable *pv, void *userarg)
{
    CALC_ProcessVariable *me = (CALC_ProcessVariable *)userarg;
    printf("CALC %s: status change from %s. Overall: %s\n",
           me->get_name(), pv->get_name(),
           (const char *)(me->is_valid() ? "valid" : "invalid"));
    me->recalc();
    me->do_status_callbacks();
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
    
    for (size_t i=0; i<arg_count; ++i)
    {
        if (arg_pv[i] && arg_pv[i]->is_valid())
        {
            arg[i] = arg_pv[i]->get_double();
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
        
    expression->calc(arg, value);

    struct timeval t;
    gettimeofday(&t, 0);
    time = t.tv_sec;
    nano = t.tv_usec*(unsigned long)1000;
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



