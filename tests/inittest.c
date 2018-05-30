#include <SDL2/SDL.h>

#include <getopt.h>
#include <unistd.h>
#include <signal.h>

int cont_error = 0;
int twice = 0;
int no_init = 0;
int no_quit = 0;
int segv = 0;
int ignore_pipe = 0;
int handle_pipe = 0;
int handle_first = 0;
int wait = 0;

int* null_ptr = 0;

struct option long_options[] = {
    {"continue",       no_argument, 0, 'c'},
    {"twice",          no_argument, 0, 't'},
    {"no-init",        no_argument, 0, 'i'},
    {"no-quit",        no_argument, 0, 'q'},
    {"raise-segv",     no_argument, 0, 's'},
    {"ignore-sigpipe", no_argument, 0, 'p'},
    {"handle-sigpipe", no_argument, 0, 'a'},
    {"handler-first",  no_argument, 0, 'f'},
    {"wait",           no_argument, 0, 'w'},
    {"help",           no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

void check_error(int error)
{
    fprintf(stderr, "%s\n", error ? SDL_GetError(): "OK");
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
    fprintf(stderr, "SIGPIPE raised\n");
    return;
}

void set_handle_pipe()
{
    struct sigaction sa = { 0 };
    sa.sa_handler = &pipe_handler;
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
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
"  -s, --raise-segv      Raise fatal signal by NULL pointer access\n"
"  -p, --ignore-sigpipe  Set SIGPIPE handler to ignore\n"
"  -a, --handle-sigpipe  Install SIGPIPE handler showing message\n"
"  -f, --handler-first   Set SIGPIPE handler before SDL_Init\n"
"  -w, --wait            Add 20 seconds delay before writing to stdout\n",
        argv0);
}

int main(int argc, char** argv)
{
    int c;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "ctiqspafwh", long_options,
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

            case 'f': handle_first = 1; break;

            case 'w': wait = 1; break;

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
    }

    if (twice) {
        fprintf(stderr, "SDL_Init: ");
        if (!no_init)
            check_error(SDL_Init(SDL_INIT_VIDEO) != 0);
        else
            fprintf(stderr, "Skipped\n");

        fprintf(stderr, "SDL_Quit: ");
        if (!no_quit)
        {
            SDL_Quit();
            fprintf(stderr, "Done\n");
        }
        else
            fprintf(stderr, "Skipped\n");
    }

    fprintf(stderr, "SDL_Init: ");
    if (!no_init)
        check_error(SDL_Init(SDL_INIT_VIDEO) != 0);
    else
        fprintf(stderr, "Skipped\n");

    if (!handle_first) {
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

    printf("stdout rollo\n");
    fflush(stdout);

    fprintf(stderr, "SDL_Quit: ");
    if (!no_quit)
    {
        SDL_Quit();
        fprintf(stderr, "Done\n");
    }
    else
        fprintf(stderr, "Skipped\n");
}

