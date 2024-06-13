#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <future>
#include <thread>

typedef std::map<std::pair<int, int>, int> LUT;

void print_map(const LUT &pairs) {
    for (auto const &[key, val] : pairs) {
        std::cout << key.first << "," << key.second << " -> " << val << std::endl;
    }
}

void print_vec(const std::vector<int> &vec) {
    for (int value : vec) {
        std::cout << value << " ";
    }
    std::cout << "\nTotal bytes: " << vec.size() << std::endl;
}

std::tuple<std::vector<int>, int> to_bytes(std::string str) {
    std::vector<int> int_bytes;
    const unsigned char *test_bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    while (*test_bytes) {
        int_bytes.push_back(static_cast<int>(*test_bytes));
        test_bytes++;
    }
    return {int_bytes, int_bytes.size()};
}

LUT count_pairs_parallel(const std::vector<int>& bytes, int start, int end) {
    LUT local_pairs;
    for (int i = start; i < end - 1; i++) {
        std::pair<int, int> pair = {bytes[i], bytes[i + 1]};
        local_pairs[pair]++;
    }
    return local_pairs;
}

LUT count_pairs(std::vector<int> bytes) {
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::future<LUT>> futures;
    LUT total_pairs;

    int chunk_size = bytes.size() / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1)? bytes.size() : start + chunk_size;
        futures.emplace_back(std::async(count_pairs_parallel, bytes, start, end));
    }

    for (auto& fut : futures) {
        LUT partial_pairs = fut.get();
        for (const auto& [pair, count] : partial_pairs) {
            total_pairs[pair] += count;
        }
    }

    return total_pairs;
}

std::pair<int, int> get_top_pair(LUT map) {
    std::pair<int, int> top_pair = map.begin()->first;

    for (auto const& [key, val] : map) {
        if (val > map[top_pair]) {
            top_pair = key;
        }
    }

    return top_pair;
}

std::vector<int> merge_parallel(const std::vector<int>& bytes, std::pair<int, int> tgt_pair, int replacement, int start, int end) {
    std::vector<int> output;
    for (size_t i = start; i < end; ++i) {
        if (i + 1 < end && std::make_pair(bytes[i], bytes[i + 1]) == tgt_pair) {
            output.push_back(replacement);
            i++; // Skip the next element as it's part of the target pair
        } else {
            output.push_back(bytes[i]);
        }
    }
    return output;
}

std::vector<int> merge(std::vector<int> bytes, std::pair<int, int> tgt_pair, int replacement) {
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::future<std::vector<int>>> futures;
    std::vector<int> output;

    int chunk_size = bytes.size() / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1)? bytes.size() : start + chunk_size;
        futures.emplace_back(std::async(merge_parallel, bytes, tgt_pair, replacement, start, end));
    }

    for (auto& fut : futures) {
        std::vector<int> partial_output = fut.get();
        output.insert(output.end(), partial_output.begin(), partial_output.end());
    }

    return output;
}

std::vector<int> compress(int vocab_size, std::vector<int> bytes) {
    LUT merges_done;
    int num_merges = vocab_size - 256;

    for (int i = 0; i < num_merges; i++) {
        auto count_future = std::async(std::launch::async, count_pairs, bytes);
        LUT pairs = count_future.get();

        auto top_pair_future = std::async(std::launch::async, get_top_pair, pairs);
        std::pair<int, int> top_pair = top_pair_future.get();

        auto merge_future = std::async(std::launch::async, merge, bytes, top_pair, i + 256);
        bytes = merge_future.get();

        merges_done[top_pair] = i + 256;
    }

    return bytes;
}

int main() {
    int vocab_size = 2048;

    std::ifstream file("shakespeare.txt");
    std::string corpus((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto [encoded_str, size] = to_bytes(corpus);

    std::vector<int> merged_bytes = compress(vocab_size, encoded_str);

    int new_size = merged_bytes.size();

    std::cout << "Original size: " << size << "\nNew size: " << new_size << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision(1) << size / (float)new_size << "X" << std::endl;
}