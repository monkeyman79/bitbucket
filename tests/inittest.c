#include <SDL2/SDL.h>

#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <dlfcn.h>

int cont_error = 0;
int twice = 0;
int no_init = 0;
int no_quit = 0;
int segv = 0;
int ignore_pipe = 0;
int handle_pipe = 0;
int handle_segv = 0;
int handle_first = 0;
int unloaded = 0;
int wait = 0;

int* null_ptr = 0;
int (*pSDL_Init)(uint32_t flags);
void (*pSDL_Quit)(void);
const char* (*pSDL_GetError)(void);

#ifdef DYNAMIC_LOAD
void *libsdl2_handle;
#endif

struct option long_options[] = {
    {"continue",       no_argument, 0, 'c'},
    {"twice",          no_argument, 0, 't'},
    {"no-init",        no_argument, 0, 'i'},
    {"no-quit",        no_argument, 0, 'q'},
    {"raise-sigsegv",  no_argument, 0, 's'},
    {"ignore-sigpipe", no_argument, 0, 'p'},
    {"handle-sigpipe", no_argument, 0, 'a'},
    {"handle-sigsegv", no_argument, 0, 'e'},
    {"handler-first",  no_argument, 0, 'f'},
    {"wait",           no_argument, 0, 'w'},
    {"unloaded",       no_argument, 0, 'u'},
    {"help",           no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

void check_error(int error, const char* (*geterror)(void))
{
    fprintf(stderr, "%s\n", error ? geterror(): "OK");
    if (error) {
        if (!cont_error) {
            fprintf(stderr, "...exiting\n");
            exit(1);
        }
        fprintf(stderr, "...error ignored\n");
    }
}

void set_ignore_pipe()
{
    signal(SIGPIPE, SIG_IGN);
}

void pipe_handler(int sig)
{
    fprintf(stderr, "inittest SIGPIPE handler executing\n");
    return;
}

void segv_handler(int sig)
{
    sigset_t sigset;
    fprintf(stderr, "inittest SIGSEGV handler executing\n");
    fprintf(stderr, "inittest SIGSEGV handler re-raising\n");
    raise(SIGSEGV);
    return;
}

void set_handle_pipe()
{
    struct sigaction sa = { 0 };
    sa.sa_handler = &pipe_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
}

void set_handle_segv()
{
    struct sigaction sa = { 0 };
    sa.sa_handler = &segv_handler;
    sa.sa_flags = SA_NODEFER | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
}

void usage(const char* argv0)
{
    fprintf(stderr, "Usage: %s [FLAGS}\n"
"SDL initialization and cleanup test.\n\n"
"Flags:\n"
"  -c, --continue        Continue afert SDL_Init error\n"
"  -t, --twice           Call SDL_Init and SDL_Quit twice\n"
"  -i, --no-init         Skip SDL_Init\n"
"  -q, --no-quit         Skip SDL_Quit\n"
"  -p, --ignore-sigpipe  Set SIGPIPE handler to ignore\n"
"  -a, --handle-sigpipe  Install SIGPIPE handler\n"
"  -e, --handle-sigsegv  Install SIGSEGV handler\n"
"  -f, --handler-first   Install signal handlers before SDL_Init\n"
"  -s, --raise-sigsegv   Raise fatal signal by NULL pointer access\n"
"  -w, --wait            Add 20 seconds delay before signal or writing to stdout\n"
"  -u, --unloaded        Raise signal or print message after SDL_Quit\n",
        argv0);
}

#ifndef DYNAMIC_LOAD
#define cSDL_Init SDL_Init
#define cSDL_Quit SDL_Quit
#define cSDL_GetError SDL_GetError
#else
#define cSDL_Init pSDL_Init
#define cSDL_Quit pSDL_Quit
#define cSDL_GetError pSDL_GetError
#endif

#define _dlerror ((const char* (*)(void))dlerror)

int init_sdl()
{
#ifdef DYNAMIC_LOAD
    fprintf(stderr, "dlopen(libSDL2.so): ");
    // libsdl2_handle = dlopen("libSDL2.so", RTLD_LAZY);
    libsdl2_handle = dlopen("/usr/local/lib/libSDL2.so", RTLD_LAZY);
    check_error(libsdl2_handle == NULL, _dlerror);

    fprintf(stderr, "dlsym(SDL_Init): ");
    pSDL_Init = (int (*)(uint32_t)) dlsym(libsdl2_handle, "SDL_Init");
    check_error(pSDL_Init == NULL, _dlerror);

    fprintf(stderr, "dlsym(SDL_Quit): ");
    pSDL_Quit = (void (*)(void)) dlsym(libsdl2_handle, "SDL_Quit");
    check_error(pSDL_Quit == NULL, _dlerror);

    fprintf(stderr, "dlsym(SDL_GetError): ");
    pSDL_GetError = (const char* (*)(void)) dlsym(libsdl2_handle, "SDL_GetError");
    check_error(pSDL_GetError == NULL, _dlerror);
#endif

    fprintf(stderr, "SDL_Init: ");
    if (!no_init)
        check_error(cSDL_Init(SDL_INIT_VIDEO) != 0, cSDL_GetError);
    else
        fprintf(stderr, "Skipped\n");
}

int quit_sdl()
{
    fprintf(stderr, "SDL_Quit: ");
    if (!no_quit)
    {
        cSDL_Quit();
        fprintf(stderr, "Done\n");
    }
    else
        fprintf(stderr, "Skipped\n");

#ifdef DYNAMIC_LOAD
    fprintf(stderr, "dlclose(libSDL2.so): ");
    check_error(dlclose(libsdl2_handle) != 0, _dlerror);
#endif
}

void install_sig_handlers()
{
    if (ignore_pipe) {
        fprintf(stderr, "Ignore SIGPIPE: ");
        set_ignore_pipe();
        fprintf(stderr, "Done\n");
    }
    else if (handle_pipe) {
        fprintf(stderr, "Set SIGPIPE handler: ");
        set_handle_pipe();
        fprintf(stderr, "Done\n");
    }
    if (handle_segv) {
        fprintf(stderr, "Set SIGSEGV handler: ");
        set_handle_segv();
        fprintf(stderr, "Done\n");
    }
}

int main(int argc, char** argv)
{
    int c;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "ctiqspaefwuh", long_options,
                &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'c': cont_error = 1; break;

            case 't': twice = 1; break;

            case 'i': no_init = 1; break;

            case 'q': no_quit = 1; break;

            case 's': segv = 1; break;

            case 'p': ignore_pipe = 1; break;

            case 'a': handle_pipe = 1; break;

            case 'e': handle_segv = 1; break;

            case 'f': handle_first = 1; break;

            case 'w': wait = 1; break;

            case 'u': unloaded = 1; break;

            case '?': return 1;

            case 'h': usage(argv[0]); return 0;

            default:
                abort();
        }
    }

    if (optind < argc) {
        fprintf(stderr, "%s: unexpected argument\n", argv[0]);
        return 1;
    }

    if (ignore_pipe && handle_pipe) {
        fprintf(stderr,
            "%s: --ignore-pipe and --handle-pipe are mutually exclusive\n");
        return 1;
    }

    if (handle_first) {
        install_sig_handlers();
    }

    if (twice) {
        init_sdl();

        quit_sdl();
    }

    init_sdl();

    if (!handle_first) {
        install_sig_handlers();
    }

    if (unloaded) {
        quit_sdl();
    }

    if (segv) {
        fprintf(stderr, "Null pointer access: ");
        *null_ptr = 0;
        fprintf(stderr, "Passed... hmmm\n");
    }

    if (wait) {
        fprintf(stderr, "Delay 20 seconds: ");
        sleep(20);
        fprintf(stderr, "Done\n");
    }

    printf("this goes to stdout\n");
    fflush(stdout);

    if (!unloaded) {
        quit_sdl();
    }

    fprintf(stderr, "Bye!\n");
}

/* vi: set ts=4 sw=4 expandtab: */
