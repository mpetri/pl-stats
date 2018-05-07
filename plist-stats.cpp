#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>

const size_t block_size = 128;

void count_one_blocks(const uint32_t* list, size_t num_u32, bool gaps)
{
    std::string name = "[DOCS]";
    if (!gaps)
        name = "[FREQS]";
    auto end = list + num_u32;
    size_t total_num_postings = 0;
    size_t one_postings = 0;
    size_t all_one_blocks = 0;
    size_t num_full_blocks = 0;
    size_t num_lists = 0;
    while (list != end) {
        auto list_size = list[0];
        size_t full_blocks = list_size / block_size;
        size_t prev = 0;
        for (size_t i = 0; i < full_blocks; i++) {
            auto block_ptr = list + i * block_size;
            auto ones_before = one_postings;
            for (size_t j = 0; j < block_size; j++) {
                one_postings += ((block_ptr[j] - prev) == 1);
                if (gaps)
                    prev = block_ptr[j];
            }
            if (ones_before + block_size == one_postings)
                all_one_blocks++;
        }
        num_full_blocks += full_blocks;
        total_num_postings += list_size;
        list += (list_size + 1);
        num_lists++;
    }
    std::cout << name << " total_num_postings = " << total_num_postings << "\n";
    std::cout << name << " one_postings = " << one_postings << "\n";
    std::cout << name << " num_lists = " << num_lists << "\n";
    std::cout << name << " all_one_blocks = " << all_one_blocks << "\n";
    std::cout << name << " num_full_blocks = " << num_full_blocks << "\n";

    std::cout << name << " percent_one_postings = "
              << double(one_postings) / double(total_num_postings) << "\n";
    std::cout << name << " percent_one_blocks = "
              << double(all_one_blocks) / double(num_full_blocks) << "\n";
}

int main(int argc, char** argv)
{
    std::string prefix = argv[1];
    {
        auto doc_file = prefix + ".docs";
        boost::iostreams::mapped_file_source file(doc_file);
        auto size_bytes = file.size();
        auto u32_ptr = reinterpret_cast<const uint32_t*>(file.data());
        auto size_u32 = size_bytes / 4;
        u32_ptr += 2; // skip first list
        count_one_blocks(u32_ptr, size_u32 - 2, true);
    }
    {
        auto freq_file = prefix + ".freqs";
        boost::iostreams::mapped_file_source file(freq_file);
        auto size_bytes = file.size();
        auto u32_ptr = reinterpret_cast<const uint32_t*>(file.data());
        auto size_u32 = size_bytes / 4;
        count_one_blocks(u32_ptr, size_u32, false);
    }
}