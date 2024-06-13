#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

typedef std::map<std::pair<int, int>, int> LUT;

void print_map(const LUT &pairs)
{
    for (auto const &[key, val] : pairs)
    {
        std::cout << key.first << "," << key.second << " -> " << val << std::endl;
    }
}

void print_vec(const std::vector<int> &vec)
{
    for (int value : vec){
        std::cout << value << " ";
    }

    std::cout << "\nTotal bytes: " << vec.size() << std::endl;
}

std::tuple<std::vector<int>, int> to_bytes(std::string str){
    std::vector<int> int_bytes;
    
    const unsigned char *test_bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    
    while (*test_bytes)
    {
        int_bytes.push_back(static_cast<int>(*test_bytes));
        test_bytes++;
    }
    
    return {int_bytes, int_bytes.size()};
}


LUT count_pairs(std::vector<int> bytes){
    LUT pairs;
    int size = bytes.size();

    for (int i = 0; i < size - 1; i++){
        std::pair<int, int> pair = {bytes[i], bytes[i + 1]};
        pairs[pair]++;
    }

    return pairs;
}

std::pair<int, int> get_top_pair(LUT map){
    std::pair<int, int> top_pair = map.begin() -> first;

    for (auto const& [key, val]: map){
        if (val > map[top_pair]){
            top_pair = key;
        }
    }

    return top_pair;
}

std::vector<int> merge(std::vector<int> bytes, std::pair<int, int> tgt_pair, int replacement) {
    std::vector<int> output;

    int size = bytes.size();

    for (size_t i = 0; i < size; ++i)
    {
        if (i + 1 < size && std::make_pair(bytes[i], bytes[i + 1]) == tgt_pair)
        {
            output.push_back(replacement);
            i++; // Skip the next element as it's part of the target pair
        }
        else
        {
            output.push_back(bytes[i]);
        }
    }

    return output;
}

std::vector<int> compress (int vocab_size, std::vector<int> bytes){
    LUT merges_done;
    int num_merges = vocab_size - 256;

    for (int i = 0; i < num_merges; i++)
    {
        LUT pairs = count_pairs(bytes);
        std::pair<int, int> top_pair = get_top_pair(pairs);
        bytes = merge(bytes, top_pair, i + 256);
        merges_done[top_pair] = i + 256;
    }

    return bytes;
}

int main()
{
    int vocab_size = 2048;

    // load test_corpus.txt into a string
    std::ifstream file("shakespeare.txt");
    std::string corpus((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto [encoded_str, size] = to_bytes(corpus);

    std::vector<int> merged_bytes = compress(vocab_size, encoded_str);

    int new_size = merged_bytes.size();

    std::cout << "Original size: " << size << "\nNew size: " << new_size << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision(1) << size / (float)new_size << "X" << std::endl;
}