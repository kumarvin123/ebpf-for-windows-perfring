// Copyright (c) eBPF for Windows contributors
// SPDX-License-Identifier: MIT

#pragma once

#include "api_common.hpp"
#include "ebpf_api.h"
#include "spec_type_descriptors.hpp"

#if !defined(EBPF_API_LOCKING)
#define EBPF_API_LOCKING
#endif

struct bpf_object;

typedef struct _ebpf_ring_buffer_subscription ring_buffer_subscription_t;
typedef struct _ebpf_perf_event_array_subscription perf_event_array_subscription_t;

typedef struct bpf_program
{
    struct bpf_object* object;
    char* section_name;
    char* program_name;
    struct ebpf_inst* instructions;
    uint32_t instruction_count;
    ebpf_program_type_t program_type;
    ebpf_attach_type_t attach_type;
    ebpf_handle_t handle;
    fd_t fd;
    bool autoload;
    bool pinned;
    const char* log_buffer;
    uint32_t log_buffer_size;
} ebpf_program_t;

typedef struct bpf_map
{
    const struct bpf_object* object;
    char* name;

    // Map handle generated by the execution context.
    ebpf_handle_t map_handle;

    // Map ID generated by the execution context.
    ebpf_id_t map_id;

    // File descriptor specific to the caller's process.
    fd_t map_fd;

    // Original fd as it appears in the eBPF byte code
    // before relocation.
    fd_t original_fd;

    // Original fd of the inner_map.
    fd_t inner_map_original_fd;

    struct bpf_map* inner_map;
    ebpf_map_definition_in_memory_t map_definition;
    char* pin_path;
    bool pinned;
    // Whether this map is newly created or reused
    // from an existing map.
    bool reused;
} ebpf_map_t;

typedef struct bpf_link
{
    char* pin_path;
    ebpf_handle_t handle;
    fd_t fd;
    bool disconnected;
} ebpf_link_t;

typedef struct bpf_object
{
    char* object_name = nullptr;
    char* file_name = nullptr;
    fd_t native_module_fd = ebpf_fd_invalid;
    std::vector<ebpf_program_t*> programs;
    std::vector<ebpf_map_t*> maps;
    bool loaded = false;
    ebpf_execution_type_t execution_type = EBPF_EXECUTION_ANY;
} ebpf_object_t;

/**
 *  @brief Initialize the eBPF user mode library.
 */
uint32_t
ebpf_api_initiate() noexcept;

/**
 *  @brief Terminate the eBPF user mode library.
 */
void
ebpf_api_terminate() noexcept;

void
clean_up_ebpf_program(_In_ _Post_invalid_ ebpf_program_t* program) noexcept;

void
clean_up_ebpf_programs(_Inout_ std::vector<ebpf_program_t*>& programs) noexcept;

EBPF_API_LOCKING void
clean_up_ebpf_map(_In_ _Post_invalid_ ebpf_map_t* map) noexcept;

void
clean_up_ebpf_maps(_Inout_ std::vector<ebpf_map_t*>& maps) noexcept;

/**
 * @brief Get next eBPF object.
 *
 * @param[in] previous Pointer to previous eBPF object, or NULL to get the first one.
 * @return Pointer to the next object, or NULL if none.
 */
EBPF_API_LOCKING
_Ret_maybenull_ struct bpf_object*
ebpf_object_next(_In_opt_ const struct bpf_object* previous) noexcept;

/**
 * @brief Get next program in ebpf_object object.
 *
 * @param[in] previous Pointer to previous eBPF program, or NULL to get the first one.
 * @param[in] object Pointer to eBPF object.
 * @return Pointer to the next program, or NULL if none.
 */
_Ret_maybenull_ struct bpf_program*
ebpf_program_next(_In_opt_ const struct bpf_program* previous, _In_ const struct bpf_object* object) noexcept;

/**
 * @brief Get previous program in ebpf_object object.
 *
 * @param[in] next Pointer to next eBPF program, or NULL to get the last one.
 * @param[in] object Pointer to eBPF object.
 * @return Pointer to the previous program, or NULL if none.
 */
_Ret_maybenull_ struct bpf_program*
ebpf_program_previous(_In_opt_ const struct bpf_program* next, _In_ const struct bpf_object* object) noexcept;

