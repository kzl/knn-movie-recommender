#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

string MOVIES_FILE = "data/movies.csv";
string RATINGS_FILE = "data/ratings.csv";

int MOVIE_CAP = 100000;
int K = 10, R = 10;

unordered_map<string, int> movies_name_to_id;
unordered_map<int, string> movies_id_to_name;
map<int, map<int, double> > ratings; // (userId, (movieId, rating))

vector<string> split(string s, string delim) {
	int pos = 0;
	string token;
	vector<string> output;
	
	while ((pos = s.find(delim)) != string::npos) {
	    token = s.substr(0, pos);
		output.push_back(token);
	    s.erase(0, pos + delim.length());
	}
	
	return output;
}

double user_dist(map<int, double> sample_ratings, int user_id) {
	map<int, double> user_ratings = ratings[user_id];
	
	double cost = 0.0;
	for (map<int, double>::iterator it = sample_ratings.begin(); it != sample_ratings.end(); it++) {
		if (user_ratings.find(it->first) == user_ratings.end()) {
			if (it->second <= 2.75) {
				cost += pow(5.0 - it->second, 2);
			} else {
				cost += pow(it->second - 0.5, 2);
			}
		}
	}
	
	return cost;
}

int main() {
	printf("Loading files...\n");
	
	// Read movies to dictionaries
	ifstream f_movies(MOVIES_FILE);
	
	int num_movies = -1;
	string line;
	while (getline(f_movies, line)) {
		num_movies++;
		if (num_movies == 0) continue;
		
		vector<string> l = split(line, ",");
		movies_name_to_id[l[1]] = stoi(l[0]);
		movies_id_to_name[stoi(l[0])] = l[1];
		
		if (num_movies >= MOVIE_CAP) break;
	}
	f_movies.close();
	
	// Read ratings to dictionary of vector of pairs
	ifstream f_ratings(RATINGS_FILE);
	
	int num_ratings = -1;
	while (getline(f_ratings, line)) {
		num_ratings++;
		if (num_ratings == 0) continue;
		
		vector<string> l = split(line, ",");
		ratings[stoi(l[0])][stoi(l[1])] = stod(l[2]); // (userId, (movieId, rating))
	}
	f_ratings.close();
	
	// Create a console for the user to interact with the program
	bool terminate = 0;
	map<int, double> sample_ratings;
	string input;
	
	printf("Finished loading files.\n");
	
	while (!terminate) {
		printf("\nEnter a command:\n");
		getline(cin, input);
		
		if (input.compare("add_rating") == 0) {
			printf("Enter the name of a movie:\n");
			getline(cin, input);
			
			if (movies_name_to_id.find(input) == movies_name_to_id.end()) {
				printf("Movie not found.\n");
				continue;
			}
			
			printf("Enter the rating for this movie:\n");
			string rating_val;
			getline(cin, rating_val);
			double val = stod(rating_val);
			
			if (val < 0.5 || val > 5.0) {
				printf("Value out of bounds.\n");
				continue;
			}
			
			sample_ratings[movies_name_to_id[input]] = val;
			
		} else if (input.compare("remove_rating") == 0) {
			printf("Enter the name of a movie:\n");
			getline(cin, input);
			
			if (movies_name_to_id.find(input) == movies_name_to_id.end()) {
				printf("Movie not found.\n");
				continue;
			}
			
			sample_ratings.erase(sample_ratings.find(movies_name_to_id[input]));
			
		} else if (input.compare("view_ratings") == 0) {
			if (sample_ratings.size() == 0) {
				printf("No ratings to show.\n");
				continue;
			}
			
			for (map<int, double>::iterator it = sample_ratings.begin(); it != sample_ratings.end(); it++) {
				printf("%s: %.1f\n", movies_id_to_name[it->first].c_str(), it->second);
			}
			
		} else if (input.compare("recommend") == 0) {
			printf("Enter the number of recommendations to generate:\n");
			getline(cin, input);
			int val = stoi(input);
			
			if (val < 1 || val > 100) {
				printf("Value out of bounds.\n");
				continue;
			}
			R = val;
			
			printf("Generating recommendations...\n");
			
			priority_queue<pair<double, int> > costs;
			
			for (map<int, map<int, double> >::iterator it = ratings.begin(); it != ratings.end(); it++) {
				costs.push(pair<double, int>(-1.0 * user_dist(sample_ratings, it->first), it->first));
			}
			
			map<int, double> movies_to_recommend;
			for (int k = 0; k < K; k++) {
				int user_id = costs.top().second;
				costs.pop();
				
				for (map<int, double>::iterator it = ratings[user_id].begin(); it != ratings[user_id].end(); it++) {
					if (sample_ratings.find(it->first) != sample_ratings.end()) continue;
					movies_to_recommend[it->first] += (it->second - 2.75) / (double) K;
				}
			}
			
			priority_queue<pair<double, int> > recommendations;
			
			for (map<int, double>::iterator it = movies_to_recommend.begin(); it != movies_to_recommend.end(); it++) {
				recommendations.push(pair<double, int>(it->second, it->first));
			}
			
			printf("Your top %d recommendations are:\n", R);
			
			for (int r = 0; r < R; r++) {
				int movie_id = recommendations.top().second;
				double est_rating = recommendations.top().first + 2.75;
				recommendations.pop();
				
				printf("#%d: %s\n", r+1, movies_id_to_name[movie_id].c_str());
			}
			
		} else if (input.compare("quit") == 0) {
			printf("Program will now terminate.\n");
			terminate = 1;
			
		} else if (input.compare("help") == 0) {
			printf("Commands: add_rating, remove_rating, view_ratings, recommend, quit\n");
			
		} else {
			printf("Invalid command.\n");
		}
	}
	
	return 0;
}
