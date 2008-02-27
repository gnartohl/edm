#include"pv_factory.h"
#include"textupdate.h"
#include"regTextupdate.h"
#include"strip.h"
#include"byte.h"
#include"environment.str"
#include"pv_register.str"

// --------------------------------------------------------
// Registration
// --------------------------------------------------------

typedef struct
{
    char *className;
    char *typeName;
    char *text;
} libRecType;

static int libRecIndex = 0;

static libRecType exported[] =
{
    { TEXTUPDATE_CLASSNAME, global_str2, reg_str1 },
    { REGTEXTUPDATE_CLASSNAME, global_str2, reg_str2 },
    { TEXTENTRY_CLASSNAME, global_str5, reg_str3 },
    { BYTE_CLASSNAME, global_str2, reg_str4 }
};

extern "C"
{

    char *author ( void ) {

    static char *a = "Kay Kasemir (kasemirk@ornl.gov)";

      return a;

    }

    int firstRegRecord(char **className, char **typeName, char **text)
    {
        *className = exported[0].className;
        *typeName  = exported[0].typeName;
        *text      = exported[0].text;
        libRecIndex = 1;
        return 0;
    }
  
    int nextRegRecord(char **className, char **typeName, char **text)
    {
        int max = sizeof(exported) / sizeof(exported[0]);
        if (libRecIndex >= max)
            return -1; //no more 
        *className = exported[libRecIndex].className;
        *typeName  = exported[libRecIndex].typeName;
        *text      = exported[libRecIndex].text;
        libRecIndex++;
        return 0;
    }

    void *create_TextupdateClassPtr (void)
    {
        edmTextupdateClass *obj = new edmTextupdateClass;
        return (void *) obj;
    }
    
    void *clone_TextupdateClassPtr (void *rhs)
    {
        edmTextupdateClass *src = (edmTextupdateClass *) rhs;
        edmTextupdateClass *obj = new edmTextupdateClass(src);
        return (void *) obj;
    }

    void *create_RegTextupdateClassPtr (void)
    {
        edmRegTextupdateClass *obj = new edmRegTextupdateClass;
        return (void *) obj;
    }
    
    void *clone_RegTextupdateClassPtr (void *rhs)
    {
        edmRegTextupdateClass *src = (edmRegTextupdateClass *) rhs;
        edmRegTextupdateClass *obj = new edmRegTextupdateClass(src);
        return (void *) obj;
    }

    void *create_TextentryClassPtr (void)
    {
        edmTextentryClass *obj = new edmTextentryClass;
        return (void *) obj;
    }
    
    void *clone_TextentryClassPtr (void *rhs)
    {
        edmTextentryClass *src = (edmTextentryClass *) rhs;
        edmTextentryClass *obj = new edmTextentryClass(src);
        return (void *) obj;
    }

    void *create_ByteClassPtr (void)
    {
        edmByteClass *obj = new edmByteClass;
        return (void *) obj;
    }

    void *clone_ByteClassPtr (void *rhs)
    {
        edmByteClass *src = (edmByteClass *) rhs;
        edmByteClass *obj = new edmByteClass(src);
        return (void *) obj;
    }
    
} // extern "C"




