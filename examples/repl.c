/* The Lua console driving the UI: the slider, the label and the theme are all
 * shared with the script, in both directions.
 *
 *     count = count + 1
 *     set_label("hello from Lua")
 *     toast("3 times count is " .. count * 3)
 *     dark(true)
 *     for i = 1, 5 do count = count + 1 end
 */
#include <cui/cui.h>
#include <cui/cui_app.h>

#include <stdio.h>
#include <string.h>

typedef struct { cui_script *lua; cui_ctx *ui; bool dark; } rp_app;

static char label[256] = "Written from the app";

static int set_label_fn(void *ud, const char *const *args, int n, char *out, size_t cap) {
    (void)ud;
    if (n < 1) { snprintf(out, cap, "usage: set_label(text)"); return 1; }
    snprintf(label, sizeof label, "%s", args[0]);
    return 0;
}

static int toast_fn(void *ud, const char *const *args, int n, char *out, size_t cap) {
    (void)out; (void)cap;
    rp_app *a = ud;
    if (!a->ui || n < 1) return 0;
    cui_toast(a->ui, args[0]);
    return 0;
}

static int dark_fn(void *ud, const char *const *args, int n, char *out, size_t cap) {
    (void)out; (void)cap;
    rp_app *a = ud;
    a->dark = n >= 1 && args[0] && (!strcmp(args[0], "true") || !strcmp(args[0], "1"));
    if (a->ui) cui_set_theme(a->ui, a->dark ? CUI_THEME_DARK : CUI_THEME_LIGHT);
    return 0;
}

static void frame(cui_ctx *ui, void *user) {
    rp_app *a = user;
    a->ui = ui;
    /* same containers as the gallery's developer tools page: a scrolling
     * column, so every card keeps its box even when the window is short */
    cui_scroll(ui, .id = "main", .width = CUI_FILL, .height = CUI_FILL, .fade_edges = true) {
        cui_column(ui, .pad = 20, .gap = 12, .width = CUI_FILL) {
            cui_row(ui, .gap = 12, .align = CUI_ALIGN_CENTER) {
                cui_heading(ui, "Lua console", 3);
                cui_muted(ui, "the widgets above are shared with the script");
            }
            cui_row(ui, .gap = 16, .align = CUI_ALIGN_CENTER) {
                float cnt = (float)cui_script_get_number(a->lua, "count");
                if (cui_slider(ui, "count", &cnt, 0, 100, .show_value = false))
                    cui_script_set_number(a->lua, "count", cnt);
                cui_text(ui, label, .font = CUI_FONT_MEDIUM);
                if (cui_switch(ui, "Dark", &a->dark)) cui_set_theme(ui, a->dark ? CUI_THEME_DARK : CUI_THEME_LIGHT);
            }
            cui_script_widget(ui, "panel", a->lua, "panel");
            cui_repl(ui, "console", a->lua, .width = CUI_FILL, .height = 300, .autofocus = true);
        }
    }
}

int main(void) {
    static rp_app a;
    a.lua = cui_script_new();
    if (a.lua) {
        cui_script_set_fn(a.lua, "set_label", set_label_fn, NULL);
        cui_script_set_fn(a.lua, "toast", toast_fn, &a);
        cui_script_set_fn(a.lua, "dark", dark_fn, &a);
        cui_script_set_number(a.lua, "count", 0);
        cui_script_eval(a.lua,
            "function panel(ui, self)\n"
            "  ui.card({ gap = 8 }, function()\n"
            "    ui.label('a card drawn from Lua')\n"
            "    ui.text('count is shared with the slider above: ' .. count)\n"
            "    if ui.button('+1') then count = count + 1 end\n"
            "    self.who = ui.textfield('who', self.who or 'who to greet?')\n"
            "    if ui.button('greet', { variant = 'primary' }) then toast('hello, ' .. self.who) end\n"
            "  end)\n"
            "end");
        cui_script_printf(a.lua, "\x1b[2mtry: count = count + 1   set_label(\"hi\")   toast(\"hello\")   dark(true)\x1b[0m\n");
    }
    int r = cui_app_run(&(cui_app_desc){ .title = "Lua console", .width = 720, .height = 560 }, frame, &a);
    cui_script_free(a.lua);
    return r;
}
