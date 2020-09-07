#include <proto/exec.h>
#include <exec/resident.h>
#include <exec/errors.h>
#include <libraries/dos.h>

#if DEBUG
#include <clib/debug_protos.h>
#endif

#define STR(s) #s      /* Turn s into a string literal without expanding macro definitions (however, \
                          if invoked from a macro, macro arguments are expanded). */
#define XSTR(s) STR(s) /* Turn s into a string literal after macro-expanding it. */

#define DEVICE_NAME "simple.device"
#define DEVICE_DATE "(1 Sep 2020)"
#define DEVICE_ID_STRING "simple " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " " DEVICE_DATE /* format is: 'name version.revision (d.m.yy)' */
#define DEVICE_VERSION 1
#define DEVICE_REVISION 0
#define DEVICE_PRIORITY 0 /* Most people will not need a priority and should leave it at zero. */

struct ExecBase *SysBase;
BPTR saved_seg_list;

BOOL is_open;

/*-----------------------------------------------------------
A library or device with a romtag should start with moveq #-1,d0 (to
safely return an error if a user tries to execute the file), followed by a
Resident structure.
------------------------------------------------------------*/
int __attribute__((no_reorder)) _start()
{
    return -1;
}

/*----------------------------------------------------------- 
A romtag structure.  After your driver is brought in from disk, the
disk image will be scanned for this structure to discover magic constants
about you (such as where to start running you from...).

endcode is a marker that shows the end of your code. Make sure it does not
span hunks, and is not before the rom tag! It is ok to put it right after
the rom tag -- that way you are always safe.
Make sure your program has only a single code hunk if you put it at the 
end of your code.
------------------------------------------------------------*/
asm("romtag:                                \n"
    "       dc.w    "XSTR(RTC_MATCHWORD)"   \n"
    "       dc.l    romtag                  \n"
    "       dc.l    endcode                 \n"
    "       dc.b    "XSTR(RTF_AUTOINIT)"    \n"
    "       dc.b    "XSTR(DEVICE_VERSION)"  \n"
    "       dc.b    "XSTR(NT_DEVICE)"       \n"
    "       dc.b    "XSTR(DEVICE_PRIORITY)" \n"
    "       dc.l    _device_name            \n"
    "       dc.l    _device_id_string       \n"
    "       dc.l    _auto_init_tables       \n"
    "endcode:                               \n");

char device_name[] = DEVICE_NAME;
char device_id_string[] = DEVICE_ID_STRING;

/*------- init_device ---------------------------------------
FOR RTF_AUTOINIT:
  This routine gets called after the device has been allocated.
  The device pointer is in d0. The AmigaDOS segment list is in a0.
  If it returns the device pointer, then the device will be linked
  into the device list.  If it returns NULL, then the device
  will be unloaded.

IMPORTANT:
  If you don't use the "RTF_AUTOINIT" feature, there is an additional
  caveat. If you allocate memory in your Open function, remember that
  allocating memory can cause an Expunge... including an expunge of your
  device. This must not be fatal. The easy solution is don't add your
  device to the list until after it is ready for action.

CAUTION: 
This function runs in a forbidden state !!!                   
This call is single-threaded by Exec
------------------------------------------------------------*/
static struct Library __attribute__((used)) * init_device(BPTR seg_list asm("a0"), struct Library *dev asm("d0"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running init_device()\n");
#endif

    /* !!! required !!! save a pointer to exec */
    SysBase = *(struct ExecBase **)4UL;

    /* save pointer to our loaded code (the SegList) */
    saved_seg_list = seg_list;

    dev->lib_Node.ln_Type = NT_DEVICE;
    dev->lib_Node.ln_Name = device_name;
    dev->lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    dev->lib_Version = DEVICE_VERSION;
    dev->lib_Revision = DEVICE_REVISION;
    dev->lib_IdString = (APTR)device_id_string;

    is_open = FALSE;

    return dev;
}

/* device dependent expunge function 
!!! CAUTION: This function runs in a forbidden state !!! 
This call is guaranteed to be single-threaded; only one task 
will execute your Expunge at a time. */
static BPTR __attribute__((used)) expunge(struct Library *dev asm("a6"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running expunge()\n");
#endif

    if (dev->lib_OpenCnt != 0)
    {
        dev->lib_Flags |= LIBF_DELEXP;
        return 0;
    }

    //xyz_shutdown();

    BPTR seg_list = saved_seg_list;
    Remove(&dev->lib_Node);
    FreeMem((char *)dev - dev->lib_NegSize, dev->lib_NegSize + dev->lib_PosSize);
    return seg_list;
}

/* device dependent open function 
!!! CAUTION: This function runs in a forbidden state !!!
This call is guaranteed to be single-threaded; only one task 
will execute your Open at a time. */
static void __attribute__((used)) open(struct Library *dev asm("a6"), struct IORequest *ioreq asm("a1"), ULONG unitnum asm("d0"), ULONG flags asm("d1"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running open()\n");
#endif

    ioreq->io_Error = IOERR_OPENFAIL;
    ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;

    if (unitnum != 0)
        return;

    if (!is_open)
    {
        //initialize and open here

        //xyz_initialize();
        //if (xy_open() != 0)
        //	return;

        is_open = TRUE;
    }

    dev->lib_OpenCnt++;
    ioreq->io_Error = 0; //Success
}

/* device dependent close function 
!!! CAUTION: This function runs in a forbidden state !!!
This call is guaranteed to be single-threaded; only one task 
will execute your Close at a time. */
static BPTR __attribute__((used)) close(struct Library *dev asm("a6"), struct IORequest *ioreq asm("a1"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running close()\n");
#endif

    ioreq->io_Device = NULL;
    ioreq->io_Unit = NULL;

    dev->lib_OpenCnt--;

    if (dev->lib_OpenCnt == 0 && (dev->lib_Flags & LIBF_DELEXP))
        return expunge(dev);

    return 0;
}

/* device dependent beginio function */
static void __attribute__((used)) begin_io(struct Library *dev asm("a6"), struct IORequest *ioreq asm("a1"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running begin_io()\n");
#endif
}

/* device dependent abortio function */
static ULONG __attribute__((used)) abort_io(struct Library *dev asm("a6"), struct IORequest *ioreq asm("a1"))
{
#if DEBUG
    KPrintF((CONST_STRPTR) "running abort_io()\n");
#endif

    return IOERR_NOCMD;
}

static ULONG device_vectors[] =
    {
        (ULONG)open,
        (ULONG)close,
        (ULONG)expunge,
        0, //extFunc not used here
        (ULONG)begin_io,
        (ULONG)abort_io,
        -1}; //function table end marker

/*-----------------------------------------------------------
The romtag specified that we were "RTF_AUTOINIT".  This means
that the RT_INIT structure member points to one of these
tables below. If the AUTOINIT bit was not set then RT_INIT
would point to a routine to run. 

MyDev_Sizeof    data space size
device_vectors  pointer to function initializers
dataTable       pointer to data initializers
init_device     routine to run
------------------------------------------------------------*/
const ULONG auto_init_tables[4] =
    {
        sizeof(struct Library),
        (ULONG)device_vectors,
        0,
        (ULONG)init_device};