/**
 * @brief Unload an eBPF program.
 *
 * @param[in, out] program Program to unload.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 */
EBPF_API_LOCKING
_Must_inspect_result_ ebpf_result_t
ebpf_program_unload(_Inout_ struct bpf_program* program) noexcept;

/**
 * @brief Bind a map to a program so that it holds a reference on the map.
 *
 * @param[in] prog_fd File descriptor of program to bind map to.
 * @param[in] map_fd File descriptor of map to bind.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_program_bind_map(fd_t program_fd, fd_t map_fd) noexcept;

/**
 * @brief Get next map in ebpf_object object.
 *
 * @param[in] previous Pointer to previous eBPF map, or NULL to get the first one.
 * @param[in] object Pointer to eBPF object.
 * @return Pointer to the next map, or NULL if none.
 */
_Ret_maybenull_ struct bpf_map*
ebpf_map_next(_In_opt_ const struct bpf_map* previous, _In_ const struct bpf_object* object) noexcept;

/**
 * @brief Get previous map in ebpf_object object.
 *
 * @param[in] next Pointer to next eBPF map, or NULL to get the last one.
 * @param[in] object Pointer to eBPF object.
 * @return Pointer to the previous map, or NULL if none.
 */
_Ret_maybenull_ struct bpf_map*
ebpf_map_previous(_In_opt_ const struct bpf_map* next, _In_ const struct bpf_object* object) noexcept;

/**
 * @brief Create a new map.
 *
 * @param[in] map_type Type of outer map to create.
 * @param[in] map_name Optionally, the name to use for the map.
 * @param[in] key_size Size in bytes of keys.
 * @param[in] value_size Size in bytes of values.
 * @param[in] max_entries Maximum number of entries in the map.
 * @param[in] opts Structure of options using which a map gets created.
 * @param[out] map_fd File descriptor for the created map. The caller needs to
 *  call _close() on the returned fd when done.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_create(
    enum bpf_map_type map_type,
    _In_opt_z_ const char* map_name,
    uint32_t key_size,
    uint32_t value_size,
    uint32_t max_entries,
    _In_opt_ const struct bpf_map_create_opts* opts,
    _Out_ fd_t* map_fd) noexcept;

/**
 * @brief Fetch fd for a program object.
 *
 * @param[in] program Pointer to eBPF program.
 * @return fd for the program on success, ebpf_fd_invalid on failure.
 */
fd_t
ebpf_program_get_fd(_In_ const struct bpf_program* program) noexcept;

/**
 * @brief Clean up ebpf_object. Also delete all the sub objects
 * (maps, programs) and close the related file descriptors.
 *
 * @param[in] object Pointer to ebpf_object.
 */
void
ebpf_object_close(_In_opt_ _Post_invalid_ struct bpf_object* object) noexcept;

void
initialize_map(_Out_ ebpf_map_t* map, _In_ const map_cache_t& map_cache) noexcept;

