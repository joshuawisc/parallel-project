#include<iostream>
#include<vector>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<numeric>
#include<stack>
#include<math.h>
#include<random>
#include <bits/stdc++.h>

using namespace std;

// // return a random floating point value between 0 and 1
static float randomFloat() { return static_cast<float>(rand()) / RAND_MAX; }

static float random2() {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(0, INT_MAX);
    return float(distribution(generator))/INT_MAX;
}

// // return a random floating point value between 0 and 1
static float randomFloatR(unsigned int *seedp) { return static_cast<float>(rand_r(seedp)) / RAND_MAX; }

class LSystem {
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
    for (unsigned int i = 0; i < input.size(); i++) {
      if (rules.find(input[i]) != rules.end()) {
        output_sizes[i] = rules[input[i]].size();
      } else {
        output_sizes[i] = 1;
      }
    }

    // TODO: get prefix sum of output lengths
    vector<int> output_sums(input.size());
    // inclusive_scan(output_sizes.begin(), output_sizes.end(), output_sums.begin());
    // TODO: EXCLUSIVE SCAN
    // std::exclusive_scan(output_sizes.begin(), output_sizes.end(), output_sums.begin(), 0);

    // TODO: for each character, write new string into output
    int output_size = output_sums.back() + output_sizes.back();
    vector<int> output(output_size);
    for (unsigned int i = 0; i < input.size(); i++) {
      if (rules.find(input[i]) != rules.end()) {
        // if the character in rules, copy the new string into output
        for (unsigned int j = 0; j < rules[input[i]].size(); j++) {
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
  float rotation;
  int depth;
  string instructions;
  vector<float> lines;

  LSystem() {

  }

  // axiom, rules, and rotation determine the L-System
  // x, y, and angle determine the starting position and angle
  LSystem(string axiom, unordered_map<char, string> rules, float rotation, float x, float y, float angle, float length, float color[3], int depth) {
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
    this->depth = depth;
  }

  string generate(int d) {
    string current = axiom;
    for (int i = 0; i < d; i++) {
      current = update(current);
      // current = this->update_par(current);
    }

    instructions = current;
    return current;
  }
  int numLines(int d) {
    int num_lines = 0;
    for (char c : instructions) {
      if (c == 'F') num_lines++;
    }
    return num_lines;
  }

  // Make sure to call generate() before calling this function
  vector<float> getLines(int d) {
    int num_lines = numLines(d);
    lines.reserve(4 * num_lines); // store x, y, new_x, new_y for each line
    stack<float> stack_x;
    stack<float> stack_y;
    stack<float> stack_angle;

    unsigned int s = 618;

    int l = 0;
    for (char c : instructions) {
      if (c == 'F') {
        float new_x = x + length * cos(angle);
        float new_y = y + length * sin(angle);
        lines[4 * l] = x;
        lines[4 * l + 1] = y;
        lines[4 * l + 2] = new_x;
        lines[4 * l + 3] = new_y;
        x = new_x, y = new_y;
        l++;
      } else if (c == '+') {
        angle += rotation*random2();
      } else if (c == '-') {
        angle -= rotation*random2();
      } else if (c == '[') {
        stack_x.push(x);
        stack_y.push(y);
        stack_angle.push(angle);
      } else if (c == ']') {
        x = stack_x.top();
        y = stack_y.top();
        angle = stack_angle.top();
        stack_x.pop();
        stack_y.pop();
        stack_angle.pop();
      }
    }
    //printLines();
    return lines;
  }

  void printLines() {
      for (int i = 0; i < (int)lines.size(); i+=4) {
          printf("%.3f, %.3f, %.3f, %.3f\n", lines[i], lines[i+1], lines[i+2], lines[i+3]);
      }
  }
};

class KochCurve : public LSystem {
public:
  KochCurve(float x, float y, float angle, float length, float color[3], int depth) : LSystem(
    // axiom
    "F",
    // rules
    {
      {'F', "F+F-F-F+F"}
    },
    // rotation
    90,
    x, y, angle, length, color, depth
  ) {}
};

class Fern : public LSystem {
public:
  Fern(float x, float y, float angle, float length, float color[3], int depth) : LSystem(
    // axiom
    "X",
    // rules
    {
      {'X', "F-[[X]+X]+F[+FX]-X"},
      {'F', "FF"}
    },
    // rotation
    0.436,
    x, y, angle, length, color, depth
  ) {}

  int numLines(int d) {
    // we can compute the number of lines for the fern more efficiently using a recurrence
    // F[n] = 3 * 2^(n-1)[2^n - 1]
    return 3 * (1 << (d - 1)) * ((1 << d) - 1);
  }
};


// int main() {
//   // draw
//   float rotation = 80;
//   float line_length = 10;

//   float x = 0, y = 0; // initial position
//   float angle = 0; // initial angle
//   float colors[3] = {1.0, 1.0, 1.0};
//   Fern L(x, y, angle, 1.0, colors);
//   string instructions = L.generate(3);
//   // cout << instructions << endl;

//   vector<float> lines = L.getLines(3);
//   for (auto v : lines) {
//     cout << v << " ";
//   }
//   cout << endl;

//   L.generate(1);
//   cout << L.numLines(1) << endl;
//   L.generate(2);
//   cout << L.numLines(2) << endl;
//   L.generate(3);
//   cout << L.numLines(3) << endl;

// }
