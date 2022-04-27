#include<iostream>
#include<vector>
#include<stdexcept>
#include<string>

using namespace std;


// update the input string by one iteration by applying rules
string update(string input, string(*rules)(char)) {
  string output;
  for (char c : input) {
    string next = rules(c);
    output.insert(output.end(), next.begin(), next.end());
  }
  return output;
}


// generate l-system of depth d
// starting from axiom and applying rules
// print result to stdout
void generate(string axiom, string(*rules)(char), int d) {
  string current = axiom;

  cout << current << endl << endl;
  for (int i = 0; i < d; i++) {
    current = update(current, rules);
    cout << current << endl << endl;
  }
}

/*
  Just set the axiom and rules to determine the l-system
*/

int main() {
  // axiom
  string KochCurveAxiom = {'F'};
  // rules
  string (*KochCurveRules)(char) = [](char c) {
    if (c == 'F') {
      return string("F+F-F-F+F");
    } else {
      return string(1, c);
    }
  };

  // generate l-system
  generate(KochCurveAxiom, KochCurveRules, 3);
}