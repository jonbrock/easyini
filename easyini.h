#ifndef _EASYINI_H_
#define _EASYINI_H_

typedef struct easyini_entry {
	char *key;
	char *value;
} easyini_entry_t;

typedef struct easyini_section {
	char *key;
	easyini_entry_t *entries;
	int nentries;
} easyini_section_t;

typedef struct easyini_config {
	easyini_section_t global_section;
	easyini_section_t *sections;
	int nsections;
} easyini_config_t;

typedef struct easyini_parse_state {
	easyini_config_t *config;
	easyini_section_t *section;
} easyini_parse_state_t;


easyini_config_t *easyini_parse_filename(const char *filename);
easyini_config_t *easyini_parse_file(FILE *fp);

const char *easyini_get_string(easyini_config_t *config, const char *section, const char *key, const char *default_value);
int easyini_get_boolean(easyini_config_t *config, const char *section, const char *key, int default_value);
int easyini_get_int(easyini_config_t *config, const char *section, const char *key, int default_value);

#endif
