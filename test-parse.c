#include "view.h"

#include <stdio.h>

char *parse_stuff[] = {
	">2345345sdfsdf2334",
	"sdfsdf23120934sf11",
	"30949403490>>45df>12>5",
	NULL
};

uint8_t IntParser(DataView *ahead, DVParseStep *step, void *state) {
	uint64_t *u64 = (uint64_t *) state;
	char c;
	
	do {
		c = CViewAt(ahead, step->n_discard);
		if (c == 0)
			return 0;
		step->n_discard++;
	} while (c < '0' || c > '9');

	*u64 = *u64 * 10 + (c - '0');
	
	return 1;
}

int main() {
	uint64_t n;
	char **p;
	DataView v = VIEW_NULL;
	
	v.use_jmp = 1;

	if (sigsetjmp(v.error_jmp, 0)) {
		fprintf(stderr, "ViewParse: %m\n");
		return 1;
	}

	for (p = parse_stuff; *p != NULL; p++) {
		n = 0;
		ViewSetCstr(&v, *p);
		ViewParse(&v, (void *) &n, IntParser);
		printf("\"%s\" -> %lu\n", *p, n);
	}

	return 0;
}
