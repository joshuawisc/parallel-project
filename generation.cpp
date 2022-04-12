#include<iostream>
#include<vector>
#include<stdexcept>
#include<string>

using namespace std;


// update the input string by one iteration
string update(string input, string(*rules)(char)) {
  string output;
  for (char c : input) {
    string next = rules(c);
    output.insert(output.end(), next.begin(), next.end());
  }
  return output;
}


// generate l-system of depth d
void generate(string axiom, int d, string(*rules)(char)) {
  string current = axiom;

  cout << current << endl << endl;
  for (int i = 0; i < d; i++) {
    current = update(current, rules);
    cout << current << endl << endl;
  }
}


int main() {
  string KochCurveAxiom = {'F'};
  string (*KochCurveRules)(char) = [](char c) {
    if (c == 'F') {
      return string("F+F-F-F+F");
    } else {
      return string(1, c);
    }
  };

  generate(KochCurveAxiom, 3, KochCurveRules);
}