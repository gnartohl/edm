// loc_pv_factory.cc
//
// sinclair@mail.phy.ornl.gov

#include<stdio.h>
#include<stdlib.h>
#include<float.h>

#include"loc_pv_factory_old.h"

static PV_Factory *loc_pv_factory = new LOC_PV_Factory();

//#define DEBUG_LOC

static ProcessVariable::Type integer_type =
{ ProcessVariable::Type::integer, 32, "integer:32" };

static ProcessVariable::Type double_type =
{ ProcessVariable::Type::real, 64, "real:64" };

static ProcessVariable::Type enum_type =
{ ProcessVariable::Type::enumerated, 16, "enumerated:16" };
   
static ProcessVariable::Type string_type =
{ ProcessVariable::Type::text, 0, "text:0" };

// specific types
static ProcessVariable::specificType i_type =
{ ProcessVariable::specificType::integer, 32 };

static ProcessVariable::specificType d_type =
{ ProcessVariable::specificType::real, 64 };

static ProcessVariable::specificType e_type =
{ ProcessVariable::specificType::enumerated, 16 };
   
static ProcessVariable::specificType str_type =
{ ProcessVariable::specificType::text, 0 };

/* 1/1/90 20 yr (5 leap) of seconds */
static const unsigned epochSecPast1970 = 7305*86400;

// --------------------- LOC_PV_Factory -------------------------------

// All LOC_PV_Factories share the same static PV pool,
// a hashtable by PV name
//enum { HashTableSize=43 };
enum { HashTableSize=503 };
class LocHashTableItem
{
public:
    const char *name;
    void *context;
    LOC_ProcessVariable *pv;
    DLNode node;
};

// Helpers for Hashtable template to use the LocHashTableItem
size_t hash(const LocHashTableItem *item, size_t N)
{   return generic_string_hash(item->name, N); }

bool equals(const LocHashTableItem *lhs, const LocHashTableItem *rhs)
{   

  return ( strcmp(lhs->name, rhs->name) == 0 );

}

typedef Hashtable<LocHashTableItem,
                  offsetof(LocHashTableItem,node),
                  HashTableSize> PVHash;
static PVHash processvariables;

LOC_PV_Factory::LOC_PV_Factory()
{
#ifdef DEBUG_LOC
    fprintf( stderr,"LOC_PV_Factory created\n");
#endif
}

LOC_PV_Factory::~LOC_PV_Factory()
{
#ifdef DEBUG_LOC
    fprintf( stderr,"LOC_PV_Factory deleted\n");
#endif
}

ProcessVariable *LOC_PV_Factory::create(const char *PV_name)
{

    char tmp[PV_Factory::MAX_PV_NAME+1], *tk, *ctx;

    LOC_ProcessVariable *pv;
    LocHashTableItem item;

    strncpy( tmp, PV_name, PV_Factory::MAX_PV_NAME );
    tmp[PV_Factory::MAX_PV_NAME] = 0;

    ctx = NULL;
    tk = strtok_r( tmp, "=~", &ctx );

    //item.name = PV_name;
    item.name = tk;
    PVHash::iterator entry = processvariables.find(&item);
    if (entry != processvariables.end()) {
        pv = (*entry)->pv;
        pv->reference();
    }
    else {
        LocHashTableItem *n_item = new LocHashTableItem();
        pv = new LOC_ProcessVariable(tk);
        n_item->name = pv->get_name();
        n_item->pv = pv;
        processvariables.insert(n_item);
        tk = strtok_r( NULL, "=~", &ctx );
        pv->setAttributes( tk );
	strcpy( pv->units, "" );
    }
    return pv;
}

void LOC_PV_Factory::forget(LOC_ProcessVariable *pv)
{
    LocHashTableItem item;
    item.name = pv->get_name();
    PVHash::iterator entry = processvariables.find(&item);
    if (entry != processvariables.end())
    {
        LocHashTableItem *p_item = *entry;
        processvariables.erase(entry);
        delete p_item;
        return;
    }
    fprintf(stderr,"LOC_PV_Factory: internal error in 'forget', PV %s\n",
            pv->get_name());
}

// ---------------------- LOC_ProcessVariable -------------------------

