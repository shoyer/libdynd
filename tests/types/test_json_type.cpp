//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "inc_gtest.hpp"

#include <dynd/array.hpp>
#include <dynd/types/json_type.hpp>
#include <dynd/types/string_type.hpp>

using namespace std;
using namespace dynd;

TEST(JSONDType, Create) {
    ndt::type d;

    // Strings with various encodings
    d = ndt::make_json();
    EXPECT_EQ(json_type_id, d.get_type_id());
    EXPECT_EQ(dynamic_kind, d.get_kind());
    EXPECT_EQ(sizeof(void *), d.get_data_alignment());
    EXPECT_EQ(2*sizeof(void *), d.get_data_size());
    EXPECT_FALSE(d.is_expression());
    // Roundtripping through a string
    EXPECT_EQ(d, ndt::type(d.str()));
}

TEST(JSONDType, Validation) {
    nd::array a;

    a = nd::array("[1,2,3]").ucast(ndt::make_json()).eval();
    EXPECT_EQ(ndt::make_json(), a.get_type());
    EXPECT_EQ("[1,2,3]", a.as<string>());

    EXPECT_THROW(nd::array("[1,2,3]#").ucast(ndt::make_json()).eval(), invalid_argument);
}