/**
 * @brief Pin an eBPF map to specified path.
 * @param[in] program Pointer to eBPF map.
 * @param[in] path Pin path for the map.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_pin(_In_ struct bpf_map* map, _In_opt_z_ const char* path) noexcept;

/**
 * @brief Unpin an eBPF map from the specified path.
 * @param[in] map Pointer to eBPF map.
 * @param[in] path Pin path for the map.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_unpin(_In_ struct bpf_map* map, _In_opt_z_ const char* path) noexcept;

/**
 * @brief Set pin path for an eBPF map.
 * @param[in] map Pointer to eBPF map.
 * @param[in] path Pin path for the map.
 *
 * @retval EBPF_SUCCESS The API succeeded.
 * @retval EBPF_NO_MEMORY Out of memory.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_set_pin_path(_In_ struct bpf_map* map, _In_opt_z_ const char* path) noexcept;

/**
 * @brief Update value for the specified key in an eBPF map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] key Pointer to buffer containing key, or NULL for a map with no keys.
 * @param[out] value Pointer to buffer containing value.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_update_element(fd_t map_fd, _In_opt_ const void* key, _In_ const void* value, uint64_t flags) noexcept;

/**
 * @brief Update a collection of keys and values in the map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] keys Pointer to buffer containing keys.
 * @param[in] values Pointer to buffer containing values.
 * @param[in, out] count On input, contains the maximum number of elements to
 * update. On output, contains the actual number of elements updated.
 * @param[in] flags Flags to control the behavior of the API.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_update_element_batch(
    fd_t map_fd, _In_opt_ const void* keys, _In_ const void* values, _Inout_ uint32_t* count, uint64_t flags) noexcept;

/**
 * @brief Delete an element in an eBPF map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] key Pointer to buffer containing key.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_delete_element(fd_t map_fd, _In_ const void* key) noexcept;

/**
 * @brief Delete a set of keys from the eBPF map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] keys Pointer to buffer containing list of keys.
 * @param[in,out] count On input, contains the maximum number of elements to
 * delete. On output, contains the actual number of elements deleted.
 * @param[in] flags Flags to control the behavior of the API.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_KEY_NOT_FOUND The key was not found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_delete_element_batch(fd_t map_fd, _In_ const void* keys, _Inout_ uint32_t* count, uint64_t flags) noexcept;

/**
 * @brief Look up an element in an eBPF map.
 *  For a singleton map, return the value for the given key.
 *  For a per-cpu map, return aggregate value across all CPUs.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] key Pointer to buffer containing key.
 * @param[out] value Pointer to buffer that contains value on success.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_lookup_element(fd_t map_fd, _In_opt_ const void* key, _Out_ void* value) noexcept;

/**
 * @brief Fetch the next batch of keys and values from an eBPF map.
 *  For a singleton map, return the value for the given key.
 *  For a per-cpu map, return aggregate value across all CPUs.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] in_batch Pointer to buffer containing keys.
 * @param[out] out_batch Pointer to buffer that contains values on success.
 * @param[out] keys Pointer to buffer that contains keys on success.
 * @param[out] values Pointer to buffer that contains values on success.
 * @param[in, out] count On input, contains the maximum number of elements to
 * return. On output, contains the actual number of elements returned.
 * @param[in] flags Flags to control the behavior of the API.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS The end of the map has been reached.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_KEY_NOT_FOUND The key was not found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_lookup_element_batch(
    fd_t map_fd,
    _In_opt_ const void* in_batch,
    _Out_ void* out_batch,
    _Out_ void* keys,
    _Out_ void* values,
    _Inout_ uint32_t* count,
    uint64_t flags) noexcept;

/**
 * @brief Look up an element in an eBPF map.
 *  For a singleton map, return the value for the given key.
 *  For a per-cpu map, return aggregate value across all CPUs.
 *  On successful lookup, the element is removed from the map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] key Pointer to buffer containing key, or NULL for a map with no keys.
 * @param[out] value Pointer to buffer that contains value on success.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_lookup_and_delete_element(fd_t map_fd, _In_opt_ const void* key, _Out_ void* value) noexcept;

/**
 * @brief Fetch the next batch of keys and values from an eBPF map.
 *  For a singleton map, return the value for the given key.
 *  For a per-cpu map, return aggregate value across all CPUs.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] in_batch Pointer to buffer containing keys.
 * @param[out] out_batch Pointer to buffer that contains values on success.
 * @param[out] keys Pointer to buffer that contains keys on success.
 * @param[out] values Pointer to buffer that contains values on success.
 * @param[in, out] count On input, contains the maximum number of elements to
 * return. On output, contains the actual number of elements returned.
 * @param[in] flags Flags to control the behavior of the API.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS The end of the map has been reached.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_KEY_NOT_FOUND The key was not found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_lookup_and_delete_element_batch(
    fd_t map_fd,
    _In_opt_ const void* in_batch,
    _Out_ void* out_batch,
    _Out_ void* keys,
    _Out_ void* values,
    _Inout_ uint32_t* count,
    uint64_t flags) noexcept;

/**
 * @brief Return the next key in an eBPF map.
 *
 * @param[in] map_fd File descriptor for the eBPF map.
 * @param[in] previous_key Pointer to buffer containing
    previous key or NULL to restart enumeration.
 * @param[out] next_key Pointer to buffer that contains next
 *  key on success.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS previous_key was the last key.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_map_get_next_key(fd_t map_fd, _In_opt_ const void* previous_key, _Out_opt_ void* next_key) noexcept;

/**
 * @brief Detach a link given a file descriptor.
 *
 * @param[in] fd File descriptor for the link.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_FD The file descriptor was not valid.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_detach_link_by_fd(fd_t fd) noexcept;

/**
 * @brief Open a file descriptor for the map with a given ID.
 *
 * @param[in] id ID for the map.
 * @param[out] fd A new file descriptor.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_PARAMETER No such ID found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_map_fd_by_id(ebpf_id_t id, _Out_ int* fd) noexcept;

/**
 * @brief Open a file descriptor for the eBPF program with a given ID.
 *
 * @param[in] id ID for the eBPF program.
 * @param[out] fd A new file descriptor.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_PARAMETER No such ID found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_program_fd_by_id(ebpf_id_t id, _Out_ int* fd) noexcept;

/**
 * @brief Open a file descriptor for the link with a given ID.
 *
 * @param[in] id ID for the link.
 * @param[out] fd A new file descriptor.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_PARAMETER No such ID found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_link_fd_by_id(ebpf_id_t id, _Out_ int* fd) noexcept;

/**
 * @brief Look for the next link ID greater than a given ID.
 *
 * @param[in] start_id ID to look for an ID after.
 * @param[out] next_id The next ID.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS No more IDs found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_next_link_id(ebpf_id_t start_id, ebpf_id_t _Out_* next_id) noexcept;

/**
 * @brief Look for the next map ID greater than a given ID.
 *
 * @param[in] start_id ID to look for an ID after.
 * @param[out] next_id The next ID.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS No more IDs found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_next_map_id(ebpf_id_t start_id, ebpf_id_t _Out_* next_id) noexcept;

/**
 * @brief Look for the next program ID greater than a given ID.
 *
 * @param[in] start_id ID to look for an ID after.
 * @param[out] next_id The next ID.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MORE_KEYS No more IDs found.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_get_next_program_id(ebpf_id_t start_id, ebpf_id_t _Out_* next_id) noexcept;

/**
 * @brief Obtain information about the eBPF object referred to by bpf_fd.
 * This function populates up to info_len bytes of info, which will
 * be in one of the following formats depending on the eBPF object type of
 * bpf_fd:
 *
 * * struct bpf_link_info
 * * struct bpf_map_info
 * * struct bpf_prog_info
 *
 * @param[in] bpf_fd File descriptor referring to an eBPF object.
 * @param[in, out] info Pointer to memory in which to write the info obtained.
 * On input, contains any additional parameters to use.
 * @param[in, out] info_size On input, contains the maximum number of bytes to
 * write into the info.  On output, contains the actual number of bytes written.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_object_get_info_by_fd(
    fd_t bpf_fd,
    _Inout_updates_bytes_to_(*info_size, *info_size) void* info,
    _Inout_ uint32_t* info_size,
    _Out_opt_ ebpf_object_type_t* type) noexcept;

/**
 * @brief Pin an object to the specified path.
 * @param[in] fd File descriptor to the object.
 * @param[in] path Path to pin the object to.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_object_pin(fd_t fd, _In_z_ const char* path) noexcept;

/**
 * @brief Get fd for a pinned object by pin path.
 * @param[in] path Pin path for the object.
 * @param[out] fd File descriptor for the pinned object, -1 if not found.
 *
 * @retval EBPF_SUCCESS on success, or an error code on failure.
 */
