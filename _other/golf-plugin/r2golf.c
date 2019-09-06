#include "md.c"

#define D if(0)

typedef enum {
	GOLF_VM_DESCRIPTION,
	GOLF_VM_VERIFY,
	GOLF_VM_INPUT,
	GOLF_VM_EXPECT,
	GOLF_VM_HINT,
} GOLF_VM;

typedef struct golf_t {
	char *name;
	RList *descriptions;
	RList *hints;
	char *input;
	char *verify;
	char *expect;
	// VM
	GOLF_VM mode;
	int stat_cmds;
	int stat_helps;
	int stat_hint;
	bool done;
} Golf;

static void parse_quote(Golf *g, const char *text) {
	switch (g->mode) {
	case GOLF_VM_VERIFY:
		g->verify = r_str_appendf (g->verify, "%s\n", strdup (text));
		break;
	case GOLF_VM_EXPECT:
		g->expect = r_str_appendf (g->expect, "%s\n", strdup (text));
		break;
	default:
		break;
	}
}
static void parse_text(Golf *g, const char *text) {
	switch (g->mode) {
	case GOLF_VM_DESCRIPTION:
		r_list_append (g->descriptions, strdup (text));
		break;
	default:
		break;
	}
}

static void parse_item(Golf *g, const char *text) {
	switch (g->mode) {
	case GOLF_VM_INPUT:
		free (g->input);
		g->input = strdup (text);
		break;
	case GOLF_VM_HINT:
		r_list_append (g->hints, strdup (text));
		break;
	default:
		break;
	}
}

static void parse_head1(Golf *g, const char *text) {
	free (g->name);
	g->name = strdup (text);
}

static void parse_head2(Golf *g, const char *text) {
	if (!strcmp (text, "Description")) {
		g->mode = GOLF_VM_DESCRIPTION;
	} else if (!strcmp (text, "Verify")) {
		g->mode = GOLF_VM_VERIFY;
	} else if (!strcmp (text, "Expect")) {
		g->mode = GOLF_VM_EXPECT;
	} else if (!strcmp (text, "Input")) {
		g->mode = GOLF_VM_INPUT;
	} else if (!strcmp (text, "Hints")) {
		g->mode = GOLF_VM_HINT;
	}
}

static bool golf_parse(Golf *g, const char *file) {
	char *data = r_file_slurp (file, NULL);
	if (data) {
		MD_TYPE t,tt;
		const char *e, *ee, *p = data;
		while (true) {
			const char *n = md_next (p, &t, &e);
			if (!n) {
				break;
			}
			if (n > e) {
				break;
			}
			if (n == e) {
				p++;
				continue;
			}
			char *str = r_str_ndup (n, e -n);
			switch (t) {
			case MD_TYPE_HEAD1:
				parse_head1 (g, str);
				D eprintf ("HEAD1 (%s)\n", str);
				break;
			case MD_TYPE_HEAD2:
				parse_head2 (g, str);
				D eprintf ("HEAD2 (%s)\n", str);
				break;
			case MD_TYPE_TEXT:
				parse_text (g, str);
				eprintf ("TEXT (%s)\n", str);
				break;
			case MD_TYPE_QUOTE:
				parse_quote (g, str);
				D eprintf ("QUOTE (%s)\n", str);
				break;
			case MD_TYPE_ITEM:
				parse_item (g, str);
				D eprintf ("ITEM (%s)\n", str);
				break;
			case MD_TYPE_UNKNOWN:
				D eprintf ("ERROR\n");
				break;
			}
			free (str);
			p = e + 1;
		}
		free (data);
		return true;
	}
	return false;
}

void golf_free (Golf *g) {
	r_list_free (g->hints);
	r_list_free (g->descriptions);
	free (g->name);
	free (g);
}

Golf *golf_new(const char *file) {
	Golf *g = R_NEW0 (Golf);
	g->hints = r_list_newf (free);
	g->descriptions = r_list_newf (free);
	char *fn = r_str_newf ("t/%s.r2golf", file);
	if (!golf_parse (g, fn)) {
		eprintf ("Cannot open %s\n", fn);
		golf_free (g);
		g = NULL;
	}
	free (fn);
	return g;
}

RList *golf_enumerate(const char *path) {
	RList * files = r_sys_dir (path);
	RList * res = r_list_newf (free);
	const char *t;
	RListIter *iter;
	r_list_foreach (files, iter, t) {
		if (r_str_endswith (t, ".r2golf")) {
			int len = strlen (t) - strlen (".r2golf");
			r_list_append (res, r_str_ndup (t, len));
		}
	}
	r_list_sort (res, (RListComparator)strcmp);
	r_list_free (files);
	return res;
}
