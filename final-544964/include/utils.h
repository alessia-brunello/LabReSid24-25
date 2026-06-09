#ifndef UTILS_H
#define UTILS_H

int extract_id_from_path(const char* path, const char* prefix);

int get_json_value(const char* json, const char* key, char* output, int output_size);

#endif
