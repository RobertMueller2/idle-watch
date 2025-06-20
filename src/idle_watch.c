#include "ext-idle-notify-client-protocol.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <time.h>
#include <wayland-client.h>

static struct wl_display* display;
static struct wl_registry* registry;
static struct wl_seat* seat;
static struct ext_idle_notifier_v1* idle_notifier;
static struct ext_idle_notification_v1* idle_notification;

static bool print_timestamp = false;
static char* idle_str = "[Idle] Seat has gone idle";
static char* resume_str = "[Resume] Activity resumed";
static char* initial_output;
static uint32_t timeout = 1000;

static void cleanup()
{
    if (idle_notification) {
        ext_idle_notification_v1_destroy(idle_notification);
    }
    if (idle_notifier) {
        ext_idle_notifier_v1_destroy(idle_notifier);
    }
    if (seat) {
        wl_seat_destroy(seat);
    }
    if (registry) {
        wl_registry_destroy(registry);
    }
    if (display) {
        wl_display_disconnect(display);
    }
}

static void print_with_timestamp(const char* msg)
{
    if (print_timestamp) {
        time_t now = time(NULL);
        struct tm tm_now;
        char buf[20];

        localtime_r(&now, &tm_now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_now);
        printf("[%s] %s\n", buf, msg);
    } else {
        printf("%s\n", msg);
    }
}

static void do_exit(int)
{
    cleanup();
    exit(0);
}

static void handle_idled(void* data __attribute_maybe_unused__,
    struct ext_idle_notification_v1* notification __attribute_maybe_unused__)
{
    print_with_timestamp(idle_str);
}

static void handle_resumed(void* data __attribute_maybe_unused__,
    struct ext_idle_notification_v1* notification __attribute_maybe_unused__)
{
    print_with_timestamp(resume_str);
}

static const struct ext_idle_notification_v1_listener idle_notification_listener = {
    .idled = handle_idled,
    .resumed = handle_resumed,
};

static void global_registry_handler(void* data __attribute_maybe_unused__,
    struct wl_registry* reg, uint32_t name, const char* interface,
    uint32_t version __attribute_maybe_unused__)
{
    if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(reg, name, &wl_seat_interface, 1);
    } else if (strcmp(interface, "ext_idle_notifier_v1") == 0) {
        idle_notifier
            = wl_registry_bind(reg, name, &ext_idle_notifier_v1_interface, 1);
    }
}

static const struct wl_registry_listener registry_listener
        = { .global = global_registry_handler,
              .global_remove = NULL };

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--timestamp") == 0) {
            print_timestamp = true;
        } else if (strcmp(argv[i], "--idle") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing string for --idle\n");
                return 1;
            }
            idle_str = argv[++i];
        } else if (strcmp(argv[i], "--initial-output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing string for --initial-output\n");
                return 1;
            }
            initial_output = argv[++i];
        } else if (strcmp(argv[i], "--resume") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing string for --resume\n");
                return 1;
            }
            resume_str = argv[++i];
        } else if (strcmp(argv[i], "--timeout") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --timeout\n");
                return 1;
            }
            timeout = (uint32_t)atoi(argv[++i]);
        } else {
            fprintf(stderr,
                "Usage: %s [--timestamp] [--idle <string>] [--resume <string>] "
                " [--timeout <ms>] [--initial-output <string>]\n",
                argv[0]);
            if (strcmp(argv[i], "--help") == 0) {
                return 0;
            }
            return 1;
        }
    }

    int exitcode = 0;
    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        exitcode = 2;
        goto cleanup;
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_roundtrip(display);

    if (!seat) {
        fprintf(stderr, "No wl_seat found\n");
        exitcode = 3;
        goto cleanup;
    }

    if (!idle_notifier) {
        fprintf(stderr, "ext_idle_notifier_v1 not available\n");
        exitcode = 4;
        goto cleanup;
    }

    idle_notification = ext_idle_notifier_v1_get_idle_notification(
        idle_notifier, timeout, seat);
    ext_idle_notification_v1_add_listener(
        idle_notification, &idle_notification_listener, NULL);

    signal(SIGTERM, do_exit);
    signal(SIGINT, do_exit);

    setbuf(stdout, NULL);
    if (initial_output) {
        print_with_timestamp(initial_output);
    }

    while (wl_display_dispatch(display));

cleanup:
    cleanup();

    return exitcode;
}
