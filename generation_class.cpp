#include<iostream>
#include<vector>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<numeric>

using namespace std;

class LSystem {
private:
  string axiom;
  unordered_map<char, string> rules;
  float rotation;

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
  float x;
  float y;
  float angle;
  float length;
  float color[3];

  // axiom, rules, and rotation determine the L-System
  // x, y, and angle determine the starting position and angle
  LSystem(string axiom, unordered_map<char, string> rules, float rotation, float x, float y, float angle, float length, float color[3]) {
    this->axiom = axiom;
    this->rules = rules;
    this->rotation = rotation;
    this->x = x;
    this->y = y;
    this->angle = angle;
    this->length = length;
    this->color[0] = color[0];
    this->color[1] = color[1];
    this->color[2] = color[2];
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

class KochCurve : public LSystem {
public:
  KochCurve(float x, float y, float angle, float length, float color[3]) : LSystem(
    // axiom
    "F",
    // rules
    {
      {'F', "F+F-F-F+F"}
    },
    // rotation
    80,
    x, y, angle, length, color
  ) {}
};


int main() {
  // draw
  float rotation = 80;
  float line_length = 10;

  float x = 0, y = 0; // initial position
  float angle = 0; // initial angle

  KochCurve L = KochCurve(x, y, angle);
  string instructions = L.generate(3);
  cout << instructions << endl;
  
  // draw loop
  for (char c : instructions) {
    if (c == 'F') {
      float new_x = line_length * cos(angle);
      float new_y = line_length * sin(angle);
      // TODO: draw line from (x, y) to (new_x, new_y)
      x = new_x, y = new_y;
    } else if (c == '+') {
      angle += rotation;
    } else if (c == '-') {
      angle -= rotation;
    } else if (c == '[') {
      // TODO: save current position and angle
    } else if (c == ']') {
      // TODO: restore current position and angle
    }
  }
}