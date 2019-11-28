#include "tagged_storage.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* TODO:
 *
 * - Ensure 4 or 8 byte alignment of regions
 */

static const int ts_magic = 0x54616753; /* "TagS" */
static const ts_ver_t ts_version = 0;
static const ts_tag_t ts_tag_invalid = 0;

typedef struct ts_entry_header {
	ts_tag_t tag;
	ts_ver_t version;
	size_t data_size;

	uint8_t data[];
} *ts_entry_header_t;

typedef struct ts_entry_footer {
	size_t data_size;
} *ts_entry_footer_t;

/*---------------------------- Navigation ----------------------------*/

static inline ts_entry_header_t
ts_data_header(void *data) {
	return (ts_entry_header_t)(((uint8_t *)data) - sizeof(struct ts_entry_header));
}

static inline ts_entry_footer_t
ts_data_footer(void *data) {
	return (ts_entry_footer_t)(((uint8_t *)data) + ts_data_header(data)->data_size);
}


static inline ts_entry_header_t
ts_entry_first(struct ts *s) {
	return (ts_entry_header_t)s->entries;
}

static inline ts_entry_footer_t
ts_entry_footer(ts_entry_header_t hdr) {
	uint8_t *ptr = (uint8_t *)hdr;
	ptr += sizeof(struct ts_entry_header);
	ptr += hdr->data_size;
	return (ts_entry_footer_t)ptr;
}

static inline ts_entry_header_t
ts_entry_next(ts_entry_header_t hdr) {
	uint8_t *ptr = (uint8_t *)hdr;
	ptr += sizeof(struct ts_entry_header);
	ptr += hdr->data_size;
	ptr += sizeof(struct ts_entry_footer);
	return (ts_entry_header_t)ptr;
}

static inline ts_entry_header_t
ts_entry_prev(ts_entry_header_t hdr) {
	uint8_t *ptr = (uint8_t *)hdr;
	ptr -= sizeof(struct ts_entry_footer);
	ptr -= ((ts_entry_footer_t)ptr)->data_size;
	ptr -= sizeof(struct ts_entry_header);
	return (ts_entry_header_t)ptr;
}


static inline size_t
ts_entry_space_total(struct ts *s) {
	return s->storage_size - sizeof(struct  ts);
}

static inline size_t
ts_entry_space_available(struct ts *s) {
	return ts_entry_space_total(s) - s->entries_size;
}

static inline size_t
ts_entry_space_required(size_t data_size) {
	return sizeof(struct ts_entry_header)
		+ data_size
		+ sizeof(struct ts_entry_footer);
}

/*---------------------------- Assertions ----------------------------*/

static inline int
ts_valid_magic(struct ts *s) {
	return s->magic == ts_magic;
}

static inline int
ts_valid_version(struct ts *s) {
	return s->version == ts_version;
}

static inline int
ts_valid_sizes(struct ts *s) {
	return s->entries_size <= ts_entry_space_total(s);
}

static inline int
ts_valid_ptr_data(struct ts *s, void *data) {
	return (s->entries < (uint8_t *)data)
		&& (uint8_t *)data < (s->entries + s->entries_size);
}

static inline int
ts_valid_ptr_hdr(struct ts *s, ts_entry_header_t hdr) {
	return (s->entries <= (uint8_t *)hdr)
		&& (uint8_t *)hdr < (s->entries + s->entries_size);
}

static inline int
ts_valid_entry(ts_entry_header_t hdr) {
	return hdr->data_size == ts_entry_footer(hdr)->data_size;
}

/*---------------------------- Interface ----------------------------*/


void ts_init(struct ts *s, size_t storage_size) {
	assert(s);

	if (!ts_consistent(s, storage_size)) {
		ts_clean(s);
	}
}

void *ts_append(struct ts *s, ts_tag_t tag, ts_ver_t version, size_t data_size) {
	ts_entry_header_t hdr;
	assert(s);
	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
	assert(tag != ts_tag_invalid);

	if (ts_entry_space_available(s) < ts_entry_space_required(data_size)) {
		return NULL;
	}

	hdr = (ts_entry_header_t)(s->entries + s->entries_size);
	s->entries_size += ts_entry_space_required(data_size);

	hdr->tag = tag;
	hdr->version = version;
	hdr->data_size = data_size;
	ts_entry_footer(hdr)->data_size = data_size;

	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
	assert(ts_valid_ptr_hdr(s, hdr));
	assert(ts_valid_ptr_data(s, hdr->data));
	assert(ts_valid_entry(hdr));
	return hdr->data;
}

void *ts_resize(struct ts *s, void *data, size_t data_size) {
	assert(s);
	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
	assert(ts_valid_ptr_data(s, data));

	data_size = data_size;

	assert(0); /* Unimplemented */

	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
}

void ts_remove(struct ts *s, void *data) {
	ts_resize(s, data, 0);
}

void *ts_find_next(struct ts *s, ts_tag_t tag, void *data_start,
		    ts_ver_t *version, size_t *data_size) {
	ts_entry_header_t hdr;
	assert(s);
	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
	assert((data_start == NULL) || ts_valid_ptr_data(s, data_start));

	if (data_start == NULL) {
		hdr = ts_entry_first(s);
	} else {
		hdr = ts_data_header(data_start);
		hdr = ts_entry_next(hdr);
	}

	for (; ts_valid_ptr_hdr(s, hdr); hdr = ts_entry_next(hdr)) {
		if (hdr->tag == tag) {
			if (version) {
				*version = hdr->version;
			}
			if(data_size) {
				*data_size = hdr->data_size;
			}
			return hdr->data;
		}
	}

	return NULL;
}

/*---------------------------- Maintence ----------------------------*/

void ts_reset(struct ts *s, size_t storage_size) {
	assert(s);

	memset(s, 0, storage_size);
	s->magic = ts_magic;
	s->version = ts_version;
	s->storage_size = storage_size;
	s->entries_size = 0;

	assert(ts_valid_magic(s));
	assert(ts_valid_version(s));
	assert(ts_valid_sizes(s));
}

bool ts_consistent(struct ts *s, size_t storage_size) {
	assert(s);
	assert(storage_size);

	/* Order In Which We Validate
	 * * Honor the struct ts's entries size
	 * * An entry is good if both header and footer agree
	 */
	assert(0); /* Unimplemented */
}

void ts_clean(struct ts *s) {
	assert(s);

	void *data = NULL;

	while ((data = ts_find_next(s, ts_tag_invalid, data, NULL, NULL))) {
		ts_remove(s, data);
	}
}