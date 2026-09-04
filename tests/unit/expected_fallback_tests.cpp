#define FRAMEGRAPH_FORCE_EXPECTED_FALLBACK 1
#include "framegraph/expected.hpp"
#include <string>
int main(){
    framegraph::Expected<int,std::string> value{42};
    if(!value||!value.has_value()||*value!=42||*value.operator->()!=42)return 1;
    framegraph::Expected<int,std::string> error=framegraph::unexpected(std::string("typed"));
    if(error||error.error()!="typed")return 2;
    framegraph::Expected<void,std::string> done;
    if(!done)return 3;
    framegraph::Expected<void,std::string> failed=framegraph::unexpected(std::string("void"));
    if(failed||failed.error()!="void")return 4;
}
