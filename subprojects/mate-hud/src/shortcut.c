#include "shortcut.h"
#include <gio/gio.h>
#include <glib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Longer than the helper's 2 s Activate call, so a helper that is mid-call
 * still gets to exit (and release its grabs) on its own. */
#define HELPER_EXIT_TIMEOUT_MS 3000

struct ShortcutGrab {
    GPid   child_pid;
    guint  child_watch_id;
    gboolean stopping;
    char  *accelerator;
};

static char *helper_path_for_current_binary(void)
{
    g_autofree char *self_path = g_file_read_link("/proc/self/exe", NULL);
    if (!self_path || !*self_path)
        return NULL;

    g_autofree char *dir = g_path_get_dirname(self_path);
    return g_build_filename(dir, "mate-hud-shortcut-helper", NULL);
}

static void shortcut_child_watch(GPid pid, gint status, gpointer user_data)
{
    ShortcutGrab *sg = user_data;

    if (sg->child_watch_id != 0) {
        g_source_remove(sg->child_watch_id);
        sg->child_watch_id = 0;
    }

    g_spawn_close_pid(pid);
    sg->child_pid = 0;

    if (sg->stopping)
        return;

    if (WIFSIGNALED(status)) {
        g_warning("mate-hud: shortcut helper exited via signal %d",
                  WTERMSIG(status));
    } else if (WIFEXITED(status)) {
        g_warning("mate-hud: shortcut helper exited with status %d",
                  WEXITSTATUS(status));
    } else {
        g_warning("mate-hud: shortcut helper exited unexpectedly");
    }
}

/* Reap the helper after SIGTERM. Waiting keeps the old grab from overlapping
 * the next helper's, but a helper that never exits must not freeze the daemon:
 * after the timeout it is killed, and the X server drops its grabs when the
 * connection closes. */
static void wait_for_helper_exit(GPid pid)
{
    int status = 0;
    gint64 deadline = g_get_monotonic_time() + HELPER_EXIT_TIMEOUT_MS * 1000;

    while (g_get_monotonic_time() < deadline) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid || (r < 0 && errno != EINTR))
            return;
        g_usleep(1000);
    }

    g_warning("mate-hud: shortcut helper (pid=%d) did not exit; killing it",
              (int)pid);
    kill(pid, SIGKILL);
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
        ;
}

ShortcutGrab *shortcut_new(Display *xdpy G_GNUC_UNUSED)
{
    return g_new0(ShortcutGrab, 1);
}

gboolean shortcut_grab(ShortcutGrab *sg, const char *accelerator,
                       const char *wm_class G_GNUC_UNUSED,
                       int history_max G_GNUC_UNUSED)
{
    g_autofree char *helper = NULL;
    gchar *argv[3] = { NULL, NULL, NULL };
    g_autoptr(GError) err = NULL;

    if (!sg) return FALSE;

    shortcut_ungrab(sg);

    if (!accelerator || !*accelerator)
        return FALSE;

    helper = helper_path_for_current_binary();
    if (!helper || !g_file_test(helper, G_FILE_TEST_IS_EXECUTABLE)) {
        g_warning("mate-hud: shortcut helper not found or not executable: %s",
                  helper ? helper : "(null)");
        return FALSE;
    }

    argv[0] = helper;
    argv[1] = (gchar *)accelerator;

    sg->stopping = FALSE;
    if (!g_spawn_async(NULL, argv, NULL,
                       G_SPAWN_DO_NOT_REAP_CHILD,
                       NULL, NULL, &sg->child_pid, &err)) {
        g_warning("mate-hud: failed to start shortcut helper for '%s': %s",
                  accelerator, err->message);
        sg->child_pid = 0;
        return FALSE;
    }

    sg->child_watch_id = g_child_watch_add(sg->child_pid, shortcut_child_watch, sg);
    g_free(sg->accelerator);
    sg->accelerator = g_strdup(accelerator);
    g_message("mate-hud: started shortcut helper for '%s' (pid=%d)",
              accelerator, (int)sg->child_pid);
    return TRUE;
}

void shortcut_ungrab(ShortcutGrab *sg)
{
    if (!sg) return;

    sg->stopping = TRUE;

    if (sg->child_pid != 0) {
        kill(sg->child_pid, SIGTERM);
    }

    if (sg->child_watch_id != 0) {
        g_source_remove(sg->child_watch_id);
        sg->child_watch_id = 0;
    }

    if (sg->child_pid != 0) {
        wait_for_helper_exit(sg->child_pid);
        g_spawn_close_pid(sg->child_pid);
        sg->child_pid = 0;
    }

    sg->stopping = FALSE;
}

void shortcut_free(ShortcutGrab *sg)
{
    if (!sg) return;
    shortcut_ungrab(sg);
    g_free(sg->accelerator);
    g_free(sg);
}