int LOC_ProcessVariable::setAttributes (
  char *string
) {

  // atributes should look like <type>:<value> or, if enum,
  // e:<num value>,<state1>,<state2>,...

int i;
char tmp[PV_Factory::MAX_PV_NAME+1], *tk, *ctx;

  numEnumStates = 0;
  dataType = 's';
  strcpy( buf, "" );
  bufLen = 0;

  if ( !string ) return 0;
  if ( strlen(string) < 1 ) return 0;

  i = 0;

  dataType = string[i]; i++;

  switch ( dataType ) {

  case 's':

    status = 0;
    severity = 0;

    if ( &string[i] ) i++;
    if ( &string[i] ) {
      strncpy( buf, &string[i], MAX_BUF_CHARS );
      buf[MAX_BUF_CHARS] = 0;
    }
    else {
      strcpy( buf, "" );
    }
    bufLen = strlen( buf );
    break;

  case 'd':
  case 'i':

    status = 0;
    severity = 0;

    if ( &string[i] ) i++;
    if ( &string[i] ) {
      strncpy( buf, &string[i], MAX_BUF_CHARS );
      buf[MAX_BUF_CHARS] = 0;
    }
    else {
      strcpy( buf, "0" );
    }
    bufLen = strlen( buf );
    break;

  case 'e':

    status = 0;
    severity = 0;

    if ( &string[i] ) i++;
    if ( &string[i] ) {

      strncpy( tmp, &string[i], MAX_BUF_CHARS );
      tmp[MAX_BUF_CHARS] = 0;

      ctx = NULL;
      tk = strtok_r( tmp, ",;", &ctx );
      if ( tk ) {
        strncpy( buf, tk, MAX_BUF_CHARS );
        buf[MAX_BUF_CHARS] = 0;
        bufLen = strlen( buf );
      }
      else {
        strcpy( buf, "0" );
        bufLen = strlen( buf );
        numEnumStates = 2;
        enums[0] = new char[2];
        strcpy( enums[0], "0" );
        enums[1] = new char[2];
        strcpy( enums[1], "1" );
        break;
      }

      numEnumStates = 0;
      tk = strtok_r( NULL, ",;", &ctx );

      while ( tk ) {

        enums[numEnumStates] = new char[strlen(tk)+1];
        strcpy( enums[numEnumStates], tk );
        if ( numEnumStates < MAX_ENUM_NUM ) numEnumStates++;

        tk = strtok_r( NULL, ",;", &ctx );

      }

    }
    else {

      strcpy( buf, "0" );
      bufLen = strlen( buf );
      numEnumStates = 2;
      enums[0] = new char[2];
      strcpy( enums[0], "0" );
      enums[1] = new char[2];
      strcpy( enums[1], "1" );
      break;

    }

    break;

  default:

    status = 0;
    severity = 0;

    dataType = 's';

    if ( &string[i] ) i++;
    if ( &string[i] ) {
      strncpy( buf, &string[i], MAX_BUF_CHARS );
      buf[MAX_BUF_CHARS] = 0;
    }
    else {
      strcpy( buf, "" );
    }
    bufLen = strlen( buf );

    break;

  }

  //fprintf( stderr, "LOC_ProcessVariable::setAttributes, string = [%s]\n", string );

  return 1;

}

LOC_ProcessVariable::LOC_ProcessVariable(const char *_name)
        : ProcessVariable(_name)
{
    status = UDF_ALARM;
    severity = INVALID_ALARM;
    precision = 4;
    upper_disp_limit = 100.0;
    lower_disp_limit = -100.0;
    upper_alarm_limit = DBL_MAX;
    lower_alarm_limit = -DBL_MAX;
    upper_warning_limit = DBL_MAX;
    lower_warning_limit = -DBL_MAX;
    upper_ctrl_limit = 100.0;
    lower_ctrl_limit = -100.0;
    is_connected = true;
    have_ctrlinfo = true;
    strcpy( buf, "" );
    bufLen = 0;
    //fprintf( stderr,"LOC_ProcessVariable %s created\n", get_name());
}

LOC_ProcessVariable::~LOC_ProcessVariable()
{
    LOC_PV_Factory::forget(this);
}

void LOC_ProcessVariable::connect_callback(
    conHndArgsType arg)
{

  //fprintf( stderr, "connect_callback\n" );

  // me->is_connected = true;
  // do callback

}

void LOC_ProcessVariable::value_callback (
  eventArgsType args)
{

  //fprintf( stderr, "value_callback\n" );

  //LOC_ProcessVariable *me = (LOC_ProcessVariable *) ?;

  //me->do_value_callbacks();

}

bool LOC_ProcessVariable::is_valid() const
{

  return true;

}

const ProcessVariable::Type &LOC_ProcessVariable::get_type() const
{

  switch ( dataType ) {

  case 'd':

    return double_type;

  case 'i':

    return integer_type;

  case 's':

    return string_type;

  case 'e':

    return enum_type;

  }

  return string_type;

}   

const ProcessVariable::specificType &LOC_ProcessVariable::get_specific_type() const
{

  switch ( dataType ) {

  case 'd':

    return d_type;

  case 'i':

    return i_type;

  case 's':

    return str_type;

  case 'e':

    return e_type;

  }

  return str_type;

}   

int LOC_ProcessVariable::get_int() const
{

int i = atol( buf );

  //fprintf( stderr, "[%s] int value is %-d\n", get_name(), i );
  return i;

}

