/*
 * Copyright (C) 2018 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ProtobufTest.h"

TEST_F(ProtobufTest, testExample) {
    // put some test data in
    tutorial::Person person1, person2;

    // test values
    std::string t_name("Hans Wurst"), t_num1("12982"), t_num2("12989101");
    int t_id = 0x1337;

    // assign person1 values
    person1.set_name(t_name);
    person1.set_id(t_id);
    auto number1 = person1.add_phones();
    auto number2 = person1.add_phones();
    number1->set_number(t_num1);
    number2->set_number(t_num2);

    // serialize to buffer
    Buffer data;
    data.padd(person1.ByteSizeLong(), 0);
    person1.SerializeToArray(data.data(), data.size());

    // deserialize to person2
    person2.ParseFromArray(data.const_data(), data.size());

    // check values
    EXPECT_EQ(t_name, person2.name());
    EXPECT_EQ(t_id, person2.id());
    EXPECT_EQ(2, person2.phones_size());
    EXPECT_EQ(t_num1, person2.phones(0).number());
    EXPECT_EQ(t_num2, person2.phones(1).number());
}