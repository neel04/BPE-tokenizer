#include <iostream>
#include <vector>

std::vector<int> prefix_sum(int arr[], int size){
	std::vector<int> prefix_arr(size, 0);

	for (int i = 0; i < size; i++){
		int sum = 0;

		for (int j = 0; j <= i; j++){
			sum += arr[j];
		}

		prefix_arr[i] = sum;
	}

	return prefix_arr;
}

int main(){
	int arr[5] = {1, 2, 3, 4, 5};
	int size = sizeof(arr) / sizeof(arr[0]);

	std::vector<int> result = prefix_sum(arr, size);

	std::cout << "Prefix Sum: {";

	for (int i = 0; i < result.size(); i++) {
		std::cout << result[i] << ", ";
	}

	std::cout << '}' << std::endl;

	return 0;
}