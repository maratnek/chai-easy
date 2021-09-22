#include <chaiscript/chaiscript.hpp>

std::string helloWorld(const std::string &t_name) {
  return "Hello " + t_name + "!";
}

int main() {
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&helloWorld), "helloWorld");

  std::string filename = "create_wallet.chai";
  chai.eval_file("./" + filename);
//   chai.eval_file("transaction.chai");

//   chai.eval(R"(
//     puts(helloWorld("Bob"));
//   )");


}