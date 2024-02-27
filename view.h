#pragma once

#include <errno.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>

typedef struct DataView_s {
	void      *data;
	uint64_t   size;
	int64_t    error;
	uint8_t    use_jmp;
	sigjmp_buf error_jmp;
} DataView;

typedef struct DVParseStep_s {
	uint64_t n_discard; // # of characters to remove from the look-ahead DataView
} DVParseStep;

typedef uint8_t (*DVConditional)(uint8_t b);
// "ahead" contains the data left to parse
// the parse step describes what to do in between each call from ViewParse
// state is a pointer to caller-supplied arbitrary parser state, preferably a struct
//  - this could be used to implement lookbehind etc.
typedef uint8_t (*DVParser)(DataView *ahead, DVParseStep *step, void *state);

#define VIEW_NULL ((DataView) { 0 })


DataView CreateView(void *data, uint64_t size);
DataView ViewSub(DataView *view, uint64_t start, uint64_t size);
DataView ViewCut(DataView *view, uint64_t count);
DataView ViewSubCond(DataView *view, uint64_t start, DVConditional cond);
DataView ViewCutCond(DataView *view, DVConditional cond);
void     ViewSet(DataView *view, void *data, uint64_t size);

#define  ViewCstr(str) CreateView(str, strlen(str) + 1)
#define  ViewDup(view) CreateView(view->data, view->size)
#define  ViewSetCstr(view, str) ViewSet(view, str, strlen(str) + 1)


uint8_t  ViewAt(DataView *view, uint64_t pos);
void     ViewDiscard(DataView *view, uint64_t count);
void     ViewDiscardEnd(DataView *view, uint64_t count);
// NOTE: only swaps data + size
void     ViewSwap(DataView *a, DataView *b);
// sizeof(out) should fit at least count bytes
uint64_t ViewCopy(DataView *view, void *out, uint64_t start, uint64_t count);

#define CViewAt(view, pos) ((char) ViewAt(view, pos))

// see description of typedef DVParser
void ViewParse(DataView *view, void *state, DVParser parser);
