#include "pv_factory.h"
#include "multiLineTextUpdate.h"
#include "environment.str"
#include "diamondWidgets.str"
#include "edm.version"

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
    { MULTILINE_TEXTUPDATE_CLASSNAME, global_str2, reg_str1 },
    { MULTILINE_TEXTENTRY_CLASSNAME, global_str5, reg_str2 }
};

extern "C"
{

char *version ( void ) {

    static char *v = VERSION;

      return v;

    }

    char *author ( void ) {

    static char *a = "Steve Singleton (steve.singleton@diamond.ac.uk)";

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

    void *create_multiLineTextUpdateClassPtr (void)
    {
        edmmultiLineTextUpdateClass *obj = new edmmultiLineTextUpdateClass;
        return (void *) obj;
    }

    void *clone_multiLineTextUpdateClassPtr (void *rhs)
    {
        edmmultiLineTextUpdateClass *src = (edmmultiLineTextUpdateClass *) rhs;
        edmmultiLineTextUpdateClass *obj = new edmmultiLineTextUpdateClass(src);
        return (void *) obj;
    }

    void *create_multiLineTextEntryClassPtr (void)
    {
        edmmultiLineTextEntryClass *obj = new edmmultiLineTextEntryClass;
        return (void *) obj;
    }
    
    void *clone_multiLineTextEntryClassPtr (void *rhs)
    {
        edmmultiLineTextEntryClass *src = (edmmultiLineTextEntryClass *) rhs;
        edmmultiLineTextEntryClass *obj = new edmmultiLineTextEntryClass(src);
        return (void *) obj;
    }

} // extern "C"