double LOC_ProcessVariable::get_double() const
{

double d = atof( buf );

  if ( strcmp( buf, "RAND()" ) == 0 ) {
    d = drand48();
  }

  //fprintf( stderr, "[%s] double value is %-.15g\n", get_name(), d );
  return d;

}

size_t LOC_ProcessVariable::get_string(char *strbuf, size_t len) const
{

size_t l = bufLen;

  if ( len < l ) l = len;
  strncpy( strbuf, buf, l );
  strbuf[l] = 0;

  return l;

}

size_t LOC_ProcessVariable::get_dimension() const
{

  return 1;

}

const char *LOC_ProcessVariable::get_char_array() const
{

  return (char *) NULL;

}

const short *LOC_ProcessVariable::get_short_array() const
{

  return (short *) NULL;

}

const int *LOC_ProcessVariable::get_int_array() const
{

  return (int *) NULL;

}

const double * LOC_ProcessVariable::get_double_array() const
{

  return (double *) NULL;

}

size_t LOC_ProcessVariable::get_enum_count() const
{

  return numEnumStates;

}

const char *LOC_ProcessVariable::get_enum(size_t i) const
{

  if ( ( i >= 0 ) && ( i < numEnumStates ) ) {
    return enums[i];
  }
  else {
    return NULL;
  }

}

time_t LOC_ProcessVariable::get_time_t() const
{

  return time;

}

unsigned long LOC_ProcessVariable::get_nano() const
{

  return nano;

}

short LOC_ProcessVariable::get_status() const
{

  return status;

}

short LOC_ProcessVariable::get_severity() const
{

  return severity;

}

short LOC_ProcessVariable::get_precision() const
{

  return precision;

}

const char *LOC_ProcessVariable::get_units() const
{

  return units;

}

double LOC_ProcessVariable::get_upper_disp_limit() const
{

  return upper_disp_limit;

}

double LOC_ProcessVariable::get_lower_disp_limit() const
{

  return lower_disp_limit;

}

double LOC_ProcessVariable::get_upper_alarm_limit() const
{

  return upper_alarm_limit;

}

double LOC_ProcessVariable::get_upper_warning_limit() const
{

  return upper_warning_limit;

}

double LOC_ProcessVariable::get_lower_warning_limit() const
{

  return lower_warning_limit;

}

double LOC_ProcessVariable::get_lower_alarm_limit() const
{

  return lower_alarm_limit;

}

double LOC_ProcessVariable::get_upper_ctrl_limit() const
{

  return upper_ctrl_limit;

}

double LOC_ProcessVariable::get_lower_ctrl_limit() const
{

  return lower_ctrl_limit;

}

bool LOC_ProcessVariable::have_write_access() const
{

  return true;

}

bool LOC_ProcessVariable::put(double value)
{

  //fprintf( stderr, "[%s] put double, value = %-.15g\n", get_name(), value );

  status = 0;
  severity = 0;

  snprintf( buf, MAX_BUF_CHARS, "%-.15g", value );
  bufLen = strlen( buf );
  do_value_callbacks();

  return true;

}

bool LOC_ProcessVariable::put(int value)
{

  //fprintf( stderr, "[%s] put int, value = %-d\n", get_name(), value );

  status = 0;
  severity = 0;

  snprintf( buf, MAX_BUF_CHARS, "%-d", value );
  bufLen = strlen( buf );
  do_value_callbacks();

  return true;

}
    
bool LOC_ProcessVariable::put(const char *value)
{

int l;

  status = 0;
  severity = 0;

  if ( strlen(value)+1 > (unsigned int) MAX_BUF_CHARS ) {
    l = MAX_BUF_CHARS;
  }
  else {
    l = strlen(value);
  }

  strncpy( buf, value, l );
  buf[l] = 0;
  bufLen = strlen( buf );
  do_value_callbacks();

  return true;

}

bool LOC_ProcessVariable::putText(char *value)
{

int l;

  status = 0;
  severity = 0;

  if ( strlen(value)+1 > (unsigned int) MAX_BUF_CHARS ) {
    l = MAX_BUF_CHARS;
  }
  else {
    l = strlen(value);
  }

  strncpy( buf, value, l );
  buf[l] = 0;
  bufLen = strlen( buf );
  do_value_callbacks();

  return true;

}

bool LOC_ProcessVariable::putArrayText(char *value)
{

int l;

  status = 0;
  severity = 0;

  if ( strlen(value)+1 > (unsigned int) MAX_BUF_CHARS ) {
    l = MAX_BUF_CHARS;
  }
  else {
    l = strlen(value);
  }

  strncpy( buf, value, l );
  buf[l] = 0;
  bufLen = strlen( buf );
  do_value_callbacks();

  return true;

}

#ifdef __cplusplus
extern "C" {
#endif

ProcessVariable *create_LOCPtr (
  const char *PV_name
) {

ProcessVariable *ptr;

  ptr = loc_pv_factory->create( PV_name );
  return ptr;

}

#ifdef __cplusplus
}
#endif
