#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "easyini.h"

#define MAX_INI_LINE_LENGTH 512

#define STATE_NORMAL 0
#define STATE_ESCAPE 1

static char *get_until(const char *source, const char **next_source, const char *delims);
static void parse_ini_line(const char *line, easyini_parse_state_t *state);

static char *get_until(const char *source, const char **next_source, const char *delims) {
	char *value;
	char *orig;
	int state = STATE_NORMAL;
	int len = strlen(source);
	char *spacestart = NULL;

	value = (char *)malloc(len+1);
	orig = value;
	while (*source) {
		switch (state) {
			case STATE_NORMAL:
				switch (*source) {
					case '\\':
						spacestart = NULL;
						state = STATE_ESCAPE;
						break;
					default:
						if (strchr(delims, *source)) goto done;
						if (isspace(*source)) {
							if (spacestart == NULL) spacestart = value;
						}
						else spacestart = NULL;
						*value++ = *source;
				}
				break;
			case STATE_ESCAPE:
				spacestart = NULL;
				switch (*source) {
					case 'n':
						*value++ = '\n';
						state = STATE_NORMAL;
						break;
					case 't':
						*value++ = '\t';
						state = STATE_NORMAL;
						break;
					default:
						*value++ = *source;
						state = STATE_NORMAL;
				}
				break;
		}
		source++;
	}
done:

	if (next_source != NULL) *next_source = source;
	if (spacestart != NULL) *spacestart = 0;
	*value = 0;
	return orig;
}

easyini_config_t *easyini_parse_filename(const char *filename) {
	FILE *fp = fopen(filename, "r");
	easyini_config_t *config = easyini_parse_file(fp);
	fclose(fp);
	return config;
}

easyini_config_t *easyini_parse_file(FILE *fp) {
	char line[MAX_INI_LINE_LENGTH+1];
	if (fp == NULL) return NULL;

	easyini_parse_state_t state;
	state.config = (easyini_config_t *)calloc(1, sizeof(easyini_config_t));
	state.section = &state.config->global_section;

	while (fgets(line, MAX_INI_LINE_LENGTH, fp)) {
		line[MAX_INI_LINE_LENGTH] = 0;
		parse_ini_line(line, &state);
	}

	return state.config;
}

static void parse_ini_line(const char *line, easyini_parse_state_t *state) {
	const char *delim;
	int nspaces;

	if (line == NULL || *line == 0) return;

	while (isspace(*line)) line++;

	if (*line == 0) return;

	switch (*line) {
		case ';':
		case '#':
			return;
		case '[':
			/* New section */
			line++;
			while (isspace(*line)) line++;
			state->config->nsections++;
			state->config->sections = (easyini_section_t *)realloc(state->config->sections, sizeof(easyini_section_t) * state->config->nsections);
			state->section = state->config->sections + (state->config->nsections-1);
			state->section->key = get_until(line, NULL, "]");
			break;
		default:
			/* key/value pair */
			state->section->nentries++;
			state->section->entries = (easyini_entry_t *)realloc(state->section->entries, sizeof(easyini_entry_t) * state->section->nentries);
			easyini_entry_t *entry = state->section->entries + (state->section->nentries-1);
			entry->key = get_until(line, &delim, "=:");
			if (*delim != 0) delim++;
			while (isspace(*delim) || *delim == '=' || *delim == ':') delim++;
			entry->value = get_until(delim, NULL, ";#");
	}
}

easyini_section_t *easyini_get_section(easyini_config_t *config, const char *section, int index) {
	int i;
	if (section == NULL) return &(config->global_section);
	for (i = 0; i < config->nsections; i++) {
		if (!strcasecmp(config->sections[i].key, section)) {
			if (index <= 0) return config->sections+i;
			else index--;
		}
	}
	return NULL;
}

easyini_entry_t *easyini_get_entry(easyini_config_t *config, const char *section, int section_index, const char *key, int key_index) {
	int i;
	easyini_section_t *s = easyini_get_section(config, section, section_index);
	if (s == NULL) return NULL;
	for (i = 0; i < s->nentries; i++) {
		if (!strcasecmp(s->entries[i].key, key)) {
			if (key_index <= 0) return s->entries+i;
			else key_index--;
		}
	}
	return NULL;
}

const char *easyini_get_string(easyini_config_t *config, const char *section, const char *key, const char *default_value) {
	easyini_entry_t *entry = easyini_get_entry(config, section, 0, key, 0);
	if (entry == NULL) return default_value;
	else return entry->value;
}

int easyini_get_boolean(easyini_config_t *config, const char *section, const char *key, int default_value) {
	easyini_entry_t *entry = easyini_get_entry(config, section, 0, key, 0);
	if (entry == NULL || entry->value == NULL || *(entry->value) == 0) return default_value;
	else switch (*(entry->value)) {
		case 't':
		case 'T':
		case 'y':
		case 'Y':
		case '1':
			return 1;
		default:
			return 0;
	}
}

int easyini_get_int(easyini_config_t *config, const char *section, const char *key, int default_value) {
	easyini_entry_t *entry = easyini_get_entry(config, section, 0, key, 0);
	if (entry == NULL || entry->value == NULL || *(entry->value) == 0) return default_value;
	char *endptr = NULL;
	int value = strtol(entry->value, &endptr, 0);
	if (endptr != NULL && *endptr != 0) return default_value;
	else return value;
}
