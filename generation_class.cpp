#include<iostream>
#include<vector>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<numeric>

using namespace std;

class LSystemGenerator {
private:
  string axiom;
  unordered_map<char, string> rules;

  string update(string input) {
    string output;
    for (char c : input) {
      if (rules.find(c) != rules.end()) {
        // if the character in rules, replace it with the rule
        output += rules[c];
      } else {
        // if the character not in rules, leave it as is
        output += c;
      }
    }
    return output;
  }

  string update_par(string input) {
    // TODO: compute new length of each char
    vector<int> output_sizes(input.size());
    for (int i = 0; i < input.size(); i++) {
      if (rules.find(input[i]) != rules.end()) {
        output_sizes[i] = rules[input[i]].size();
      } else {
        output_sizes[i] = 1;
      }
    }

    // TODO: get prefix sum of output lengths
    vector<int> output_sums(input.size());
    // inclusive_scan(output_sizes.begin(), output_sizes.end(), output_sums.begin());
    exclusive_scan(output_sizes.begin(), output_sizes.end(), output_sums.begin(), 0);

    // TODO: for each character, write new string into output
    int output_size = output_sums.back() + output_sizes.back();
    vector<int> output(output_size);
    for (int i = 0; i < input.size(); i++) {
      if (rules.find(input[i]) != rules.end()) {
        // if the character in rules, copy the new string into output
        for (int j = 0; j < rules[input[i]].size(); j++) {
          output[output_sums[i] + j] = rules[input[i]][j];
        }
      } else {
        // if the character not in rules, copy the character into output
        output[output_sums[i]] = input[i];
      }
    }

    // convert to string before returning (Inefficient)
    return string(output.begin(), output.end());
  }

public:
  LSystemGenerator(string axiom, unordered_map<char, string> rules) {
    this->axiom = axiom;
    this->rules = rules;
  }

  string generate(int d) {
    string current = this->axiom;
    for (int i = 0; i < d; i++) {
      current = this->update(current);
      // current = this->update_par(current);
    }
    return current;
  }
};

class KochCurve : public LSystemGenerator {
public:
  KochCurve() : LSystemGenerator(
    // axiom
    "F",
    // rules
    {
      {'F', "F+F-F-F+F"}
    }
  ) {}
};


int main() {
  KochCurve k = KochCurve();
  cout << k.generate(4) << endl;
}