ebpf_result_t
ebpf_object_get(_In_z_ const char* path, _Out_ fd_t* fd) noexcept;

/**
 * @brief Open a file without loading the programs.
 *
 * @param[in] path File name to open.
 * @param[in] object_name Optional object name to override file name
 * as the object name.
 * @param[in] pin_root_path Optional root path for automatic pinning of maps.
 * @param[in] program_type Optional program type for all programs.
 * If NULL, the program type is derived from the section names.
 * @param[in] attach_type Default attach type for all programs.
 * If NULL, the attach type is derived from the section names.
 * @param[out] object Returns a pointer to the object created.
 * @param[out] error_message Error message string, which
 * the caller must free using ebpf_free_string().
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
EBPF_API_LOCKING
_Must_inspect_result_ ebpf_result_t
ebpf_object_open(
    _In_z_ const char* path,
    _In_opt_z_ const char* object_name,
    _In_opt_z_ const char* pin_root_path,
    _In_opt_ const ebpf_program_type_t* program_type,
    _In_opt_ const ebpf_attach_type_t* attach_type,
    _Outptr_ struct bpf_object** object,
    _Outptr_result_maybenull_z_ const char** error_message) noexcept;

/**
 * @brief Open a ELF file from memory without loading the programs.
 *
 * @param[in] buffer Pointer to the buffer containing the ELF file.
 * @param[in] buffer_size Size of the buffer containing the ELF file.
 * @param[in] object_name Optional object name to override file name
 * as the object name.
 * @param[in] pin_root_path Optional root path for automatic pinning of maps.
 * @param[in] program_type Optional program type for all programs.
 * If NULL, the program type is derived from the section names.
 * @param[in] attach_type Default attach type for all programs.
 * If NULL, the attach type is derived from the section names.
 * @param[out] object Returns a pointer to the object created.
 * @param[out] error_message Error message string, which
 * the caller must free using ebpf_free_string().
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
EBPF_API_LOCKING
_Must_inspect_result_ ebpf_result_t
ebpf_object_open_memory(
    _In_reads_(buffer_size) const uint8_t* buffer,
    size_t buffer_size,
    _In_opt_z_ const char* object_name,
    _In_opt_z_ const char* pin_root_path,
    _In_opt_ const ebpf_program_type_t* program_type,
    _In_opt_ const ebpf_attach_type_t* attach_type,
    _Outptr_ struct bpf_object** object,
    _Outptr_result_maybenull_z_ const char** error_message) noexcept;

/**
 * @brief Load all the programs in a given object.
 *
 * @param[in, out] object Object from which to load programs.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_object_load(_Inout_ struct bpf_object* object) noexcept;

/**
 * @brief Unload all the programs in a given object.
 *
 * @param[in, out] object Object in which to unload programs.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are wrong.
 */
