typedef enum md_type_e {
	MD_TYPE_UNKNOWN,
	MD_TYPE_HEAD1,
	MD_TYPE_HEAD2,
	MD_TYPE_ITEM,
	MD_TYPE_QUOTE,
	MD_TYPE_TEXT,
} MD_TYPE;

const char *md_next(const char *data, MD_TYPE *type, const char **end) {
	MD_TYPE ttype;
	const char *tend;
	if (!type) {
		type = &ttype;
	}
	if (!end) {
		end = &tend;
	}
	bool nl = true;
	*type = MD_TYPE_UNKNOWN;
	if (!strncmp (data, "==", 2) || !strncmp (data, "--", 2)) {
		data = strchr (data, '\n');
		if (data) {
			data++;
		}
	}
	const char *ol = data;
	const char *pl = data;
	int twice = 0;
	const char *oend = NULL;
	const char *odata = data;
	while (*data) {
		char *nextline = strchr (data, '\n');
		if (!nextline) {
			break;
		}
		*type = MD_TYPE_UNKNOWN;
		const bool isText = isalpha (*data);
		const bool isHead1 = nextline[1] == '=';
		const bool isHead2 = nextline[1] == '-';
		if (*data == '\n') {
			if (!nl) {
				ol = pl;
				pl = data + 1;
			}
			nl = true;
			if (isalpha (data[1])) {
				twice++;
			}
		} else {
			if (isHead1) {
				*type = MD_TYPE_HEAD1;
				*end = strchr (ol, '\n');
				*end = strchr (nextline, '\n');
				return ol;
			}
			if (isHead2) {
				*type = MD_TYPE_HEAD2;
				*end = strchr (ol, '\n');
				*end = strchr (nextline, '\n');
				return ol + 1;
			}
			if (nl) {
				if (!strncmp (data, "====", 4)) {
					*type = MD_TYPE_HEAD1;
					*end = strchr (ol, '\n');
					return ol;
				}
				if (!strncmp (data, "----", 4)) {
					*end = strchr (ol, '\n');
					if (*end != ol) {
						*type = MD_TYPE_HEAD2;
						return ol;
					}
					data = *end;
				}
				if (!strncmp (data, "* ", 2)) {
					*type = MD_TYPE_ITEM;
					*end = strchr (data, '\n');
					return data + 2;
				}
				if (!strncmp (data, "\t", 1)) {
					*type = MD_TYPE_QUOTE;
					*end = strchr (data, '\n');
					return data + 1;
				}
				if (!isHead1 && !isHead2 && isText) {
					*end = nextline;
					*type = MD_TYPE_TEXT;
					return odata + 1;
				}
			} else {
				// wen return text?
			}
			nl = false;
		}
		data++;
	}
	return NULL;
}
