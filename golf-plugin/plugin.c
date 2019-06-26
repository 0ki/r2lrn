/* radare - Apache - Copyright 2019 - pancake */

#include <r_core.h>
#include "r2golf.c"

static Golf *g = NULL;
static int inception = 0;

static void _golf_stat_update(RCore *core, const char *cmd) {
	if (!strncmp (cmd, "golf", 4)) {
		return;
	}
	if (inception == 0 && g) {
		if (strchr (cmd, '?')) {
			g->stat_helps++;
		} else {
			g->stat_cmds++;
		}
	}
}

static int _golf_ranking(RCore *core) {
	int res = 10;
	res -= (g->stat_cmds / 8);
	res -= (g->stat_helps / 4);
	res -= (g->stat_hint);
	return (res > 0)? res: 0;
}

static void _golf_stat(RCore *core) {
	if (!g) {
		eprintf ("Error: use `golf open` to select a new lab.\n");
		return;
	}
	int ranking = _golf_ranking (core);
	eprintf ("Ranking Points: %d/10\n", ranking);
	eprintf ("Commands Executed: %d\n", g->stat_cmds);
	eprintf ("Ask Helps: %d\n", g->stat_helps);
	eprintf ("Checked Hints: %d\n", g->stat_hint);
}

static char *_select_golf(bool interactive) {
	char *res = NULL;
	RList *golfs = golf_enumerate ("./t");
	RListIter *iter;
	char *t;
	int n = 1;
	r_list_foreach (golfs, iter, t) {
		r_cons_printf ("%d  %s\n", n++, t);
	}
	r_cons_flush ();
	if (interactive) {
		while (true) {
			char *s = r_cons_input ("> ");
			n = atoi (s);
			if (!strcmp (s, "q")) {
				break;
			}
			if (n > 0) {
				n--;
				char *item = r_list_get_n (golfs, n);
				if (item) {
					res = strdup (item);
					free (s);
					break;
				}
			}
			free (s);
		}
	}
	r_list_free (golfs);
	return res;
}

static void golf_start(const char *t) {
	if (g) {
		golf_free (g);
	}
	g = golf_new ("./");
	if (!g) {
		eprintf ("Invalid golf\n");
	}
	
	golf_free (g);
}

static bool golf_test(RCore *core) {
	bool res = false;
	if (g) {
		if (g->done) {
			return true;
		}
		if (_golf_ranking (core) < 1) {
			r_core_cmd0 (core, "?E You failed the mission!");
			r_cons_flush ();
			g->done = true;
			res = true;
		} else {
			RListIter *iter;
			char *s = r_core_cmd_str (core, g->verify);
			s = r_str_trim (s);
			char *e = r_str_trim (g->expect);
			// eprintf ("COMPARE (%s)(%s)\n", s, e);
			if (!strcmp (s, e)) {
				r_core_cmd0 (core, "?E Congratulations you solved the exercise!");
				g->done = true;
				r_cons_printf ("\n Type 'golf open' to run another lab.\n");
				r_cons_flush ();
				res = true;
			}
			free (s);
		}
		
	}
	return res;
}

static bool _start(RCore *core, const char *fn) {
	if (fn && *fn) {
		r_core_cmdf (core, "\"o %s\"", fn);
	}
	return true;
}

static bool _reset(RCore *core) {
	r_core_cmd0 (core, "o-*");
	r_core_cmd0 (core, "f-*");
	r_core_cmd0 (core, "s 0");
	r_core_cmd0 (core, "$hint=golf hint");
	r_core_cmd0 (core, "$stat=golf stat");
	r_core_cmd0 (core, "e cmd.prompt=golf test");
	r_core_cmd0 (core, "e scr.color=2");
	return true;
}

static void _golf_hint(RCore *core) {
	if (!core || !g) {
		return;
	}
	int nth = g->stat_hint % r_list_length (g->hints);
	char *n = r_list_get_n (g->hints, nth);
	if (!n) {
		return;
	}
	g->stat_hint++;
	char *s = strdup (n);
	r_core_cmdf (core, "\"?E %s\"", s);
	free (s);
	r_cons_flush ();
}

static bool _golf_open(RCore *core) {
	r_core_cmd0 (core, "?E Welcome to the r2 tutorial engine!");
	r_cons_flush ();
	r_cons_any_key (NULL);

	r_cons_clear00 ();
	r_core_cmd0 (core, "?E Pick one of the following exercises..");
	r_cons_flush ();

	char *golf = _select_golf (true);
	if (golf) {
		r_core_cmdf (core, "?E You have selected %s", golf);
		r_cons_flush ();
		g = golf_new (golf);
		if (g) {
			_reset (core);
			_start (core, g->input);
			r_core_cmdf (core, "?E %s", g->name);
			r_cons_flush ();
			RListIter *iter;
			char *d;
			r_list_foreach (g->descriptions, iter, d) {
				r_cons_clear00 ();
				r_core_cmdf (core, "?E %s", d);
				r_cons_flush ();
				r_cons_any_key (NULL);
			}
		} else {
			eprintf ("Cannot open '%s'\n", golf);
		}
		free (golf);
	}
	return true;
}

static const char *golfHelpMessage = \
	"Usage: golf [action]\n"
	" golf open     start a new lab\n"
	" golf hint     give me a hint\n"
	" golf test     verify if it has been solved\n"
	" golf stat     check status of current exercise\n"
	" golf kill     close the current session\n";

static void _golf_help (RCore *core) {
	r_cons_printf ("%s\n", golfHelpMessage);
}

static bool isRootCommand(RCore *core, const char *input) {
	int maxDepth = r_config_get_i (core->config, "cmd.depth");
	return (maxDepth - 1 == core->cmd_depth);
}

static int r_cmd_golf_call(void *user, const char *input) {
        RCore *core = (RCore *) user;
	if (!isRootCommand (core, input)) {
		return false;
	}
	inception++;
	if (!strncmp (input, "golf", 4)) {
		if (input[4] == ' ') {
			const char *cmd = input + 5;
			if (!strncmp (cmd, "open", 4)) {
				_golf_open (core);
				inception--;
				return true;
			}
			if (!strncmp (cmd, "hint", 4)) {
				_golf_hint (core);
				inception--;
				return true;
			}
			if (!strncmp (cmd, "test", 4)) {
				golf_test (core);
				inception--;
				return true;
			}
			if (!strncmp (cmd, "kill", 4)) {
				golf_free (g);
				g = NULL;
				return true;
			}
			if (!strncmp (cmd, "list", 4)) {
				free (_select_golf (false));
				inception--;
				return true;
			}
			if (!strncmp (cmd, "stat", 4)) {
				_golf_stat (core);
				inception--;
				return true;
			}
		}
		_golf_help (core);	
		inception--;
		return true;
	}
	if (*input != 'q' && g && g->done) {
		eprintf ("GAME OVER.\n");
		eprintf ("- Run 'golf open' to start a new game.\n");
		eprintf ("- Or 'golf kill' to leave the party.\n");
		return true;
	}
	inception--;
	_golf_stat_update (core, input);
	return false;
}

RCorePlugin r_core_plugin_golf = {
        .name = "golf",
        .desc = "plugin to learn about r2",
        .license = "Apache",
	.author = "pancake",
        .call = r_cmd_golf_call,
};

#ifndef CORELIB
R_API RLibStruct radare_plugin = {
        .type = R_LIB_TYPE_CORE,
        .data = &r_core_plugin_golf,
        .version = R2_VERSION
};
#endif