EBPF_API_LOCKING _Must_inspect_result_ ebpf_result_t
ebpf_object_unload(_Inout_ struct bpf_object* object) noexcept;

typedef int (*ring_buffer_sample_fn)(void* ctx, void* data, size_t size);

/**
 * @brief Subscribe for notifications from the input ring buffer map.
 *
 * @param[in] ring_buffer_map_fd File descriptor to the ring buffer map.
 * @param[in, out] sample_callback_context Pointer to supplied context to be passed in notification callback.
 * @param[in] sample_callback Function pointer to notification handler.
 * @param[out] subscription Opaque pointer to ring buffer subscription object.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_ring_buffer_map_subscribe(
    fd_t ring_buffer_map_fd,
    _Inout_opt_ void* sample_callback_context,
    ring_buffer_sample_fn sample_callback,
    _Outptr_ ring_buffer_subscription_t** subscription) noexcept;

/**
 * @brief Unsubscribe from the ring buffer map event notifications.
 *
 * @param[in] subscription Pointer to ring buffer subscription to be canceled.
 */
bool
ebpf_ring_buffer_map_unsubscribe(_In_ _Post_invalid_ ring_buffer_subscription_t* subscription) noexcept;

typedef void (*perf_buffer_sample_fn)(void* ctx, int cpu, void* data, uint32_t size);
typedef void (*perf_buffer_lost_fn)(void* ctx, int cpu, uint64_t cnt);

/**
 * @brief Subscribe for notifications from the input perf event array map.
 *
 * @param[in] perf_event_array_map_fd File descriptor to the perf event array map.
 * @param[in, out] sample_callback_context Pointer to supplied context to be passed in notification callback.
 * @param[in] sample_callback Function pointer to notification handler.
 * @param[in] lost_callback Function pointer to lost record notification handler.
 * @param[out] subscription Opaque pointer to perf event array subscription object.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_NO_MEMORY Out of memory.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_perf_event_array_map_subscribe(
    fd_t perf_event_array_map_fd,
    _Inout_opt_ void* callback_context,
    perf_buffer_sample_fn sample_callback,
    perf_buffer_lost_fn lost_callback,
    _Outptr_ perf_event_array_subscription_t** subscription) noexcept;

/**
 * @brief Unsubscribe from the perf event array map event notifications.
 *
 * @param[in] subscription Pointer to perf event array subscription to be canceled.
 */
bool
ebpf_perf_event_array_map_unsubscribe(_In_ _Post_invalid_ perf_event_array_subscription_t* subscription) noexcept;

