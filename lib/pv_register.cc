#include<epics_pv_factory.h>
#include<textupdate.h>
#include<strip.h>

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

static libRecType libRec[] =
{
#if 0
    { "EPICS_PV_Factory", "PV_Factory", "EPICS" },
#endif
    { TEXTUPDATE_CLASSNAME, "Monitors", "Textupdate" },
    { TEXTENTRY_CLASSNAME, "Controls", "Textentry" },
    { STRIP_CLASSNAME, "Monitors", "Stripchart" }
};

extern "C"
{
    int nextRegRecord(char **className, char **typeName, char **text)
    {
        int max = sizeof(libRec) / sizeof(libRecType) - 1;
        if (libRecIndex >= max) return -1; //no more 
        libRecIndex++;
        *className = libRec[libRecIndex].className;
        *typeName = libRec[libRecIndex].typeName;
        *text = libRec[libRecIndex].text;
        return 0;
    }

    int firstRegRecord(char **className, char **typeName, char **text)
    {
        libRecIndex = -1;
        return nextRegRecord(className, typeName, text);
    }

    void *create_EPICS_PV_FactoryClassPtr (void)
    {
        EPICS_PV_Factory *obj = new EPICS_PV_Factory();
        return (void *) obj;
    }
    
    void *clone_EPICS_PV_FactoryClassPtr (void *rhs)
    {
        EPICS_PV_Factory *obj = new EPICS_PV_Factory();
        return (void *) obj;
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

    void *create_StripClassPtr (void)
    {
        edmStripClass *obj = new edmStripClass;
        return (void *) obj;
    }
    
    void *clone_StripClassPtr (void *rhs)
    {
        edmStripClass *src = (edmStripClass *) rhs;
        edmStripClass *obj = new edmStripClass(src);
        return (void *) obj;
    }
    
} // extern "C"




