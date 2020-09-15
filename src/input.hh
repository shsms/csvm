#ifndef CSVM_INPUT_HH
#define CSVM_INPUT_HH

#include <string>

std::string next_chunk(std::fstream &file, uint64_t max_chunk_size);

#endif /* CSVM_INPUT_HH */
