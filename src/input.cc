#include <fstream>
#include <string>

std::string next_chunk(std::fstream &file, uint64_t max_chunk_size) {
    std::string chunk;

    auto beginning = file.tellg();
    file.seekg(0, std::ios::end);
    auto end = file.tellg();
    auto chunksize = end - beginning;

    if (chunksize > max_chunk_size) {
        chunksize = max_chunk_size;
    }
    chunk.resize(chunksize);
    file.seekg(beginning);
    file.read(&chunk[0], chunk.size());
    if (!file.eof()) {
        std::string line;
        std::getline(file, line);
        if (line.length() > 0) {
            chunk += line;
        }
    }
    return chunk;
}
