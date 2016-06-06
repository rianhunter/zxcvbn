#ifndef __ZXCVBN__MATCHING_HPP
#define __ZXCVBN__MATCHING_HPP

#include <zxcvbn/common.hpp>

#include <string>
#include <vector>

namespace zxcvbn {

std::vector<Match> omnimatch(const std::string & password,
                             const std::vector<std::string> & ordered_list = {});

}

#endif
