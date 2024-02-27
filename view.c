#include "view.h"

#include <string.h>

void __DV_Error(DataView *view, int64_t error) {
	view->error = error;

	if (view->use_jmp)
		siglongjmp(view->error_jmp, 1);
}

DataView CreateView(void *data, uint64_t size) {
	DataView res;
	
	memset(&res, 0, sizeof(DataView));

	res.data = data;
	res.size = size;

	return res;
}

DataView ViewSub(DataView *view, uint64_t start, uint64_t size) {
	if (start + size > view->size) {
		__DV_Error(view, EDOM);
		return VIEW_NULL;
	}

	return CreateView(view->data + start, size);
}

DataView ViewCut(DataView *view, uint64_t count) {
	DataView res = ViewSub(view->data, 0, count);
	if (view->error) // in case of no longjmp
		return VIEW_NULL;

	view->data += count;
	view->size -= count;

	return res;
}

DataView ViewSubCond(DataView *view, uint64_t start, DVConditional cond) {
	uint64_t size = 0;

	while (size < view->size && cond(ViewAt(view, size)))
		size++;

	return ViewSub(view, start, size);
}

DataView ViewCutCond(DataView *view, DVConditional cond) {
	uint64_t count = 0;

	while (!view->error && count < view->size && cond(ViewAt(view, count)))
		count++;

	return view->error ? VIEW_NULL : ViewCut(view, count);
}

void ViewSet(DataView *view, void *data, uint64_t size) {
	view->data = data;
	view->size = size;
}

uint8_t ViewAt(DataView *view, uint64_t pos) {
	if (pos < view->size)
		return *((uint8_t *) view->data + pos);
	
	__DV_Error(view, EDOM); // out of domain

	return 0;
}

void ViewDiscard(DataView *view, uint64_t count) {
	if ((int64_t) (view->size - count) < 0) {
		__DV_Error(view, EDOM);
		return;
	}

	view->data += count;
	view->size -= count;
}

void ViewDiscardEnd(DataView *view, uint64_t count) {
	if ((int64_t) (view->size - count) < 0) {
		__DV_Error(view, EDOM);
		return;
	}

	// why does this need to be a function
	view->size -= count;
}

void ViewSwap(DataView *a, DataView *b) {
	DataView tmp;

	tmp.data = a->data;
	tmp.size = a->size;

	a->data = b->data;
	a->size = b->size;

	b->data = tmp.data;
	b->size = tmp.size;
}

uint64_t ViewCopy(DataView *view, void *out, uint64_t start, uint64_t count) {
	if (start + count > view->size) {
		__DV_Error(view, EDOM);
		return 0;
	}

	memcpy(out, view->data + start, count);

	return count;
}

void ViewParse(DataView *view, void *state, DVParser parser) {
	DVParseStep step = { 0 };

	while (view->size && parser(view, &step, state)) {
		ViewCut(view, step.n_discard);
		if (view->error)
			return;
		
		step.n_discard = 0;
	}
}
