/**
 *  This file is part of rmlint.
 *
 *  rmlint is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  rmlint is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with rmlint.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *
 *  - Christopher <sahib> Pahl 2010-2014 (https://github.com/sahib)
 *  - Daniel <SeeSpotRun> T.   2014-2014 (https://github.com/SeeSpotRun)
 *
 * Hosted on http://github.com/sahib/rmlint
 *
 */

#ifndef RM_UTILITIES_H_INCLUDE
#define RM_UTILITIES_H_INCLUDE

#include <glib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <linux/fiemap.h>


/////////////////////////////////////
//   UID/GID VALIDITY CHECKING     //
/////////////////////////////////////

typedef struct RmUserGroupNode {
    gulong gid, uid;
} RmUserGroupNode;

/**
 * @brief Create a new list of users.
 */
RmUserGroupNode **rm_userlist_new(void);

/**
 * @brief Check if a uid and gid is contained in the list.
 *
 * @param valid_uid (out)
 * @param valid_gid (out)
 *
 * @return true if both are valid.
 */
bool rm_userlist_contains(RmUserGroupNode **list, unsigned long uid, unsigned gid, bool *valid_uid, bool *valid_gid);

/**
 * @brief Deallocate the memory allocated by rm_userlist_new()
 */
void rm_userlist_destroy(RmUserGroupNode **list);

/**
 * @brief Get the name of the user running rmlint.
 */
char *rm_util_get_username(void);

/**
 * @brief Get the group of the user running rmlint.
 */
char *rm_util_get_groupname(void);

////////////////////////////////////
//       GENERAL UTILITES         //
////////////////////////////////////

/**
 * @brief Replace {subs} with {with} in {string}
 *
 * @return a newly allocated string, g_free it.
 */
char *rm_util_strsub(const char *string, const char *subs, const char *with);

/**
 * @brief Check if a file has a invalid gid/uid or both.
 *
 * @return the appropiate RmLintType for the file
 */
int rm_util_uid_gid_check(struct stat *statp, RmUserGroupNode **userlist);

/**
 * @brief Check if a file is a binary that is not stripped.
  */
bool rm_util_is_nonstripped(const char *path);

/**
 * @brief Get the basename part of the file. It does not change filename.
 *
 * @return NULL on failure, the pointer after the last / on success.
 */
char *rm_util_basename(const char *filename);

/**
 * @brief Get the inode of the directory of the file specified in path.
 */
ino_t rm_util_parent_node(const char *path);

/*
 * @brief Takes num and converts into some human readable string. 1024 -> 1KB
 */
void rm_util_size_to_human_readable(guint64 num, char *in, gsize len);

/////////////////////////////////////
//    MOUNTTABLE IMPLEMENTATION    //
/////////////////////////////////////

typedef struct RmMountTable {
    GHashTable *part_table;
    GHashTable *disk_table;
    GHashTable *nfs_table;
} RmMountTable;

typedef struct RmDiskInfo {
    char *name;
    char is_rotational;
} RmDiskInfo;

typedef struct RmPartitionInfo {
    char *name;
    dev_t disk;
} RmPartitionInfo;

/**
 * @brief Allocates a new mounttable.
 *
 * @return The mounttable. Free with rm_mounts_table_destroy.
 */
RmMountTable *rm_mounts_table_new(void);

/**
 * @brief Destroy a previously allocated mounttable.
 *
 * @param self the table to destroy.
 */
void rm_mounts_table_destroy(RmMountTable *self);

/**
 * @brief Check if the device is or is part of a nonrotational device.
 *
 * This operation has constant time.
 *
 * @param self the table to lookup from.
 * @param device the dev_t of a file, e.g. looked up from stat(2)
 *
 * @return true if it is non a nonrational device.
 */
bool rm_mounts_is_nonrotational(RmMountTable *self, dev_t device);

/**
 * @brief Return name of device/disk.
 *
 * This operation has constant time.
 *
 * @param self the table to lookup from.
 * @param device the dev_t of a disk
 *
 * @return pointer to disk name.
 */
char *rm_mounts_get_disk_name(RmMountTable *self, dev_t device);

/**
 * @brief Same as above, but calls stat(2) on path for you.
 */
bool rm_mounts_is_nonrotational_by_path(RmMountTable *self, const char *path);

/**
 * @brief Get the disk behind the partition.
 *
 * @param self the table to lookup from.
 * @param partition the dev_t of a partition (sda1 -> 8:1), e.g. looked up from stat(2)
 *
 * @return the dev_t of the whole disk. (sda 8:0)
 */
dev_t rm_mounts_get_disk_id(RmMountTable *self, dev_t partition);

/**
 * @brief Same as above, but calls stat(2) on path for you.
 */
dev_t rm_mounts_get_disk_id_by_path(RmMountTable *self, const char *path);

/////////////////////////////////
//    FIEMAP IMPLEMENATION     //
/////////////////////////////////

/* typedef RmOffsetTable, in case we need to exchange
 * the data structure at any point.
 */
typedef GSequence *RmOffsetTable;

/**
 * @brief Create a table with the extents for a file at path.
 */
RmOffsetTable rm_offset_create_table(const char *path);

/**
 * @brief Lookup the physical offset of a file at any given offset.
 *
 * @return the physical offset starting from the disk.
 */
guint64 rm_offset_lookup(RmOffsetTable table, guint64 file_offset);

guint64 rm_offset_bytes_to_next_fragment(RmOffsetTable table, guint64 file_offset);

/**
 * @brief Create a new GThreadPool with default settings.
 *
 * @param func func to execute
 * @param data user_data to pass
 * @param threads how many threads at max to use.
 *
 * @return newly allocated GThreadPool
 */
GThreadPool *rm_util_thread_pool_new(GFunc func, gpointer data, int threads);

/**
 * @brief Push a new job to a threadpool.
 *
 * @return true on success.
 */
bool rm_util_thread_pool_push(GThreadPool *pool, gpointer data);

/**
 * @brief Free the allocated table.
 */
inline void rm_offset_free(RmOffsetTable table) {
    g_sequence_free(table);
}

#endif /* RM_UTILITIES_H_INCLUDE*/