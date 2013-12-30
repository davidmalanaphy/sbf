#include "sbfCommon.h"
#include "sbfMw.h"
#include "sbfPerfCounter.h"
#include "sbfTport.h"

static u_int    gMessages;
static uint64_t gTimeLow = UINT64_MAX;
static uint64_t gTimeTotal;
static uint64_t gTimeHigh;

static void*
dispatchCb (void* closure)
{
    sbfQueue_dispatch (closure);
    return NULL;
}

static void
timerCb (sbfTimer timer, void* closure)
{
    printf ("%u low=%llu average=%llu high=%llu" SBF_EOL,
            gMessages,
            gTimeLow == UINT64_MAX ? 0 : (unsigned long long)gTimeLow,
            gMessages == 0 ? 0 : (unsigned long long)gTimeTotal / gMessages,
            (unsigned long long)gTimeHigh);
    fflush (stdout);

    gMessages = 0;
    gTimeLow = UINT64_MAX;
    gTimeTotal = 0;
    gTimeHigh = 0;
}

static void
messageCb (sbfSub sub, sbfBuffer buffer, void* closure)
{
    uint64_t* payload = sbfBuffer_getData (buffer);
    uint64_t  now;
    uint64_t  this = *payload;
    double    interval;

    now = sbfPerfCounter_get ();
    if (now < this)
        interval = 0;
    else
        interval = sbfPerfCounter_microseconds (now - this);

    gMessages++;
    gTimeTotal += (uint64_t)interval;
    if (interval < gTimeLow)
        gTimeLow = (uint64_t)interval;
    if (interval > gTimeHigh)
        gTimeHigh = (uint64_t)interval;
}

static void
usage (const char* argv0)
{
    fprintf (stderr,
             "usage: %s "
             "[-h handler] "
             "[-i interface] "
             "[-t topic] "
             "[-v level]\n",
             argv0);

    exit (1);
}

int
main (int argc, char** argv)
{
    const char* argv0 = argv[0];
    sbfMw       mw;
    sbfQueue    queue;
    sbfThread   t;
    sbfTport    tport;
    sbfKeyValue properties;
    int         opt;
    const char* topic = "OUT";
    const char* handler = "udp";
    const char* interf = "eth0";

    sbfLog_setLevel (SBF_LOG_OFF);

    while ((opt = getopt (argc, argv, "h:i:t:v:")) != -1) {
        switch (opt) {
        case 'h':
            handler = optarg;
            break;
        case 'i':
            interf = optarg;
            break;
        case 't':
            topic = optarg;
            break;
        case 'v':
            sbfLog_setLevel (sbfLog_getLevel (optarg));
            break;
        default:
            usage (argv0);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 0)
        usage (argv0);

    mw = sbfMw_create (1);

    queue = sbfQueue_create (0);
    sbfThread_create (&t, dispatchCb, queue);

    properties = sbfKeyValue_create ();
    sbfKeyValue_put (properties, "interface", interf);

    sbfKeyValue_put (properties, "listen", "0");
    sbfKeyValue_put (properties, "connect0", "127.0.0.1");

    tport = sbfTport_create (mw, SBF_MW_ALL_THREADS, handler, properties);

    sbfTimer_create (sbfMw_getDefaultThread (mw),
                     queue,
                     timerCb,
                     NULL,
                     1);

    sbfSub_create (tport, queue, topic, NULL, messageCb, NULL);

    for (;;)
        sleep (60);

    exit (0);
}
