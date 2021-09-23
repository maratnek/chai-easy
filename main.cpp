#include <chaiscript/chaiscript.hpp>

std::string helloWorld(const std::string &t_name) {
  return "Hello " + t_name + "!";
}

size_t SizeOf(const std::string &data) {
  return data.length();
}

int main() {
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&helloWorld), "helloWorld");
  chai.add(chaiscript::fun(&SizeOf), "SizeOf");

  std::string filename = "create_wallet.chai";
  chai.eval_file("./chai-code/" + filename);
//   chai.eval_file("transaction.chai");

//   chai.eval(R"(
//     puts(helloWorld("Bob"));
//   )");
struct MyStruct{uint64_t hash = 777;} my_struct;
auto str = std::string( reinterpret_cast<char*>(&my_struct), sizeof(my_struct) );
for (const auto& it : str)
  printf("%d\n", it);
  // std::cout << "My struct bytecode" << (uint8_t)it << std::endl;

}