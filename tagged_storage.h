
#ifndef __TAGGED_STORAGE_H_
#define __TAGGED_STORAGE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint16_t ts_tag_t;
typedef uint16_t ts_ver_t;

/**
 * @brief
 *
 * Uses of this library assume that data stored in tags can be blown away
 * or corrupted when ts_clean is called.
 */
struct ts {
	int magic;
	ts_ver_t version;

	size_t storage_size; // This is so we can adjust across boots
	size_t entries_size;

	uint8_t entries[];
};

/**
 * @brief
 *
 * This will make storage consistent and compact.
 * If storage shrinks or a record is corrupt,
 * records may be dropped.
 *
 * @param s
 * @param storage_size The entire size of the buffer
 */
void ts_init(struct ts *s, size_t storage_size);

/**
 * @brief Allocate a record at the end of the tag store
 *
 * @param s The tagged storage unit
 * @param tag The tag to place
 * @param version
 * @param dsize The size of the data block
 * @return void* NULL if out of room, pointer to the data section otherwise
 */
void *ts_append(struct ts *s, ts_tag_t tag, ts_ver_t version, size_t data_size);

/**
 * @brief
 *
 * @return void* NULL if resize is too large, pointer to data otherwise
 */
void *ts_resize(struct ts *s, void *data, size_t size);

/**
 * @brief Remove a record without invalidation/cleanup
 *
 * @param s
 * @param data
 */
void ts_remove(struct ts *s, void *data);

/**
 * @brief Mark the data block as invalid
 *
 *
 * @param s
 * @param data
 */
void ts_invalidate(struct ts *s, void *data);

/**
 * @brief Find the next record with tag
 *
 * @param s
 * @param tag
 * @param start
 * @param version
 * @param dsize
 * @return void*
 */
void *ts_find_next(struct ts *s, ts_tag_t tag, void *data_start,
		    ts_ver_t *version, size_t *data_size);

/* Maintence */

void ts_reset(struct ts *s, size_t storage_size);

/**
 * @brief Scan tagged storage and force consistency
 *
 * This tool will scan the entire tagged storage looking for inconsistencies.
 * Depending on the type of inconsistency, it will be either directly
 * corrected or deferred for correction from the ts_compact utility.
 *
 * Either way, this tool ensures that the storage unit is useable after
 * running.
 *
 * @param s
 * @return true The storage unit was already consistent
 * @return false The storage unit required correcting
 */
bool ts_consistent(struct ts *s);

/**
 * @brief Removes invalid records
 *
 * This tool will remove invalid records from an already consistent
 * storage unit.
 *
 * This tool can rearrange records.
 *
 * @param s
 */
void ts_clean(struct ts *s);


#endif /* __TAGGED_STORAGE_H_ */