/**
 * @brief Get list of programs and stats in an ELF eBPF file.
 * @param[in] file Name of ELF file containing eBPF program.
 * @param[in] section Optionally, the name of the section to query.
 * @param[in] verbose Obtain additional info about the programs.
 * @param[out] data On success points to a list of eBPF programs.
 * @param[out] error_message On failure points to a text description of
 *  the error.
 */
uint32_t
ebpf_api_elf_enumerate_programs(
    _In_z_ const char* file,
    _In_opt_z_ const char* section,
    bool verbose,
    _Outptr_result_maybenull_ ebpf_api_program_info_t** infos,
    _Outptr_result_maybenull_z_ const char** error_message) noexcept;

#if !defined(CONFIG_BPF_JIT_DISABLED) || !defined(CONFIG_BPF_INTERPRETER_DISABLED)
/**
 * @brief Load an eBPF programs from raw instructions.
 *
 * @param[in] program_type The eBPF program type.
 * @param[in] program_name The eBPF program name.
 * @param[in] execution_type The execution type to use for this program. If
 *  EBPF_EXECUTION_ANY is specified, execution type will be decided by a
 *  system-wide policy.
 * @param[in] instructions The eBPF program byte code.
 * @param[in] instruction_count Number of instructions in the
 *  eBPF program byte code.
 * @param[out] log_buffer The buffer in which to write log messages.
 * @param[in] log_buffer_size Size in bytes of the caller's log buffer.
 * @param[out] program_fd Returns a file descriptor for the program.
 *  The caller should call _close() on the fd to close this when done.
 *
 * @retval EBPF_SUCCESS The operation was successful.
 * @retval EBPF_INVALID_ARGUMENT One or more parameters are incorrect.
 * @retval EBPF_NO_MEMORY Out of memory.
 * @retval EBPF_VERIFICATION_FAILED The program failed verification.
 * @retval EBPF_FAILED Some other error occured.
 */
_Must_inspect_result_ ebpf_result_t
ebpf_program_load_bytes(
    _In_ const ebpf_program_type_t* program_type,
    _In_opt_z_ const char* program_name,
    ebpf_execution_type_t execution_type,
    _In_reads_(instruction_count) const ebpf_inst* instructions,
    uint32_t instruction_count,
    _Out_writes_opt_(log_buffer_size) char* log_buffer,
    size_t log_buffer_size,
    _Out_ fd_t* program_fd) noexcept;
#endif

/**
 * @brief Get eBPF program type for the specified bpf program type.
 *
 * @param[in] program_type Bpf program type.
 *
 * @returns Pointer to eBPF program type, or NULL if not found.
 */
_Ret_maybenull_ const ebpf_program_type_t*
ebpf_get_ebpf_program_type(bpf_prog_type_t bpf_program_type) noexcept;

/**
 * @brief Get eBPF attach type for the specified bpf attach type.
 *
 * @param[in] program_type Bpf attach type.
 *
 * @returns Pointer to eBPF attach type, or NULL if not found.
 */
_Ret_maybenull_ const ebpf_attach_type_t*
get_ebpf_attach_type(bpf_attach_type_t bpf_attach_type) noexcept;

/**
 * @brief Get bpf program type for the specified eBPF program type.
 *
 * @param[in] program_type eBPF program type GUID.
 *
 * @returns Bpf program type, or BPF_PROG_TYPE_UNSPEC if not found.
 */
bpf_prog_type_t
get_bpf_program_type(_In_ const ebpf_program_type_t* program_type) noexcept;

/**
 * @brief Get bpf attach type for the specified eBPF attach type.
 *
 * @param[in] attach_type eBPF attach type GUID.
 *
 * @returns Bpf attach type, or BPF_ATTACH_TYPE_UNSPEC if not found.
 */
bpf_attach_type_t
get_bpf_attach_type(_In_ const ebpf_attach_type_t* ebpf_attach_type) noexcept;

/**
 * @brief Initialize the eBPF library's thread local storage.
 */
void
ebpf_api_thread_local_cleanup() noexcept;

/**
 * @brief Cleanup the eBPF library's thread local storage.
 */
void
ebpf_api_thread_local_initialize() noexcept;

static inline bool
prog_is_subprog(const struct bpf_object* obj, const struct bpf_program* prog)
{
    return (strcmp(prog->section_name, ".text") == 0) && (obj->programs.size() > 1);
}