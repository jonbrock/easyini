#include <stdio.h>
#include "easyini.h"

int main() {
	int i;
	int j;

	easyini_config_t *config = easyini_parse_filename("config.ini");
	for (i = 0; i < config->global_section.nentries; i++) {
		printf("%s: %s\n", config->global_section.entries[i].key, config->global_section.entries[i].value);
	}
	for (i = 0; i < config->nsections; i++) {
		easyini_section_t *section = config->sections+i;
		printf("section: %s\n", section->key);
		for (j = 0; j < section->nentries; j++) {
			printf("%s: \"%s\"\n", section->entries[j].key, section->entries[j].value);
		}
	}
}
